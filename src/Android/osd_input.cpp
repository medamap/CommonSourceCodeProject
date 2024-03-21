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

//Androidキーコード→VMキーコード
//http://faq.creasus.net/04/0131/CharCode.html
static uint8_t AndroidKeyCode[] = {
				0, //  AKEYCODE_UNKNOWN 	0
				0, //  AKEYCODE_SOFT_LEFT 	1
				0, //  AKEYCODE_SOFT_RIGHT 	2
				0, //  AKEYCODE_HOME 	3
				0, //  AKEYCODE_BACK 	4
				0, //  AKEYCODE_CALL 	5
				0, //  AKEYCODE_ENDCALL 	6
				48, //  AKEYCODE_0 	7
				49, //  AKEYCODE_1 	8
				50, //  AKEYCODE_2 	9
				51, //  AKEYCODE_3 	10
				52, //  AKEYCODE_4 	11
				53, //  AKEYCODE_5 	12
				54, //  AKEYCODE_6 	13
				55, //  AKEYCODE_7 	14
				56, //  AKEYCODE_8 	15
				57, //  AKEYCODE_9 	16
				0, //  AKEYCODE_STAR 	17
				0, //  AKEYCODE_POUND 	18
				38, //  AKEYCODE_DPAD_UP 	19
				40, //  AKEYCODE_DPAD_DOWN 	20
				37, //  AKEYCODE_DPAD_LEFT 	21
				39, //  AKEYCODE_DPAD_RIGHT 	22
				0, //  AKEYCODE_DPAD_CENTER 	23
				0, //  AKEYCODE_VOLUME_UP 	24
				0, //  AKEYCODE_VOLUME_DOWN 	25
				0, //  AKEYCODE_POWER 	26
				0, //  AKEYCODE_CAMERA 	27
				36, //  AKEYCODE_CLEAR 	28
				65, //  AKEYCODE_A 	29
				66, //  AKEYCODE_B 	30
				67, //  AKEYCODE_C 	31
				68, //  AKEYCODE_D 	32
				69, //  AKEYCODE_E 	33
				70, //  AKEYCODE_F 	34
				71, //  AKEYCODE_G 	35
				72, //  AKEYCODE_H 	36
				73, //  AKEYCODE_I 	37
				74, //  AKEYCODE_J 	38
				75, //  AKEYCODE_K 	39
				76, //  AKEYCODE_L 	40
				77, //  AKEYCODE_M 	41
				78, //  AKEYCODE_N 	42
				79, //  AKEYCODE_O 	43
				80, //  AKEYCODE_P 	44
				81, //  AKEYCODE_Q 	45
				82, //  AKEYCODE_R 	46
				83, //  AKEYCODE_S 	47
				84, //  AKEYCODE_T 	48
				85, //  AKEYCODE_U 	49
				86, //  AKEYCODE_V 	50
				87, //  AKEYCODE_W 	51
				88, //  AKEYCODE_X 	52
			    89, //  AKEYCODE_Y 	53
				90, //  AKEYCODE_Z 	54
				188, //  AKEYCODE_COMMA 	55
				190, //  AKEYCODE_PERIOD 	56
				18, //  AKEYCODE_ALT_LEFT 	57
				18, //  AKEYCODE_ALT_RIGHT 	58
				VK_LSHIFT, //  AKEYCODE_SHIFT_LEFT 	59
				VK_RSHIFT, //  AKEYCODE_SHIFT_RIGHT 	60
				9, //  AKEYCODE_TAB 	61
				32, //  AKEYCODE_SPACE 	62
				0, //  AKEYCODE_SYM 	63
				0, //  AKEYCODE_EXPLORER 	64
				0, //  AKEYCODE_ENVELOPE 	65
				13, //  AKEYCODE_ENTER 	66
				46, //  AKEYCODE_DEL 	67
				0, //  AKEYCODE_GRAVE 	68
				189, //  AKEYCODE_MINUS 	69
				222, //  AKEYCODE_EQUALS 	70
				192, //  AKEYCODE_LEFT_BRACKET 	71->@
				219, //  AKEYCODE_RIGHT_BRACKET 	72 -> [
				221, //  AKEYCODE_BACKSLASH 	73 -> ]
				187, //  AKEYCODE_SEMICOLON 	74
				186, //  AKEYCODE_APOSTROPHE 	75
				191, //  AKEYCODE_SLASH 	76
				192, //  AKEYCODE_AT 	77
				0, //  AKEYCODE_NUM 	78
				0, //  AKEYCODE_HEADSETHOOK 	79
				0, //  AKEYCODE_FOCUS 	80
				0, //  AKEYCODE_PLUS 	81
				0, //  AKEYCODE_MENU 	82
				0, //  AKEYCODE_NOTIFICATION 	83
				0, //  AKEYCODE_SEARCH 	84
				0, //  AKEYCODE_MEDIA_PLAY_PAUSE 	85
				0, //  AKEYCODE_MEDIA_STOP 	86
				0, //  AKEYCODE_MEDIA_NEXT 	87
				0, //  AKEYCODE_MEDIA_PREVIOUS 	88
				0, //  AKEYCODE_MEDIA_REWIND 	89
				0, //  AKEYCODE_MEDIA_FAST_FORWARD 	90
				0, //  AKEYCODE_MUTE 	91
				33, //  AKEYCODE_PAGE_UP 	92
				34, //  AKEYCODE_PAGE_DOWN 	93
				0, //  AKEYCODE_PICTSYMBOLS 	94
				0, //  AKEYCODE_SWITCH_CHARSET 	95
				0, //  AKEYCODE_BUTTON_A 	96
				0, //  AKEYCODE_BUTTON_B 	97
				0, //  AKEYCODE_BUTTON_C 	98
				0, //  AKEYCODE_BUTTON_X 	99
				0, //  AKEYCODE_BUTTON_Y 	100
				0, //  AKEYCODE_BUTTON_Z 	101
				0, //  AKEYCODE_BUTTON_L1 	102
				0, //  AKEYCODE_BUTTON_R1 	103
				0, //  AKEYCODE_BUTTON_L2 	104
				0, //  AKEYCODE_BUTTON_R2 	105
				0, //  AKEYCODE_BUTTON_THUMBL 	106
				0, //  AKEYCODE_BUTTON_THUMBR 	107
				0, //  AKEYCODE_BUTTON_START 	108
				0, //  AKEYCODE_BUTTON_SELECT 	109
				0, //  AKEYCODE_BUTTON_MODE 	110
				VK_ESCAPE, //  AKEYCODE_ESCAPE 	111
				46, //  AKEYCODE_FORWARD_DEL 	112
                VK_LCONTROL, //  AKEYCODE_CTRL_LEFT 	113
                VK_LCONTROL, //  AKEYCODE_CTRL_RIGHT 	114
				20, //  AKEYCODE_CAPS_LOCK 	115
				0, //  AKEYCODE_SCROLL_LOCK 	116
				0, //  AKEYCODE_META_LEFT 	117
				0, //  AKEYCODE_META_RIGHT 	118
				0, //  AKEYCODE_FUNCTION 	119
				0, //  AKEYCODE_SYSRQ 	120
				0, //  AKEYCODE_BREAK 	121
				36, //  AKEYCODE_MOVE_HOME 	122
				35, //  AKEYCODE_MOVE_END 	123
				45, //  AKEYCODE_INSERT 	124
				0, //  AKEYCODE_FORWARD 	125
				0, //  AKEYCODE_MEDIA_PLAY 	126
				0, //  AKEYCODE_MEDIA_PAUSE 	127
				0, //  AKEYCODE_MEDIA_CLOSE 	128
				0, //  AKEYCODE_MEDIA_EJECT 	129
				0, //  AKEYCODE_MEDIA_RECORD 	130
				112, //  AKEYCODE_F1 	131
				113, //  AKEYCODE_F2 	132
				114, //  AKEYCODE_F3 	133
				115, //  AKEYCODE_F4 	134
				116, //  AKEYCODE_F5 	135
				117, //  AKEYCODE_F6 	136
				118, //  AKEYCODE_F7 	137
				119, //  AKEYCODE_F8 	138
				120, //  AKEYCODE_F9 	139
				121, //  AKEYCODE_F10 	140
				122, //  AKEYCODE_F11 	141
				123, //  AKEYCODE_F12 	142
				0, //  AKEYCODE_NUM_LOCK 	143
				96, //  AKEYCODE_NUMPAD_0 	144
				97, //  AKEYCODE_NUMPAD_1 	145
				98, //  AKEYCODE_NUMPAD_2 	146
				99, //  AKEYCODE_NUMPAD_3 	147
				100, //  AKEYCODE_NUMPAD_4 	148
				101, //  AKEYCODE_NUMPAD_5 	149
				102, //  AKEYCODE_NUMPAD_6 	150
				103, //  AKEYCODE_NUMPAD_7 	151
				104, //  AKEYCODE_NUMPAD_8 	152
				105, //  AKEYCODE_NUMPAD_9 	153
				111, //  AKEYCODE_NUMPAD_DIVIDE 	154
				106, //  AKEYCODE_NUMPAD_MULTIPLY 	155
				109, //  AKEYCODE_NUMPAD_SUBTRACT 	156
				107, //  AKEYCODE_NUMPAD_ADD 	157
				110, //  AKEYCODE_NUMPAD_DOT 	158
				0, //  AKEYCODE_NUMPAD_COMMA 	159
				0, //  AKEYCODE_NUMPAD_ENTER 	160
				0, //  AKEYCODE_NUMPAD_EQUALS 	161
				0, //  AKEYCODE_NUMPAD_LEFT_PAREN 	162
				0, //  AKEYCODE_NUMPAD_RIGHT_PAREN 	163
				0, //  AKEYCODE_VOLUME_MUTE 	164
				0, //  AKEYCODE_INFO 	165
				0, //  AKEYCODE_CHANNEL_UP 	166
				0, //  AKEYCODE_CHANNEL_DOWN 	167
				0, //  AKEYCODE_ZOOM_IN 	168
				0, //  AKEYCODE_ZOOM_OUT 	169
				0, //  AKEYCODE_TV 	170
				0, //  AKEYCODE_WINDOW 	171
				0, //  AKEYCODE_GUIDE 	172
				0, //  AKEYCODE_DVR 	173
				0, //  AKEYCODE_BOOKMARK 	174
				0, //  AKEYCODE_CAPTIONS 	175
				0, //  AKEYCODE_SETTINGS 	176
				0, //  AKEYCODE_TV_POWER 	177
				0, //  AKEYCODE_TV_INPUT 	178
				0, //  AKEYCODE_STB_POWER 	179
				0, //  AKEYCODE_STB_INPUT 	180
				0, //  AKEYCODE_AVR_POWER 	181
				0, //  AKEYCODE_AVR_INPUT 	182
				0, //  AKEYCODE_PROG_RED 	183
				0, //  AKEYCODE_PROG_GREEN 	184
				0, //  AKEYCODE_PROG_YELLOW 	185
				0, //  AKEYCODE_PROG_BLUE 	186
				0, //  AKEYCODE_APP_SWITCH 	187
				0, //  AKEYCODE_BUTTON_1 	188
				0, //  AKEYCODE_BUTTON_2 	189
				0, //  AKEYCODE_BUTTON_3 	190
				0, //  AKEYCODE_BUTTON_4 	191
				0, //  AKEYCODE_BUTTON_5 	192
				0, //  AKEYCODE_BUTTON_6 	193
				0, //  AKEYCODE_BUTTON_7 	194
				0, //  AKEYCODE_BUTTON_8 	195
				0, //  AKEYCODE_BUTTON_9 	196
				0, //  AKEYCODE_BUTTON_10 	197
				0, //  AKEYCODE_BUTTON_11 	198
				0, //  AKEYCODE_BUTTON_12 	199
				0, //  AKEYCODE_BUTTON_13 	200
				0, //  AKEYCODE_BUTTON_14 	201
				0, //  AKEYCODE_BUTTON_15 	202
				0, //  AKEYCODE_BUTTON_16 	203
				0, //  AKEYCODE_LANGUAGE_SWITCH 	204
				0, //  AKEYCODE_MANNER_MODE 	205
				0, //  AKEYCODE_3D_MODE 	206
				0, //  AKEYCODE_CONTACTS 	207
				0, //  AKEYCODE_CALENDAR 	208
				0, //  AKEYCODE_MUSIC 	209
				0, //  AKEYCODE_CALCULATOR 	210
				0, //  AKEYCODE_ZENKAKU_HANKAKU 	211
				0, //  AKEYCODE_EISU 	212
				0, //  AKEYCODE_MUHENKAN 	213
				0, //  AKEYCODE_HENKAN 	214
				0, //  AKEYCODE_KATAKANA_HIRAGANA 	215
				220, //  AKEYCODE_YEN 	216    -> \ + |
				226, //  AKEYCODE_RO 	217       -> \ + _
				0, //  AKEYCODE_KANA 	218
				0, //  AKEYCODE_ASSIST 	219
				0, //  AKEYCODE_BRIGHTNESS_DOWN 	220
				0, //  AKEYCODE_BRIGHTNESS_UP 	221
				0, //  AKEYCODE_MEDIA_AUDIO_TRACK 	222
				0, //  AKEYCODE_SLEEP 	223
				0, //  AKEYCODE_WAKEUP 	224
				0, //  AKEYCODE_PAIRING 	225
				0, //  AKEYCODE_MEDIA_TOP_MENU 	226
				0, //  AKEYCODE_11 	227
				0, //  AKEYCODE_12 	228
				0, //  AKEYCODE_LAST_CHANNEL 	229
				0, //  AKEYCODE_TV_DATA_SERVICE 	230
				0, //  AKEYCODE_VOICE_ASSIST 	231
				0, //  AKEYCODE_TV_RADIO_SERVICE 	232
				0, //  AKEYCODE_TV_TELETEXT 	233
				0, //  AKEYCODE_TV_NUMBER_ENTRY 	234
				0, //  AKEYCODE_TV_TERRESTRIAL_ANALOG 	235
				0, //  AKEYCODE_TV_TERRESTRIAL_DIGITAL 	236
				0, //  AKEYCODE_TV_SATELLITE 	237
				0, //  AKEYCODE_TV_SATELLITE_BS 	238
				0, //  AKEYCODE_TV_SATELLITE_CS 	239
				0, //  AKEYCODE_TV_SATELLITE_SERVICE 	240
				0, //  AKEYCODE_TV_NETWORK 	241
				0, //  AKEYCODE_TV_ANTENNA_CABLE 	242
				0, //  AKEYCODE_TV_INPUT_HDMI_1 	243
				0, //  AKEYCODE_TV_INPUT_HDMI_2 	244
				0, //  AKEYCODE_TV_INPUT_HDMI_3 	245
				0, //  AKEYCODE_TV_INPUT_HDMI_4 	246
				0, //  AKEYCODE_TV_INPUT_COMPOSITE_1 	247
				0, //  AKEYCODE_TV_INPUT_COMPOSITE_2 	248
				0, //  AKEYCODE_TV_INPUT_COMPONENT_1 	249
				0, //  AKEYCODE_TV_INPUT_COMPONENT_2 	250
				0, //  AKEYCODE_TV_INPUT_VGA_1 	251
				0, //  AKEYCODE_TV_AUDIO_DESCRIPTION 	252
				0, //  AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP 	253
				0, //  AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN 	254
				0, //  AKEYCODE_TV_ZOOM_MODE 	255
				0, //  AKEYCODE_TV_CONTENTS_MENU 	256
				0, //  AKEYCODE_TV_MEDIA_CONTEXT_MENU 	257
				0, //  AKEYCODE_TV_TIMER_PROGRAMMING 	258
				0, //  AKEYCODE_HELP 	259
				0, //  AKEYCODE_NAVIGATE_PREVIOUS 	260
				0, //  AKEYCODE_NAVIGATE_NEXT 	261
				0, //  AKEYCODE_NAVIGATE_IN 	262
				0, //  AKEYCODE_NAVIGATE_OUT 	263
				0, //  AKEYCODE_STEM_PRIMARY 	264
				0, //  AKEYCODE_STEM_1 	265
				0, //  AKEYCODE_STEM_2 	266
				0, //  AKEYCODE_STEM_3 	267
				0, //  AKEYCODE_DPAD_UP_LEFT 	268
				0, //  AKEYCODE_DPAD_DOWN_LEFT 	269
				0, //  AKEYCODE_DPAD_UP_RIGHT 	270
				0, //  AKEYCODE_DPAD_DOWN_RIGHT 	271
				0, //  AKEYCODE_MEDIA_SKIP_FORWARD 	272
				0, //  AKEYCODE_MEDIA_SKIP_BACKWARD 	273
				0, //  AKEYCODE_MEDIA_STEP_FORWARD 	274
				0, //  AKEYCODE_MEDIA_STEP_BACKWARD 	275
				0, //  AKEYCODE_SOFT_SLEEP 	276
				0, //  AKEYCODE_CUT 	277
				0, //  AKEYCODE_COPY 	278
				0, //  AKEYCODE_PASTE 	279
				0, //  AKEYCODE_SYSTEM_NAVIGATION_UP 	280
				0, //  AKEYCODE_SYSTEM_NAVIGATION_DOWN 	281
				0, //  AKEYCODE_SYSTEM_NAVIGATION_LEFT 	282
				0, //  AKEYCODE_SYSTEM_NAVIGATION_RIGHT 	283
				0, //  AKEYCODE_ALL_APPS 	284
				0, //  AKEYCODE_REFRESH 	285
				0, //  AKEYCODE_THUMBS_UP 	286
				0, //  AKEYCODE_THUMBS_DOWN 	287
				0, //  AKEYCODE_PROFILE_SWITCH 	288
};

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
	AndroidKeyCode[111] = 8; //ESC → BS([BREAK]KEY in MZ-700Emulator)
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
	key_down_native(AndroidKeyCode[code], repeat);
}

void OSD::key_up(int code, bool extended)
{
	key_up_native(AndroidKeyCode[code]);
}

void OSD::key_down_native(int code, bool repeat)
{
	//LOGI("keyDown: %d", code);
	key_status[code] = 0x80;

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

