/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]

  	[for Android]
	Modify : @shikarunochi
	Date   : 2020.06.01-
*/

#include "osd.h"
#include "../fifo.h"

#define get_joy_range(min_value, max_value, lo_value, hi_value) \
{ \
	uint64_t center = ((uint64_t)min_value + (uint64_t)max_value) / 2; \
	lo_value = (DWORD)((center + (uint64_t)min_value) / 2); \
	hi_value = (DWORD)((center + (uint64_t)max_value) / 2); \
}

void OSD::initialize_input()
{	// initialize status
	memset(key_status, 0, sizeof(key_status));
#ifdef USE_JOYSTICK
	memset(joy_status, 0, sizeof(joy_status));
	memset(joy_to_key_status, 0, sizeof(joy_to_key_status));
#endif
#ifdef USE_MOUSE
	memset(mouse_status, 0, sizeof(mouse_status));
#endif
	key_shift_pressed = key_shift_released = false;
	key_caps_locked = false;

	lost_focus = false;

	//機種ごとのカスタマイズ
#if defined(_MZ700)
    androidToAndroidToVk[111][0] = 8; //ESC → BS([BREAK]KEY in MZ-700Emulator)
#endif

}

void OSD::release_input()
{

}

void OSD::update_input()
{
	if(lost_focus) {
		// we lost key focus so release all pressed keys
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				if(!key_status[i]) {
					vm->key_up(i);
				}
			}
		}
	} else {
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				if(!key_status[i]) {
					vm->key_up(i);
				}
			}
		}
	}
	lost_focus = false;

	// VK_$00 should be 0
	key_status[0] = 0;
}

void OSD::key_down(int code, bool extended, bool repeat)
{
	key_down_native(androidToAndroidToVk[code][0], repeat);
}

void OSD::key_up(int code, bool extended)
{
	key_up_native(androidToAndroidToVk[code][0]);
}

void OSD::key_down_native(int code, bool repeat)
{
#if true
    bool keep_frames = false;

    if(code == 0xf0) {
        code = VK_CAPITAL;
        keep_frames = true;
    } else if(code == 0xf1 || code == 0xf2) {
        code = VK_KANA;
        keep_frames = true;
    } else if(code == 0xf3 || code == 0xf4) {
        code = VK_KANJI;
        keep_frames = true;
    }
//    if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
//        code = keycode_conv[code];
//    }
    if(key_status[code] == 0 || keep_frames) {
        repeat = false;
    }

	//LOGI("keyDown: %d", code);
    key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
#else
    key_status[code] = 0x80;
#endif

    uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];

	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];

	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
			vm->key_down(VK_SHIFT, repeat);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
			vm->key_down(VK_CONTROL, repeat);
		}
	} else if(code == VK_LMENU|| code == VK_RMENU) {
		if(prev_menu == 0 && key_status[VK_MENU] != 0) {
			vm->key_down(VK_MENU, repeat);
		}
	}
	vm->key_down(code, repeat);
}

void OSD::key_up_native(int code)
{
	//LOGI("keyUp: %d", code);
	if(key_status[code] == 0) {
		return;
	}
	if((key_status[code] &= 0x7f) != 0) {
		return;
	}
	vm->key_up(code);

	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];

	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];

	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
			vm->key_up(VK_SHIFT);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
			vm->key_up(VK_CONTROL);
		}
	} else if(code == VK_LMENU || code == VK_RMENU) {
		if(prev_menu != 0 && key_status[VK_MENU] == 0) {
			vm->key_up(VK_MENU);
		}
	}
}

#ifdef USE_MOUSE
void OSD::enable_mouse()
{
}

void OSD::disable_mouse()
{
}

void OSD::toggle_mouse()
{
}
#endif

