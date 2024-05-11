/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 sound ]
*/

#include "osd.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

bool buffer_remaining_copy = 0;

void OSD::update_sound(int* extra_frames) {
#ifdef ENABLE_SOUND
    if (!soundEnable) {
        return;
    }
#endif
}

void OSD::reset_sound(){
}

void OSD::initialize_sound(int rate, int samples){
#ifdef ENABLE_SOUND
    //LOGI("Rate:%d Samples:%d",rate, samples);
#endif
}
void OSD::release_sound(){
    //LOGI("Release sound");
    sound_available = false;
}
