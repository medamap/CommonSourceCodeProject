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

#define RING_BUFFER

#if !defined(RING_BUFFER)
void OSD::update_sound(int* extra_frames){
#ifdef ENABLE_SOUND
    if(soundEnable == false){
        return;
    }
    if(oboeSound->inputSoundBufferPos > oboeSound->outputSoundBufferPos){
        return;
    }
    //LOGI("extra_frames:%d" , *extra_frames);
    *extra_frames = 0;
    sound_muted = false;
    if (sound_available) {
        oboeSound->inputSoundBufferPos=0;
        oboeSound->outputSoundBufferPos=0;
        uint16_t *sound_buffer = vm->create_sound(extra_frames);
        int length = sound_samples * sizeof(uint16_t); // stereo
        for(int index = 0;index < length;index++){
            oboeSound->soundBuffer[oboeSound->inputSoundBufferPos] = sound_buffer[index];
            oboeSound->inputSoundBufferPos++;
            if(oboeSound->inputSoundBufferPos >= SOUND_BUFFER_LENGTH){
                break;
            }
        }
    }
#endif
}

#else

void OSD::update_sound(int* extra_frames){
#ifdef ENABLE_SOUND
    if(soundEnable == false){
        return;
    }
    // データ長
    int length = sound_samples * sizeof(uint16_t); // stereo

    int buffer_distance = oboeSound->inputSoundBufferPos - oboeSound->outputSoundBufferPos;
    if (buffer_distance < 0) {
        buffer_distance += SOUND_BUFFER_LENGTH;  // リングバッファのラップアラウンドを考慮
    }

    // 未再生データがデータ長の1/4以上満たされている場合は、書き込みを避ける
    if (buffer_distance > length / 4) {
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
}

#endif

void OSD::reset_sound(){
    for(int index= 0;index <  sound_samples * sizeof(uint16_t);index++){
        oboeSound->soundBuffer[index] = 0;
    }
    oboeSound->inputSoundBufferPos = sound_samples * sizeof(uint16_t);
    oboeSound->outputSoundBufferPos = 0;
}

#if !defined(RING_BUFFER)

oboe::DataCallbackResult OBOESOUND::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    // サウンドバッファの範囲チェック
    if (outputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
        // バッファの範囲外です。適切な処理を行うか、エラーを返します。
        return oboe::DataCallbackResult::Stop;
    }

    if (inputSoundBufferPos <= outputSoundBufferPos) {
        return oboe::DataCallbackResult::Continue;
    }

    int32_t writeBufferSize = inputSoundBufferPos - outputSoundBufferPos;
    if (writeBufferSize > numFrames) {
        writeBufferSize = numFrames;
    }

    int16_t *intData = static_cast<int16_t*>(audioData);

    for (int i = 0; i < writeBufferSize; i++) {
        // SOUND_BUFFER_LENGTH を超えないようにチェック
        if (outputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
            break;  // バッファの範囲外になった場合は処理を終了
        }

        // soundBuffer からのデータ取得と変換
        if(soundBuffer[outputSoundBufferPos] <= INT_MAX / 2) {
            intData[i] = (int16_t) soundBuffer[outputSoundBufferPos] / 2;
        }else{
            intData[i] = (int16_t)(-soundBuffer[outputSoundBufferPos] / 2);
        }

        outputSoundBufferPos++;
    }

    return oboe::DataCallbackResult::Continue;
}

#else

oboe::DataCallbackResult OBOESOUND::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    int16_t *intData = static_cast<int16_t*>(audioData);

    for (int i = 0; i < numFrames; i++) {
        if (outputLoopCount < inputLoopCount ||
            (outputLoopCount == inputLoopCount && outputSoundBufferPos < inputSoundBufferPos)) {
            // soundBuffer からのデータ取得と変換
            if(soundBuffer[outputSoundBufferPos] <= INT_MAX / 2) {
                intData[i] = (int16_t) soundBuffer[outputSoundBufferPos] / 2;
            }else{
                intData[i] = (int16_t)(-soundBuffer[outputSoundBufferPos] / 2);
            }
            outputSoundBufferPos++;
        } else {
            intData[i] = 0; // バッファが追いつかない場合は無音を出力
        }
        if (outputSoundBufferPos >= SOUND_BUFFER_LENGTH) {
            outputSoundBufferPos = 0; // リングバッファ: ポインタをリセット
            outputLoopCount++; // ループカウンタをインクリメント
        }
    }

    return oboe::DataCallbackResult::Continue;
}

#endif

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