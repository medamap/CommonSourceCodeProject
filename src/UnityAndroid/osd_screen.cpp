/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 screen ]
*/

#include "osd.h"

#define REC_VIDEO_SUCCESS	1
#define REC_VIDEO_FULL		2
#define REC_VIDEO_ERROR		3

void OSD::initialize_screen()
{
    vm_screen_width = SCREEN_WIDTH;
    vm_screen_height = SCREEN_HEIGHT;
    vm_window_width = WINDOW_WIDTH;
    vm_window_height = WINDOW_HEIGHT;
    vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
    vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;

    memset(&vm_screen_buffer, 0, sizeof(bitmap_t));
    initialize_screen_buffer(&vm_screen_buffer, vm_screen_width , vm_screen_height , 0);

    draw_screen_buffer = &vm_screen_buffer;
}

double OSD::get_window_mode_power(int mode)
{
    if(mode + WINDOW_MODE_BASE == 2) {
        return 1.5;
    } else if(mode + WINDOW_MODE_BASE > 2) {
        return mode + WINDOW_MODE_BASE - 1;
    }
    return mode + WINDOW_MODE_BASE;
}

int OSD::get_window_mode_width(int mode)
{
    return 640;
}

int OSD::get_window_mode_height(int mode)
{
    return 480;
}
void OSD::set_host_window_size(int window_width, int window_height, bool window_mode)
{
}

void OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
}

void OSD::set_vm_screen_lines(int lines)
{
//	set_vm_screen_size(vm_screen_width, lines, vm_window_width, vm_window_height, vm_window_width_aspect, vm_screen_height);
}

scrntype_t* OSD::get_vm_screen_buffer(int y)
{
    return vm_screen_buffer.get_buffer(y);
}




int OSD::draw_screen()
{
    vm->draw_screen();
    return 1;
}

bitmap_t* OSD::getScreenBuffer(){
    draw_screen_buffer = &vm_screen_buffer;
    return draw_screen_buffer;
}

void OSD::invalidate_screen()
{
}

void OSD::initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode)
{
    release_screen_buffer(buffer);
    buffer->height = height;
    buffer->width = width;
    buffer->lpBmp = (scrntype_t*)malloc(width * height * sizeof(scrntype_t));
}

void OSD::release_screen_buffer(bitmap_t *buffer)
{
    free(buffer->lpBmp);
}

#ifdef USE_SCREEN_FILTER
#define _3_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _5_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _8_8(v) (((v) * 180) >> 8)

static uint8_t r0[2048], g0[2048], b0[2048], t0[2048];
static uint8_t r1[2048], g1[2048], b1[2048];

void OSD::apply_rgb_filter_to_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::apply_rgb_filter_x3_y3(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::apply_rgb_filter_x3_y2(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::apply_rgb_filter_x2_y3(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::apply_rgb_filter_x2_y2(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::apply_rgb_filter_x1_y1(bitmap_t *source, bitmap_t *dest)
{
}
#endif

//#ifdef USE_SCREEN_ROTATE
void OSD::rotate_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
}
//#endif

void OSD::stretch_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
}

#if defined(_RGB555)
#define D3DFMT_TMP D3DFMT_X1R5G5B5
#elif defined(_RGB565)
#define D3DFMT_TMP D3DFMT_R5G6B5
#elif defined(_RGB888)
#define D3DFMT_TMP D3DFMT_X8R8G8B8
#endif

bool OSD::initialize_d3d9()
{
    return true;
}

bool OSD::initialize_d3d9_surface(bitmap_t *buffer)
{
    return true;
}

void OSD::release_d3d9()
{
}

void OSD::release_d3d9_surface()
{
}

void OSD::copy_to_d3d9_surface(bitmap_t *buffer)
{
}

void OSD::capture_screen()
{
}

bool OSD::start_record_video(int fps)
{
    return true;
}

void OSD::stop_record_video()
{
}

void OSD::restart_record_video()
{
}

void OSD::add_extra_frames(int extra_frames)
{
}

unsigned __stdcall rec_video_thread(void *lpx)
{
    return 0;
}

int OSD::add_video_frames()
{
    return 0;
}

//#ifdef USE_PRINTER
//void OSD::create_bitmap(bitmap_t *bitmap, int width, int height)
//{
//	initialize_screen_buffer(bitmap, width, height, HALFTONE);
//}

//void OSD::release_bitmap(bitmap_t *bitmap)
//{
//	release_screen_buffer(bitmap);
//}

//void OSD::create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic)
//{
//}

//void OSD::release_font(font_t *font)
//{
//}

//void OSD::create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)
//{
//}

//void OSD::release_pen(pen_t *pen)
//{
//}

//void OSD::clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)
//{
//}

//int OSD::get_text_width(bitmap_t *bitmap, font_t *font, const char *text)
//{
//}

//void OSD::draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b)
//{
//}

//void OSD::draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)
//{
//}

//void OSD::draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
//{
//}

//void OSD::draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)
//{
//}

//void OSD::stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height)
//{
//}
//#endif


void OSD::write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)
{
}

