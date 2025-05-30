/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode sound - Real Implementation ]

    macOS/iOS向けの実際の音声出力システム実装
    Author : Claude
    Date   : 2025.05.29
*/

#include "osd.h"
#include "../fileio.h"
#include <cstring>
#include <cmath>
#include <stdlib.h>

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#ifdef TARGET_OS_MAC
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#elif TARGET_OS_IPHONE
#include <AVFoundation/AVFoundation.h>
#endif
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif
#endif

// 音声バッファ管理
static AudioQueueRef audioQueue = nullptr;
static AudioQueueBufferRef audioBuffers[3]; // Triple buffering
static bool audioInitialized = false;
static int16_t* mixBuffer = nullptr;
static size_t mixBufferSize = 0;

// 音声フォーマット設定
static AudioStreamBasicDescription audioFormat;

// Core Audio互換性チェック用
static AudioQueueRef testQueue = nullptr;

// Enable dual sound system support (AudioQueue + Core Audio ring buffer)
#define ENABLE_DUAL_SOUND_SYSTEM 1

// 音声コールバック関数
void audioOutputCallback(void* inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    OSD* osd = static_cast<OSD*>(inUserData);
    
    if (!osd || !osd->sound_started || osd->sound_muted) {
        // 無音データを設定
        memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataBytesCapacity);
        inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
    } else {
        // エミュレータからの音声データを取得
        int16_t* bufferPtr = static_cast<int16_t*>(inBuffer->mAudioData);
        int samples = inBuffer->mAudioDataBytesCapacity / sizeof(int16_t);
        
#if ENABLE_DUAL_SOUND_SYSTEM
        // Core Audio ring buffer system (compatible with Android)
        if (osd->coreAudioSound) {
            COREAUDIO_SOUND* coreAudio = osd->coreAudioSound;
            
            // Copy data from ring buffer
            for (int i = 0; i < samples; i += 2) { // ステレオ
                // Check if we have data available
                if (coreAudio->outputLoopCount < coreAudio->inputLoopCount ||
                    (coreAudio->outputLoopCount == coreAudio->inputLoopCount && 
                     coreAudio->outputSoundBufferPos < coreAudio->inputSoundBufferPos)) {
                    
                    // Copy left and right channels
                    // [DEBUG_START - claude code verification]
                    // uint16_t to int16_t conversion: Android版と同じ処理（2で割る）
                    // Android版: intData[i + j] = (int16_t)soundBuffer[outputSoundBufferPos + j] / 2;
                    // [DEBUG_END - claude code verification]
                    uint16_t leftSample = coreAudio->soundBuffer[coreAudio->outputSoundBufferPos];
                    uint16_t rightSample = coreAudio->soundBuffer[coreAudio->outputSoundBufferPos + 1];
                    
                    // Android版と同じ変換処理：値を2で割る（センタリングではない）
                    bufferPtr[i] = (int16_t)(leftSample / 2);
                    bufferPtr[i + 1] = (int16_t)(rightSample / 2);
                    
                    coreAudio->outputSoundBufferPos += 2; // Advance by 2 for stereo
                    
                } else {
                    // No data available, output silence
                    bufferPtr[i] = 0;
                    bufferPtr[i + 1] = 0;
                }
                
                // Handle ring buffer wraparound
                if (coreAudio->outputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
                    coreAudio->outputSoundBufferPos = 0;
                    coreAudio->outputLoopCount++;
                }
            }
        } else {
#endif
            // 音声データの生成（エミュレータコアから取得）
            if (mixBuffer && mixBufferSize >= samples * sizeof(int16_t)) {
                // 実際のエミュレータ音声を取得する場合はここで処理
                // 現在はテスト用のサイン波を生成
                static double phase = 0.0;
                const double frequency = 440.0; // A4音
                const double amplitude = 8192.0; // 振幅
                const double sampleRate = osd->sound_rate;
                
                for (int i = 0; i < samples; i += 2) { // ステレオ
                    double sample = sin(phase) * amplitude;
                    int16_t value = static_cast<int16_t>(sample);
                    
                    bufferPtr[i] = value;     // Left channel
                    bufferPtr[i + 1] = value; // Right channel
                    
                    phase += 2.0 * M_PI * frequency / sampleRate;
                    if (phase >= 2.0 * M_PI) {
                        phase -= 2.0 * M_PI;
                    }
                }
            } else {
                // バッファがない場合は無音
                memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataBytesCapacity);
            }
#if ENABLE_DUAL_SOUND_SYSTEM
        }
#endif
        
        inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
    }
    
    // バッファをキューに戻す
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}

// Core Audio対応サンプリングレート検証機能
bool is_sample_rate_supported(int rate) {
    AudioStreamBasicDescription testFormat;
    memset(&testFormat, 0, sizeof(testFormat));
    testFormat.mSampleRate = rate;
    testFormat.mFormatID = kAudioFormatLinearPCM;
    testFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    testFormat.mBytesPerPacket = 4; // 16bit * 2channels
    testFormat.mFramesPerPacket = 1;
    testFormat.mBytesPerFrame = 4;
    testFormat.mChannelsPerFrame = 2; // ステレオ
    testFormat.mBitsPerChannel = 16;
    
    OSStatus status = AudioQueueNewOutput(&testFormat, audioOutputCallback, nullptr, 
                                         CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &testQueue);
    if (status == noErr && testQueue != nullptr) {
        AudioQueueDispose(testQueue, true);
        testQueue = nullptr;
        printf("[DEBUG] Core Audio: %dHz サポート確認 ✅\n", rate);
        return true;
    }
    
    printf("[DEBUG] Core Audio: %dHz サポート確認 ❌ (status=%d)\n", rate, (int)status);
    return false;
}

// 最適なサンプリングレートを選択
int select_compatible_sample_rate(int requested_rate) {
    printf("[DEBUG] Core Audio互換性チェック開始: 要求レート=%dHz\n", requested_rate);
    
    // 1. 要求されたレートをそのまま試す
    if (is_sample_rate_supported(requested_rate)) {
        printf("[INFO] Core Audio: 要求レート %dHz を使用\n", requested_rate);
        return requested_rate;
    }
    
    // 2. 標準的なレートから最も近いものを選択
    int standard_rates[] = {44100, 48000, 96000, 22050, 11025, 8000, 192000};
    int closest_rate = 44100;
    int min_diff = abs(requested_rate - 44100);
    
    printf("[DEBUG] 代替レート検索中...\n");
    for (int rate : standard_rates) {
        if (is_sample_rate_supported(rate)) {
            int diff = abs(requested_rate - rate);
            if (diff < min_diff) {
                min_diff = diff;
                closest_rate = rate;
            }
        }
    }
    
    printf("[WARN] Core Audio: %dHz → %dHz (自動選択、差=%dHz)\n", 
           requested_rate, closest_rate, min_diff);
    return closest_rate;
}

void OSD::initialize_sound(int rate, int samples)
{
    printf("Apple音声システムを初期化中...\n");
    printf("Apple音声設定: Frequency(Hz) %d, Samples per frame %d\n", rate, samples);
    // [DEBUG_START - claude code verification]
    printf("[DEBUG] 重要: エミュレータが期待するサンプリングレート = %d Hz\n", rate);
    printf("[DEBUG] 重要: バッファサイズ（サンプル数）= %d\n", samples);
    // [DEBUG_END - claude code verification]
    
    // 要求されたレートを記録
    int requested_rate = rate;
    
    // Core Audio互換レートを選択
    int compatible_rate = select_compatible_sample_rate(rate);
    
    // 実際のレートで設定
    sound_rate = compatible_rate;
    sound_samples = samples;
    sound_available = false;
    sound_started = false;
    sound_muted = false;
    
    printf("[INFO] サウンド初期化: 要求=%dHz → 実際=%dHz\n", requested_rate, compatible_rate);
    
    // 既に初期化済みの場合は解放
    if (audioInitialized) {
        release_sound();
    }
    
#if ENABLE_DUAL_SOUND_SYSTEM
    // Create Core Audio ring buffer system (compatible with Android)
    coreAudioSound = new COREAUDIO_SOUND();
    printf("Core Audio ring buffer system created\n");
#endif
    
#ifdef __APPLE__
    // AudioStreamBasicDescriptionを設定（互換レートを使用）
    memset(&audioFormat, 0, sizeof(audioFormat));
    audioFormat.mSampleRate = compatible_rate;  // 互換レートを使用
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    audioFormat.mBytesPerPacket = 4; // 16bit * 2channels
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerFrame = 4;
    audioFormat.mChannelsPerFrame = 2; // ステレオ
    audioFormat.mBitsPerChannel = 16;
    
    // AudioQueueを作成
    OSStatus status = AudioQueueNewOutput(&audioFormat, audioOutputCallback, this, 
                                         CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &audioQueue);
    
    if (status != noErr) {
        printf("AudioQueue作成エラー: %d\n", (int)status);
        return;
    }
    
    // [DEBUG_START - claude code verification]
    printf("[DEBUG] AudioQueue設定:\n");
    printf("  - サンプリングレート: %.0f Hz\n", audioFormat.mSampleRate);
    printf("  - チャンネル数: %d\n", audioFormat.mChannelsPerFrame);
    printf("  - ビット深度: %d\n", audioFormat.mBitsPerChannel);
    printf("  - バイト/フレーム: %d\n", audioFormat.mBytesPerFrame);
    // [DEBUG_END - claude code verification]
    
    // バッファサイズを計算（フレーム数 * フレームサイズ）
    UInt32 bufferSize = samples * audioFormat.mBytesPerFrame;
    
    // 音声バッファを作成
    for (int i = 0; i < 3; i++) {
        status = AudioQueueAllocateBuffer(audioQueue, bufferSize, &audioBuffers[i]);
        if (status != noErr) {
            printf("AudioQueueBuffer作成エラー[%d]: %d\n", i, (int)status);
            return;
        }
        
        // バッファを無音で初期化
        memset(audioBuffers[i]->mAudioData, 0, bufferSize);
        audioBuffers[i]->mAudioDataByteSize = bufferSize;
        
        // バッファをキューに追加
        AudioQueueEnqueueBuffer(audioQueue, audioBuffers[i], 0, nullptr);
    }
    
    // ミックス用バッファを確保
    mixBufferSize = bufferSize;
    mixBuffer = new int16_t[mixBufferSize / sizeof(int16_t)];
    memset(mixBuffer, 0, mixBufferSize);
    
#ifdef TARGET_OS_IPHONE
    // iOS固有の設定
    // TODO: AVAudioSession設定は必要に応じて後で追加
    printf("iOS音声セッション設定をスキップ\n");
#endif
    
    audioInitialized = true;
    sound_available = true;
    
    printf("Apple音声設定完了: Frequency %d, rate %d, samples %d\n", rate, rate, samples);
    
#else
    printf("Apple音声システムは非対応環境です\n");
#endif
}

void OSD::release_sound()
{
    printf("Apple音声システムを解放中...\n");
    
#if ENABLE_DUAL_SOUND_SYSTEM
    // Release Core Audio ring buffer system
    if (coreAudioSound) {
        delete coreAudioSound;
        coreAudioSound = nullptr;
        printf("Core Audio ring buffer system released\n");
    }
#endif
    
#ifdef __APPLE__
    if (audioQueue) {
        // AudioQueueを停止
        AudioQueueStop(audioQueue, true);
        
        // バッファを解放
        for (int i = 0; i < 3; i++) {
            if (audioBuffers[i]) {
                AudioQueueFreeBuffer(audioQueue, audioBuffers[i]);
                audioBuffers[i] = nullptr;
            }
        }
        
        // AudioQueueを解放
        AudioQueueDispose(audioQueue, true);
        audioQueue = nullptr;
    }
    
    // ミックス用バッファを解放
    if (mixBuffer) {
        delete[] mixBuffer;
        mixBuffer = nullptr;
        mixBufferSize = 0;
    }
    
#ifdef TARGET_OS_IPHONE
    // iOS: セッションを非アクティブ化
    // TODO: AVAudioSession非アクティブ化は必要に応じて後で追加
    printf("iOS音声セッション非アクティブ化をスキップ\n");
#endif

#endif

    audioInitialized = false;
    sound_available = false;
    sound_started = false;
    
    printf("Apple音声システム解放完了\n");
}

void OSD::update_sound(int* extra_frames)
{
#if ENABLE_DUAL_SOUND_SYSTEM
    // Android-compatible update_sound implementation
    if (!soundEnable) {
        return;
    }
    
    // Data length (stereo samples)
    int length = sound_samples * 2;
    
    if (coreAudioSound) {
        // Calculate buffer distance (available data)
        int buffer_distance = coreAudioSound->inputSoundBufferPos - coreAudioSound->outputSoundBufferPos;
        if (buffer_distance < 0) {
            buffer_distance += SOUND_BUFFER_LENGTH;
        }
        
        // Skip writing if buffer is too full (prevent buffer overflow)
        if (buffer_distance > length / 4) {
            return;
        }
        
        *extra_frames = 0;
        sound_muted = false;
        
        if (sound_available) {
            // Get sound data from VM
            uint16_t *sound_buffer = vm->create_sound(extra_frames);
            
            // Copy to ring buffer
            for (int index = 0; index < length; index++) {
                coreAudioSound->soundBuffer[coreAudioSound->inputSoundBufferPos] = sound_buffer[index];
                coreAudioSound->inputSoundBufferPos++;
                
                // Handle ring buffer wraparound
                if (coreAudioSound->inputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
                    coreAudioSound->inputSoundBufferPos = 0;
                    coreAudioSound->inputLoopCount++;
                }
            }
        }
    }
#else
    // フレーム補正は基本的に不要
    *extra_frames = 0;
    
    // 音声システムの状態を定期的にチェック
    static int check_counter = 0;
    check_counter++;
    
    if (check_counter % 3600 == 0) { // 60秒ごと
        if (sound_available && sound_started && !sound_muted) {
            printf("音声出力中: Rate=%dHz, Samples=%d\n", sound_rate, sound_samples);
        }
    }
#endif
}

// Android-compatible reset_sound implementation
void OSD::reset_sound() {
#if ENABLE_DUAL_SOUND_SYSTEM
    if (coreAudioSound) {
        // Clear ring buffer
        memset(coreAudioSound->soundBuffer, 0, sizeof(coreAudioSound->soundBuffer));
        coreAudioSound->inputSoundBufferPos = 0;
        coreAudioSound->outputSoundBufferPos = 0;
        coreAudioSound->inputLoopCount = 0;
        coreAudioSound->outputLoopCount = 0;
        
        printf("Core Audio sound buffer reset completed\n");
    }
#endif
}

// 音声開始・停止の実装
void OSD::start_sound()
{
    if (!sound_available || sound_started) {
        return;
    }
    
    printf("音声出力を開始\n");
    
#ifdef __APPLE__
    if (audioQueue) {
        OSStatus status = AudioQueueStart(audioQueue, nullptr);
        if (status == noErr) {
            sound_started = true;
            printf("AudioQueue開始成功\n");
        } else {
            printf("AudioQueue開始エラー: %d\n", (int)status);
        }
    }
#endif
}

void OSD::stop_sound()
{
    if (!sound_started) {
        return;
    }
    
    printf("音声出力を停止\n");
    
#ifdef __APPLE__
    if (audioQueue) {
        OSStatus status = AudioQueueStop(audioQueue, true);
        if (status == noErr) {
            sound_started = false;
            printf("AudioQueue停止成功\n");
        } else {
            printf("AudioQueue停止エラー: %d\n", (int)status);
        }
    }
#endif
}

void OSD::mute_sound()
{
    sound_muted = true;
    printf("音声をミュート\n");
}

void OSD::unmute_sound()
{
    sound_muted = false;
    printf("音声ミュートを解除\n");
}

// リアルタイムサウンド再初期化（Phase 5で使用予定）
void OSD::reinitialize_sound_if_needed(int new_rate, int new_samples)
{
    // レートまたはサンプル数が変更された場合のみ再初期化
    if (sound_rate != new_rate || sound_samples != new_samples) {
        printf("[INFO] サウンド設定変更検出: %dHz→%dHz, %d→%dサンプル\n", 
               sound_rate, new_rate, sound_samples, new_samples);
        
        stop_sound();
        release_sound();
        initialize_sound(new_rate, new_samples);
        if (soundEnable) start_sound();
    }
}

// 音量制御の実装
void OSD::set_sound_volume(int volume)
{
    // volume: 0-100の範囲
    float gain = volume / 100.0f;
    
#ifdef __APPLE__
    if (audioQueue) {
        OSStatus status = AudioQueueSetParameter(audioQueue, kAudioQueueParam_Volume, gain);
        if (status == noErr) {
            printf("音量を設定: %d%% (gain=%.2f)\n", volume, gain);
        } else {
            printf("音量設定エラー: %d\n", (int)status);
        }
    }
#endif
}

// 音声データ供給の実装（エミュレータコアから呼び出される）
void OSD::supply_sound_data(int16_t* data, int samples)
{
    if (!sound_available || !sound_started || sound_muted || !mixBuffer) {
        return;
    }
    
    // ミックス用バッファにデータをコピー
    size_t copySize = std::min(static_cast<size_t>(samples * sizeof(int16_t)), mixBufferSize);
    memcpy(mixBuffer, data, copySize);
}

// Bridge.mm等から呼び出すためのC言語インターフェース
extern "C" {
    void osd_start_sound() {
        extern OSD* osd;
        if (osd) osd->start_sound();
    }
    
    void osd_stop_sound() {
        extern OSD* osd;
        if (osd) osd->stop_sound();
    }
    
    void osd_mute_sound() {
        extern OSD* osd;
        if (osd) osd->mute_sound();
    }
    
    void osd_unmute_sound() {
        extern OSD* osd;
        if (osd) osd->unmute_sound();
    }
    
    void osd_set_sound_volume(int volume) {
        extern OSD* osd;
        if (osd) osd->set_sound_volume(volume);
    }
}

// Sound file loading implementation for FDD/CMT noise
void OSD::load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size)
{
    // 初期化
    if (data) *data = nullptr;
    if (dst_size) *dst_size = 0;
    
    if (!name || !data || !dst_size) {
        return;
    }
    
    // ROMディレクトリからWAVファイルを探す
    _TCHAR file_path[_MAX_PATH];
    // 現在の実行ファイルディレクトリからの相対パスで探す
    snprintf(file_path, _MAX_PATH, "%s", name);
    
    FILEIO *fio = new FILEIO();
    if (!fio->Fopen(file_path, FILEIO_READ_BINARY)) {
        // ファイルが見つからない場合
        printf("Sound file not found: %s\n", file_path);
        delete fio;
        return;
    }
    
    // WAVヘッダーの読み込み
    struct {
        char riff[4];
        uint32_t size;
        char wave[4];
        char fmt[4];
        uint32_t fmt_size;
        uint16_t format;
        uint16_t channels;
        uint32_t sample_rate;
        uint32_t byte_per_sec;
        uint16_t block_align;
        uint16_t bits_per_sample;
    } wav_header;
    
    // RIFFヘッダー読み込み
    fio->Fread(&wav_header.riff, 4, 1);
    if (memcmp(wav_header.riff, "RIFF", 4) != 0) {
        printf("Invalid WAV file (no RIFF): %s\n", file_path);
        fio->Fclose();
        delete fio;
        return;
    }
    
    fio->Fread(&wav_header.size, 4, 1);
    fio->Fread(&wav_header.wave, 4, 1);
    
    if (memcmp(wav_header.wave, "WAVE", 4) != 0) {
        printf("Invalid WAV file (no WAVE): %s\n", file_path);
        fio->Fclose();
        delete fio;
        return;
    }
    
    // fmtチャンクを探す
    bool fmt_found = false;
    int max_chunks = 20; // 無限ループ防止
    while (!fmt_found && max_chunks-- > 0) {
        char chunk_id[4];
        uint32_t chunk_size;
        
        if (fio->Fread(chunk_id, 4, 1) != 1 || fio->Fread(&chunk_size, 4, 1) != 1) {
            break; // ファイル終端またはエラー
        }
        
        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            fmt_found = true;
            wav_header.fmt_size = chunk_size;
            fio->Fread(&wav_header.format, 2, 1);
            fio->Fread(&wav_header.channels, 2, 1);
            fio->Fread(&wav_header.sample_rate, 4, 1);
            fio->Fread(&wav_header.byte_per_sec, 4, 1);
            fio->Fread(&wav_header.block_align, 2, 1);
            fio->Fread(&wav_header.bits_per_sample, 2, 1);
            
            // 残りのfmtチャンクをスキップ
            if (chunk_size > 16) {
                fio->Fseek(chunk_size - 16, FILEIO_SEEK_CUR);
            }
        } else {
            // 他のチャンクはスキップ
            fio->Fseek(chunk_size, FILEIO_SEEK_CUR);
        }
    }
    
    if (!fmt_found || wav_header.format != 1) {  // PCMフォーマットのみサポート
        printf("Unsupported WAV format: %s\n", file_path);
        fio->Fclose();
        delete fio;
        return;
    }
    
    // dataチャンクを探す
    bool data_found = false;
    uint32_t data_size = 0;
    max_chunks = 20; // 無限ループ防止をリセット
    
    while (!data_found && max_chunks-- > 0) {
        char chunk_id[4];
        uint32_t chunk_size;
        
        if (fio->Fread(chunk_id, 4, 1) != 1 || fio->Fread(&chunk_size, 4, 1) != 1) {
            break; // ファイル終端またはエラー
        }
        
        if (memcmp(chunk_id, "data", 4) == 0) {
            data_found = true;
            data_size = chunk_size;
        } else {
            // 他のチャンクはスキップ
            fio->Fseek(chunk_size, FILEIO_SEEK_CUR);
        }
    }
    
    if (!data_found || data_size == 0) {
        printf("No data chunk in WAV file: %s\n", file_path);
        fio->Fclose();
        delete fio;
        return;
    }
    
    // 16ビットサンプルとして出力するサイズを計算
    int samples = data_size / (wav_header.bits_per_sample / 8) / wav_header.channels;
    *dst_size = samples * sizeof(int16_t);
    *data = (int16_t*)malloc(*dst_size);
    
    if (!*data) {
        printf("Failed to allocate memory for sound data\n");
        fio->Fclose();
        delete fio;
        return;
    }
    
    // データ読み込みと変換
    if (wav_header.bits_per_sample == 16) {
        // 16ビットPCM
        if (wav_header.channels == 1) {
            // モノラル
            fio->Fread(*data, samples * 2, 1);
        } else {
            // ステレオ → モノラル変換
            int16_t *stereo = (int16_t*)malloc(data_size);
            fio->Fread(stereo, data_size, 1);
            for (int i = 0; i < samples; i++) {
                (*data)[i] = (stereo[i * 2] + stereo[i * 2 + 1]) / 2;
            }
            free(stereo);
        }
    } else if (wav_header.bits_per_sample == 8) {
        // 8ビットPCM → 16ビット変換
        uint8_t *data8 = (uint8_t*)malloc(data_size);
        fio->Fread(data8, data_size, 1);
        
        if (wav_header.channels == 1) {
            // モノラル
            for (int i = 0; i < samples; i++) {
                (*data)[i] = (int16_t)((data8[i] - 128) * 256);
            }
        } else {
            // ステレオ → モノラル変換
            for (int i = 0; i < samples; i++) {
                int left = (data8[i * 2] - 128) * 256;
                int right = (data8[i * 2 + 1] - 128) * 256;
                (*data)[i] = (left + right) / 2;
            }
        }
        free(data8);
    }
    
    fio->Fclose();
    delete fio;
    
    printf("Loaded sound file: %s (%d samples)\n", name, samples);
}

void OSD::free_sound_file(int id, int16_t **data)
{
    if (data && *data) {
        free(*data);
        *data = nullptr;
    }
}

#if ENABLE_DUAL_SOUND_SYSTEM
// COREAUDIO_SOUND class implementation
COREAUDIO_SOUND::~COREAUDIO_SOUND() {
    if (audioUnit != NULL) {
        AudioOutputUnitStop(audioUnit);
        AudioUnitUninitialize(audioUnit);
        AudioComponentInstanceDispose(audioUnit);
        audioUnit = NULL;
        printf("Core Audio unit disposed\n");
    }
}

OSStatus COREAUDIO_SOUND::createAudioUnit(int sampleRate) {
    OSStatus result = noErr;
    
    // Clear ring buffer
    memset(soundBuffer, 0, sizeof(soundBuffer));
    inputSoundBufferPos = 0;
    outputSoundBufferPos = 0;
    inputLoopCount = 0;
    outputLoopCount = 0;
    
    // Describe the output unit
    AudioComponentDescription outputDescription = {};
    outputDescription.componentType = kAudioUnitType_Output;
#if TARGET_OS_OSX
    outputDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#else
    outputDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#endif
    outputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    // Find the component
    AudioComponent outputComponent = AudioComponentFindNext(NULL, &outputDescription);
    if (outputComponent == NULL) {
        printf("Failed to find audio component\n");
        return -1;
    }
    
    // Create the audio unit
    result = AudioComponentInstanceNew(outputComponent, &audioUnit);
    if (result != noErr) {
        printf("Failed to create audio unit: %d\n", (int)result);
        return result;
    }
    
    // Configure the audio unit
    AudioStreamBasicDescription audioFormat = {};
    audioFormat.mSampleRate = sampleRate;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mChannelsPerFrame = 2; // Stereo
    audioFormat.mBitsPerChannel = 16;
    audioFormat.mBytesPerFrame = (audioFormat.mBitsPerChannel * audioFormat.mChannelsPerFrame) / 8;
    audioFormat.mBytesPerPacket = audioFormat.mBytesPerFrame * audioFormat.mFramesPerPacket;
    
    result = AudioUnitSetProperty(audioUnit,
                                 kAudioUnitProperty_StreamFormat,
                                 kAudioUnitScope_Input,
                                 0,
                                 &audioFormat,
                                 sizeof(audioFormat));
    if (result != noErr) {
        printf("Failed to set audio format: %d\n", (int)result);
        AudioComponentInstanceDispose(audioUnit);
        audioUnit = NULL;
        return result;
    }
    
    // Note: Render callback will be set by AudioQueue system
    // This AudioUnit is just for compatibility structure
    
    printf("Core Audio unit created successfully\n");
    return noErr;
}

OSStatus COREAUDIO_SOUND::startAudioUnit() {
    // AudioUnit start is handled by AudioQueue system
    printf("Core Audio unit start (handled by AudioQueue)\n");
    return noErr;
}

OSStatus COREAUDIO_SOUND::stopAudioUnit() {
    // AudioUnit stop is handled by AudioQueue system
    printf("Core Audio unit stop (handled by AudioQueue)\n");
    return noErr;
}

#endif // ENABLE_DUAL_SOUND_SYSTEM