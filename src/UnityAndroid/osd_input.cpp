/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]
*/

#include "osd.h"
#include "../fifo.h"

#define get_joy_range(min_value, max_value, lo_value, hi_value) \
{ \
	float center = (min_value + max_value) / 2.0f; \
	lo_value = ((center + min_value) / 2.0f); \
	hi_value = ((center + max_value) / 2.0f); \
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

#ifdef USE_MOUSE
    // mouse emulation is disabled
    mouse_enabled = false;
#endif

    key_shift_pressed = key_shift_released = false;
    key_caps_locked = false;

    lost_focus = false;

    //機種ごとのカスタマイズ
#if defined(_MZ700) || defined(_MZ1500)
    androidToAndroidToVk[111][0] = 8; //ESC → BS([BREAK]KEY in MZ-700/1500Emulator)
#endif

}

void OSD::initialize_joystick() {
#ifdef USE_JOYSTICK
    // initialize joysticks
    joy_num = 4; //joyGetNumDevs();
    for(int i = 0; i < joy_num && i < 4; i++) {
        joy_caps[i].wNumAxes = 0;
        int axisNum = 0;
        for (int j = 0; j < 10; j++) {
            if (input_joy_info[i*32 + j*3 + 1] < 0.0f && input_joy_info[i*32 + j*3 + 2] > 0.0f) {
                joy_caps[i].wNumAxes++;
                switch (axisNum) {
                    case 0:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwXposLo, joy_caps[i].dwXposHi);
                        break;
                    case 1:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwYposLo, joy_caps[i].dwYposHi);
                        break;
                    case 2:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwZposLo, joy_caps[i].dwZposHi);
                        break;
                    case 3:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwRposLo, joy_caps[i].dwRposHi);
                        break;
                    case 4:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwUposLo, joy_caps[i].dwUposHi);
                        break;
                    case 5:
                        get_joy_range(input_joy_info[i*32 + j*3 + 1], input_joy_info[i*32 + j*3 + 2], joy_caps[i].dwVposLo, joy_caps[i].dwVposHi);
                        break;
                }
                axisNum++;
            }
        }
        joy_caps[i].dwButtonsMask = (1 << min(16, 16)) - 1; // ボタン数を16で固定する
        joy_caps[i].device_id = input_joy_info[i*32 + 31]; // デバイスID
    }
#endif
}

void OSD::release_input()
{
#ifdef USE_MOUSE
    // release mouse
    if(mouse_enabled) {
        disable_mouse();
    }
#endif
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

#ifdef USE_JOYSTICK
    // update joystick status
    memset(joy_status, 0, sizeof(joy_status));

    for(int i = 0; i < joy_num && i < 4; i++) {
        joy_status[i] = 0;

        if(joy_caps[i].wNumAxes >= 2) {
            if(input_joy_status[i*6 + 1] < joy_caps[i].dwYposLo) joy_status[i] |= 0x00000001;	// up
            if(input_joy_status[i*6 + 1] > joy_caps[i].dwYposHi) joy_status[i] |= 0x00000002;	// down
        }
        if(joy_caps[i].wNumAxes >= 1) {
            if(input_joy_status[i*6 + 0] < joy_caps[i].dwXposLo) joy_status[i] |= 0x00000004;	// left
            if(input_joy_status[i*6 + 0] > joy_caps[i].dwXposHi) joy_status[i] |= 0x00000008;	// right
        }
        if(joy_caps[i].wNumAxes >= 3) {
            if(input_joy_status[i*6 + 2] < joy_caps[i].dwZposLo) joy_status[i] |= 0x00100000;
            if(input_joy_status[i*6 + 2] > joy_caps[i].dwZposHi) joy_status[i] |= 0x00200000;
        }
        if(joy_caps[i].wNumAxes >= 4) {
            if(input_joy_status[i*6 + 3] < joy_caps[i].dwRposLo) joy_status[i] |= 0x00400000;
            if(input_joy_status[i*6 + 3] > joy_caps[i].dwRposHi) joy_status[i] |= 0x00800000;
        }
        if(joy_caps[i].wNumAxes >= 5) {
            if(input_joy_status[i*6 + 4] < joy_caps[i].dwUposLo) joy_status[i] |= 0x01000000;
            if(input_joy_status[i*6 + 4] > joy_caps[i].dwUposHi) joy_status[i] |= 0x02000000;
        }
        if(joy_caps[i].wNumAxes >= 6) {
            if(input_joy_status[i*6 + 5] < joy_caps[i].dwVposLo) joy_status[i] |= 0x04000000;
            if(input_joy_status[i*6 + 5] > joy_caps[i].dwVposHi) joy_status[i] |= 0x08000000;
        }
        joy_status[i] |= input_joy_button[i];
        //if (joy_status[i] > 0) { LOGI("joy_status[%d] = %08x", i, joy_status[i]); }
    }
    if(config.use_joy_to_key) {
        int status[256] = {0};
        if(config.joy_to_key_type == 0) {
            // cursor key
            static const int vk[] = {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT};
            for(int i = 0; i < 4; i++) {
                if(joy_status[0] & (1 << i)) {
                    status[vk[i]] = 1;
                }
            }
        } else if(config.joy_to_key_type == 1) {
            // numpad key (4-directions)
            static const int vk[] = {VK_NUMPAD8, VK_NUMPAD2, VK_NUMPAD4, VK_NUMPAD6};
            for(int i = 0; i < 4; i++) {
                if(joy_status[0] & (1 << i)) {
                    status[vk[i]] = 1;
                }
            }
        } else if(config.joy_to_key_type == 2) {
            // numpad key (8-directions)
            switch(joy_status[0] & 0x0f) {
                case 0x02 + 0x04: status[VK_NUMPAD1] = 1; break; // down-left
                case 0x02       : status[VK_NUMPAD2] = 1; break; // down
                case 0x02 + 0x08: status[VK_NUMPAD3] = 1; break; // down-right
                case 0x00 + 0x04: status[VK_NUMPAD4] = 1; break; // left
//			case 0x00       : status[VK_NUMPAD5] = 1; break;
                case 0x00 + 0x08: status[VK_NUMPAD6] = 1; break; // right
                case 0x01 + 0x04: status[VK_NUMPAD7] = 1; break; // up-left
                case 0x01       : status[VK_NUMPAD8] = 1; break; // up
                case 0x01 + 0x08: status[VK_NUMPAD9] = 1; break; // up-right
            }
        }
        if(config.joy_to_key_type == 1 || config.joy_to_key_type == 2) {
            // numpad key
            if(config.joy_to_key_numpad5 && !(joy_status[0] & 0x0f)) {
                status[VK_NUMPAD5] = 1;
            }
        }
        for(int i = 0; i < 16; i++) {
            if(joy_status[0] & (1 << (i + 4))) {
                if(config.joy_to_key_buttons[i] < 0 && -config.joy_to_key_buttons[i] < 256) {
                    status[-config.joy_to_key_buttons[i]] = 1;
                }
            }
        }
        for(int i = 0; i < 256; i++) {
            if(status[i]) {
                if(!joy_to_key_status[i]) {
                    if(!(key_status[i] & 0x80)) {
                        key_down_native(i, false);
                        // do not keep key pressed
                        if(config.joy_to_key_numpad5 && (i >= VK_NUMPAD1 && i <= VK_NUMPAD9)) {
                            key_status[i] = KEY_KEEP_FRAMES;
                        }
                    }
                    joy_to_key_status[i] = true;
                }
            } else {
                if(joy_to_key_status[i]) {
                    if(key_status[i]) {
                        key_up_native(i);
                    }
                    joy_to_key_status[i] = false;
                }
            }
        }
    }
#endif

#ifdef USE_MOUSE
    // update mouse status
    memset(mouse_status, 0, sizeof(mouse_status));

    if(mouse_enabled) {
        mouse_status[0]  = input_mouse_status[0];
        mouse_status[1]  = input_mouse_status[1];
        mouse_status[2]  = input_mouse_status[2];
        input_mouse_status[0] = 0;
        input_mouse_status[1] = 0;
    }
#endif
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
    mouse_enabled = true;
}

void OSD::disable_mouse()
{
    mouse_enabled = false;
}

void OSD::toggle_mouse()
{
    // toggle mouse enable / disable
    if(mouse_enabled) {
        disable_mouse();
    } else {
        enable_mouse();
    }
}
#endif
