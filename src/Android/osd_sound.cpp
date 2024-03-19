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

void OSD::reset_sound(){
    for(int index= 0;index <  sound_samples * sizeof(uint16_t);index++){
        oboeSound->soundBuffer[index] = 0;
    }
    oboeSound->inputSoundBufferPos = sound_samples * sizeof(uint16_t);
    oboeSound->outputSoundBufferPos = 0;
}

oboe::DataCallbackResult OBOESOUND::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    if(inputSoundBufferPos <= outputSoundBufferPos){
        return oboe::DataCallbackResult::Continue;
    }
#if !defined(__ANDROID__)
    int32_t channelCount = oboeStream->getChannelCount();
#endif
    int32_t bufferSize = oboeStream->getBufferSizeInFrames();
    int writeBufferSize = 0;
    writeBufferSize = inputSoundBufferPos - outputSoundBufferPos;

//    LOGI("IN:%d OUT %d WRITE:%d BUFFER:%d",inputSoundBufferPos, outputSoundBufferPos, writeBufferSize,bufferSize );

    if(writeBufferSize > bufferSize){
        writeBufferSize = bufferSize;
    }
    int16_t * intData = (int16_t*)audioData;
    for (int i = 0; i < writeBufferSize; i++) {
        if(soundBuffer[outputSoundBufferPos] <= INT_MAX / 2) {
            intData[i] = (int16_t) soundBuffer[outputSoundBufferPos] / 2;
        }else{
            intData[i] = (int16_t)(-soundBuffer[outputSoundBufferPos] / 2);
        }
        outputSoundBufferPos++;
        if(outputSoundBufferPos >= SOUND_BUFFER_LENGTH){
            break;
        }
    }

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