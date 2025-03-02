/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]

  	[for Android]
	Modify : @shikarunochi
	Date   : 2020.06.01-
*/

#include "osd.h"

void OSD::initialize(int rate, int samples)
{
    LOGI("initialize rate:%d samples:%d", rate, samples);
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#ifdef USE_SOCKET
    initialize_socket();
#endif
#ifdef USE_MIDI
    LOGI("initialize_midi");
    initialize_midi();
#endif
}

void OSD::release()
{
	release_input();
	//release_screen();
	release_sound();
#ifdef USE_MIDI
    LOGI("release_midi");
    release_midi();
#endif
}

void OSD::power_off()
{
}

void OSD::suspend()
{
}

void OSD::restore()
{
}

void OSD::lock_vm()
{
	lock_count++;
}

void OSD::unlock_vm()
{
	if(--lock_count <= 0) {
		force_unlock_vm();
	}
}

void OSD::force_unlock_vm()
{
	lock_count = 0;
}

//void OSD::sleep(uint32_t ms)
//{
//	//Sleep(ms);
//	sleep(ms);
//}

void convertUTF8(char *src,char *desc,int length);

#ifdef USE_DEBUGGER

void OSD::start_waiting_in_debugger()
{
}

void OSD::finish_waiting_in_debugger()
{
}

void OSD::process_waiting_in_debugger()
{
}
#endif

