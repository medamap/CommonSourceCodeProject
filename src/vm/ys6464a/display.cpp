/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ display ]
*/

#include "display.h"

static const int pat_7seg_led[40][28] = {
	{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
	{0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,0,0,6,6,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,2},
	{0,0,0,0,0,6,6,6,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,2,2},
	{0,0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2},
	{0,0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2},
	{0,0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2},
	{0,0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0},
	{0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0},
	{0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0},
	{0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0},
	{0,0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0},
	{0,0,0,6,6,6,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,2,2,2,0,0},
	{0,0,0,6,6,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,2,2,0,0},
	{0,0,5,5,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,3,3,0,0,0},
	{0,0,5,5,5,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,3,3,3,0,0,0},
	{0,0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0},
	{0,0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0},
	{0,0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0},
	{0,0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{0,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0},
	{5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0,0},
	{5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0,0},
	{5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0,0},
	{5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0,0},
	{5,5,5,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,3,3,3,0,0,0,0,0},
	{5,5,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,3,3,0,8,8,8,0},
	{0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,8,8,8,8,8},
	{0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,8,8,8,0},
};

void DISPLAY::initialize()
{
	memset(seg, 0, sizeof(seg));
	pb = pc = 0;
	
	// register event
	register_vline_event(this);
}

void DISPLAY::event_vline(int v, int clock)
{
	if(!v) {
		memset(seg, 0, sizeof(seg));
	}
	switch(pc & 0xf0) {
	case 0x80:
	case 0x90:
	case 0xa0:
	case 0xb0:
	case 0xc0:
	case 0xd0:
		for(int i = 0; i < 8; i++) {
			if(~pb & (1 << i)) {
				seg[(pc >> 4) & 7][i]++;
			}
		}
		break;
	}
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_DISPLAY_PORT_B) {
		pb = data;
	} else if(id == SIG_DISPLAY_PORT_C) {
		pc = data & 0xf0;
	}
}

void DISPLAY::draw_screen()
{
	// draw 7-seg LEDs
	scrntype_t col_h, col_l;
	scrntype_t col[9];
	
	col_h = RGB_COLOR(255, 0, 0);
	col_l = RGB_COLOR(107, 0, 0);
	col[0] = RGB_COLOR(38, 8, 0);
	
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 8; j++) {
			col[j + 1] = (seg[i][j] > 8) ? col_h : col_l;
		}
		for(int y = 0; y < 40; y++) {
			scrntype_t* dest = emu->get_screen_buffer(vm_ranges[i].y + y) + vm_ranges[i].x;
			for(int x = 0; x < 28; x++) {
				dest[x] = col[pat_7seg_led[y][x]];
			}
		}
	}
}

#define STATE_VERSION	1

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(&seg[0][0], sizeof(seg), 1);
	state_fio->StateValue(pb);
	state_fio->StateValue(pc);
	return true;
}

