/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 sound ]

  	[for Android]
	Modify : @shikarunochi
	Date   : 2020.06.01-
*/

#include "osd.h"

//#define ANDROID_SOUND_LOG

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

#if defined(ANDROID_SOUND_LOG)
// 処理の識別番号をENUMで定義
enum ProcessLabel {
    BEGIN_UPDATE_SOUND = 1,
    BRAKE_UPDATE_SOUND = 2,
    CLOSE_UPDATE_SOUND = 3,
    BEGIN_OBOE_SOUND = 4,
    CLOSE_OBOE_SOUND = 5,
};

// ログデータを保持するための構造体
struct LogEntry {
    ProcessLabel position; // 処理の識別番号
    long long timestamp;   // 経過した絶対時刻（マイクロ秒）
    int rate;              // サンプルレート
    int sample_num;        // サンプル数
    long long diff;        // 前回のログからの経過時刻（マイクロ秒）
    int bufferRemaining;   // バッファ残量
};

// ログを記録するための配列
std::vector<LogEntry> logEntries;

// 前回のログタイムスタンプ
long long lastTimestamp = 0;

// ラベル名を文字列で取得する関数
std::string get_label_name(int position) {
    switch (position) {
        case BEGIN_UPDATE_SOUND: return "BEGIN_UPDATE_SOUND";
        case BRAKE_UPDATE_SOUND: return "BRAKE_UPDATE_SOUND";
        case CLOSE_UPDATE_SOUND: return "CLOSE_UPDATE_SOUND";
        case BEGIN_OBOE_SOUND: return "BEGIN_OBOE_SOUND";
        case CLOSE_OBOE_SOUND: return "CLOSE_OBOE_SOUND";
        default: return "UNKNOWN";
    }
}

extern void push_log(ProcessLabel position, int rate, int sample_num, int buffer_remaining);
extern void dump_logs();

int sound_rate_copy;

#endif

bool sound_enable_copy = false;
bool buffer_remaining_copy = 0;

void OSD::update_sound(int* extra_frames){
#ifdef ENABLE_SOUND
    sound_enable_copy = soundEnable;
    if(!soundEnable){
        return;
    }

    // データ長
    int length = sound_samples * 2; // ステレオのために2倍

    int buffer_distance = oboeSound->inputSoundBufferPos - oboeSound->outputSoundBufferPos;
    if (buffer_distance < 0) {
        buffer_distance += SOUND_BUFFER_LENGTH;  // リングバッファのラップアラウンドを考慮
    }

    // 未再生データがデータ長の1/4以上満たされている場合は、書き込みを避ける
    if (buffer_distance > length / 4) {
#if defined(ANDROID_SOUND_LOG)
        push_log(ProcessLabel::BRAKE_UPDATE_SOUND, 0, 0, buffer_distance);
        buffer_remaining_copy = buffer_distance;
#endif
        return;
    }

    *extra_frames = 0;
    sound_muted = false;
    if (sound_available) {

        uint16_t *sound_buffer = vm->create_sound(extra_frames);

        for(int index = 0; index < length; index++){
            oboeSound->soundBuffer[oboeSound->inputSoundBufferPos] = sound_buffer[index];
            oboeSound->inputSoundBufferPos++;

            if(oboeSound->inputSoundBufferPos >= SOUND_BUFFER_LENGTH){
                oboeSound->inputSoundBufferPos = 0; // リングバッファ: ポインタをリセット
                oboeSound->inputLoopCount++; // ループカウンタをインクリメント
            }
        }
    }
#endif

#if defined(ANDROID_SOUND_LOG)
    int buffer_remaining = oboeSound->inputSoundBufferPos - oboeSound->outputSoundBufferPos;
    if (buffer_remaining < 0) {
        buffer_remaining += SOUND_BUFFER_LENGTH;  // リングバッファのラップアラウンドを考慮
    }
    push_log(ProcessLabel::BEGIN_UPDATE_SOUND, sound_rate, length, buffer_remaining);
    buffer_remaining_copy = buffer_remaining;
#endif
}

void OSD::reset_sound(){
    for(int index= 0;index <  sound_samples * sizeof(uint16_t);index++){
        oboeSound->soundBuffer[index] = 0;
    }
    oboeSound->inputSoundBufferPos = 0;
    oboeSound->outputSoundBufferPos = 0;
}

oboe::DataCallbackResult OBOESOUND::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    int16_t *intData = static_cast<int16_t*>(audioData);

    for (int i = 0; i < numFrames * 2; i += 2) { // ステレオデータのために2ステップずつ進む
        if (outputLoopCount < inputLoopCount ||
            (outputLoopCount == inputLoopCount && outputSoundBufferPos < inputSoundBufferPos)) {

            // 左チャンネルと右チャンネルのデータ取得と変換
            for (int j = 0; j < 2; j++) {
                // ここでの条件やスケーリング処理は、オーディオデータの範囲と要件に基づいて検討する必要があります
                intData[i + j] = (int16_t)soundBuffer[outputSoundBufferPos + j] / 2;
            }
            outputSoundBufferPos += 2; // ステレオのために2つのサンプルを進める
        } else {
            // バッファが追いつかない場合は無音を出力
            intData[i] = 0;
            intData[i + 1] = 0;
        }

        // リングバッファのラップアラウンド処理
        if (outputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
            outputSoundBufferPos = 0; // リングバッファ: ポインタをリセット
            outputLoopCount++; // ループカウンタをインクリメント
        }
    }
#if defined(ANDROID_SOUND_LOG)
    int buffer_remaining = buffer_remaining_copy - outputSoundBufferPos;
    if (buffer_remaining < 0) {
        buffer_remaining += SOUND_BUFFER_LENGTH;  // リングバッファのラップアラウンドを考慮
    }
    push_log(ProcessLabel::BEGIN_OBOE_SOUND, sound_rate_copy, numFrames, buffer_remaining);
    buffer_remaining_copy = buffer_remaining;
#endif
    return oboe::DataCallbackResult::Continue;
}

oboe::Result OBOESOUND::createPlaybackStream(oboe::AudioStreamBuilder builder, int sampleRate) {
    inputSoundBufferPos = 0;
    outputSoundBufferPos = 0;
    return builder.setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setFormat(oboe::AudioFormat::I16)
            ->setSampleRate(sampleRate)
            ->setChannelCount(2)
            ->setCallback(this)
            ->openManagedStream(mStream);
}


void OSD::initialize_sound(int rate, int samples){
#ifdef ENABLE_SOUND
    oboeSound = new OBOESOUND();
    LOGI("Rate:%d Samples:%d",rate, samples);
    sound_samples = samples;
    sound_rate = rate;
#if defined(ANDROID_SOUND_LOG)
    sound_rate_copy = rate;
#endif
    auto result = oboeSound->createPlaybackStream(oboe::AudioStreamBuilder(), rate);
    if (result == oboe::Result::OK){
        oboeSound->mStream->start();
        sound_available = true;
    } else {
        LOGI("Error creating playback stream. Error: %s", oboe::convertToText(result));
    }

#endif
}
void OSD::release_sound(){
}

#if defined(ANDROID_SOUND_LOG)

void push_log(ProcessLabel position, int rate, int sample_num, int buffer_remaining) {
    // 現在の時刻をマイクロ秒で取得
    auto now = std::chrono::steady_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    long long diff = lastTimestamp > 0 ? timestamp - lastTimestamp : 0;
    lastTimestamp = timestamp;

    // ログデータを配列に追加
    LogEntry entry = {position, timestamp, rate, sample_num, diff};
    logEntries.push_back(entry);

    // ログ数が1000を超えたらダンプ
    if (logEntries.size() > 1000) {
        dump_logs();
    }
}

// ログの内容を出力する関数
void dump_logs() {
    for (const auto& entry : logEntries) {
        std::stringstream log_stream;
        log_stream << std::setw(16) << std::left << get_label_name(entry.position)
                   << ", Time: " << entry.timestamp << "μs"
                   << ", Diff: " << entry.diff << "μs"
                   << ", Buff: " << entry.bufferRemaining << "μs";

        if (entry.rate > 0) {
            long long play_time_us = (entry.sample_num * 1000000LL) / entry.rate;
            log_stream << ", Sound " << play_time_us << " usec";
        }

        LOGI("%s", log_stream.str().c_str());
    }
}
#endif
