/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]

  	[for Android]
	Modify : @shikarunochi
	Date   : 2020.06.01-
*/

#ifndef _ANDROID_OSD_H_
#define _ANDROID_OSD_H_

#define  LOG_TAG    "commonProject"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define ENABLE_SOUND 1

#include <android/log.h>
#if !defined(__ANDROID__)
#include "../windows.h"
#endif
#include "../vm/vm.h"
//#include "../emu.h"
#include "../common.h"
#include "../config.h"
#include "../vm/vm_template.h"
#include <jni.h>

#include <oboe/Oboe.h>

#ifdef USE_ZLIB
// relative path from *.vcproj/*.vcxproj, not from this directory :-(
	#if defined(_MSC_VER) && (_MSC_VER >= 1800)
		#ifdef _DEBUG
			#pragma comment(lib, "../src/zlib-1.2.11/vc++2013/debug/zlibstat.lib")
		#else
			#pragma comment(lib, "../src/zlib-1.2.11/vc++2013/release/zlibstat.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "../src/zlib-1.2.11/vc++2008/debug/zlibstat.lib")
		#else
			#pragma comment(lib, "../src/zlib-1.2.11/vc++2008/release/zlibstat.lib")
		#endif
	#endif
#endif

#ifdef USE_SOCKET
#include <winsock.h>
#pragma comment(lib, "wsock32.lib")
#endif

#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#pragma comment(lib, "strmiids.lib")
#include <dshow.h>
//#include <qedit.h>
EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;
EXTERN_C const IID IID_ISampleGrabberCB;
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB( double SampleTime,IMediaSample *pSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE BufferCB( double SampleTime,BYTE *pBuffer,long BufferLen) = 0;
};
EXTERN_C const IID IID_ISampleGrabber;
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( /* [out][in] */ long *pBufferSize,/* [out] */ long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( /* [retval][out] */ IMediaSample **ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback,long WhichMethodToCallback) = 0;
};
#endif
#ifdef USE_MOVIE_PLAYER
class CMySampleGrabberCB : public ISampleGrabberCB {
private:
	VM_TEMPLATE *vm;
public:
	CMySampleGrabberCB(VM_TEMPLATE *vm_ptr)
	{
		vm = vm_ptr;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 2;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if(riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void *) static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
	{
		vm->movie_sound_callback(pBuffer, lBufferSize);
		return S_OK;
	}
};
#endif
#include<string>

#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

#ifdef USE_SOCKET
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#endif

#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
#endif

//#define SUPPORT_WIN32_DLL

#define SCREEN_FILTER_NONE	0
#define SCREEN_FILTER_RGB	1
#define SCREEN_FILTER_RF	2

// check memory leaks
#ifdef _DEBUG
// _malloca is defined in typeinfo.h
#ifdef _malloca
#undef _malloca
#endif
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// osd common

#define OSD_CONSOLE_BLUE	1 // text color contains blue
#define OSD_CONSOLE_GREEN	2 // text color contains green
#define OSD_CONSOLE_RED		4 // text color contains red
#define OSD_CONSOLE_INTENSITY	8 // text color is intensified

typedef struct bitmap_s {
	// common
	inline bool initialized()
	{
		return (lpBmp != NULL);
	}
	inline scrntype_t* get_buffer(int y)
	{
		return lpBmp + width * (height - y - 1);
	}
	int width, height;

	scrntype_t* lpBmp;

} bitmap_t;

class FIFO;
class FILEIO;

class OBOESOUND;

class OSD
{
private:
	int lock_count;

	// input
	void initialize_input();
	void release_input();

	uint8_t keycode_conv[256];
	uint8_t key_status[256];	// windows key code mapping
	uint8_t key_dik[256];
	uint8_t key_dik_prev[256];
	bool key_shift_pressed, key_shift_released;
	bool key_caps_locked;
	bool lost_focus;
	
#ifdef USE_JOYSTICK
	// bit0-3	up,down,left,right
	// bit4-19	button #1-#16
	// bit20-21	z-axis pos
	// bit22-23	r-axis pos
	// bit24-25	u-axis pos
	// bit26-27	v-axis pos
	// bit28-31	pov pos
	uint32_t joy_status[4];
	int joy_num;
	struct {
		UINT wNumAxes;
		DWORD dwXposLo, dwXposHi;
		DWORD dwYposLo, dwYposHi;
		DWORD dwZposLo, dwZposHi;
		DWORD dwRposLo, dwRposHi;
		DWORD dwUposLo, dwUposHi;
		DWORD dwVposLo, dwVposHi;
		DWORD dwButtonsMask;
	} joy_caps[4];
	bool joy_to_key_status[256];
#endif
	
#ifdef USE_MOUSE
	int32_t mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
#endif
	
	// screen
	void initialize_screen();
	void release_screen();
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void release_screen_buffer(bitmap_t *buffer);
#ifdef USE_SCREEN_FILTER
	void apply_rgb_filter_to_screen_buffer(bitmap_t *source, bitmap_t *dest);
	void apply_rgb_filter_x3_y3(bitmap_t *source, bitmap_t *dest);
	void apply_rgb_filter_x3_y2(bitmap_t *source, bitmap_t *dest);
	void apply_rgb_filter_x2_y3(bitmap_t *source, bitmap_t *dest);
	void apply_rgb_filter_x2_y2(bitmap_t *source, bitmap_t *dest);
	void apply_rgb_filter_x1_y1(bitmap_t *source, bitmap_t *dest);
#endif
//#ifdef USE_SCREEN_ROTATEinitialize_screen_buffer
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
//#endif
	void stretch_screen_buffer(bitmap_t *source, bitmap_t *dest);
	bool initialize_d3d9();
	bool initialize_d3d9_surface(bitmap_t *buffer);
	void release_d3d9();
	void release_d3d9_surface();
	void copy_to_d3d9_surface(bitmap_t *buffer);
	int add_video_frames();

	bitmap_t vm_screen_buffer;
#ifdef USE_SCREEN_FILTER
	bitmap_t filtered_screen_buffer;
	bitmap_t tmp_filtered_screen_buffer;
#endif
//#ifdef USE_SCREEN_ROTATE
	bitmap_t rotated_screen_buffer;
//#endif
	bitmap_t stretched_screen_buffer;
	bitmap_t shrinked_screen_buffer;
	bitmap_t video_screen_buffer;
	
	bitmap_t* draw_screen_buffer;
	
	int host_window_width, host_window_height;
	bool host_window_mode;
	int vm_screen_width, vm_screen_height;
	int vm_window_width, vm_window_height;
	int vm_window_width_aspect, vm_window_height_aspect;
	int draw_screen_width, draw_screen_height;
	
	_TCHAR video_file_path[_MAX_PATH];
	int rec_video_fps;
	double rec_video_run_frames;
	double rec_video_frames;


	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// sound
	void initialize_sound(int rate, int samples);
	void release_sound();
	
	int sound_rate, sound_samples;
	bool sound_available, sound_started, sound_muted;
	
	bool sound_first_half;
	
	_TCHAR sound_file_path[_MAX_PATH];
	FILEIO* rec_sound_fio;
	int rec_sound_bytes;
	int rec_sound_buffer_ptr;
	
	// video device
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void initialize_video();
	void release_video();
	
	IGraphBuilder *pGraphBuilder;
	IBaseFilter *pVideoBaseFilter;
	IBaseFilter *pCaptureBaseFilter;
	ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
	ISampleGrabber *pVideoSampleGrabber;
	IBaseFilter *pSoundBaseFilter;
	ISampleGrabber *pSoundSampleGrabber;
	CMySampleGrabberCB *pSoundCallBack;
	IMediaControl *pMediaControl;
	IMediaSeeking *pMediaSeeking;
	IMediaPosition *pMediaPosition;
	IVideoWindow *pVideoWindow;
	IBasicVideo *pBasicVideo;
	IBasicAudio *pBasicAudio;
	bool bTimeFormatFrame;
	bool bVerticalReversed;
	
	bitmap_t direct_show_screen_buffer;
	bitmap_t direct_show_stretch_buffer;
	int direct_show_width, direct_show_height;
	bool direct_show_mute[2];
#endif
#ifdef USE_MOVIE_PLAYER
	double movie_frame_rate;
	int movie_sound_rate;
#endif
#ifdef USE_VIDEO_CAPTURE
	void enum_capture_devs();
	bool connect_capture_dev(int index, bool pin);
	int cur_capture_dev_index;
	int num_capture_devs;
	_TCHAR capture_dev_name[MAX_CAPTURE_DEVS][256];
#endif
	
	// socket
#ifdef USE_SOCKET
	void initialize_socket();
	void release_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
#endif
	
public:
	OSD(struct android_app* state)
	{
		this->state = state;
		lock_count = 0;
	}
	~OSD() {}
	
	// common
	VM_TEMPLATE* vm;
	
	void initialize(int rate, int samples);
	void release();
	void power_off();
	void suspend();
	void restore();
	void lock_vm();
	void unlock_vm();
	bool is_vm_locked()
	{
		return (lock_count != 0);
	}
	void force_unlock_vm();
	void sleep(uint32_t ms);
	
	// common debugger
#ifdef USE_DEBUGGER
	void start_waiting_in_debugger();
	void finish_waiting_in_debugger();
	void process_waiting_in_debugger();
#endif
	
	// common console
	void open_console(const _TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(const _TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer, unsigned int length);
	bool is_console_key_pressed(int vk);
	void close_debugger_console();
	
	// common input
	void update_input();
	void key_down(int code, bool extended, bool repeat);
	void key_up(int code, bool extended);
	void key_down_native(int code, bool repeat);
	void key_up_native(int code);
	void key_lost_focus()
	{
		lost_focus = true;
	}
#ifdef USE_MOUSE
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	bool is_mouse_enabled()
	{
		return mouse_enabled;
	}
#endif
	uint8_t* get_key_buffer()
	{
		return key_status;
	}
#ifdef USE_JOYSTICK
	uint32_t* get_joy_buffer()
	{
		return joy_status;
	}
#endif
#ifdef USE_MOUSE
	int32_t* get_mouse_buffer()
	{
		return mouse_status;
	}
#endif
#ifdef USE_AUTO_KEY
	bool now_auto_key;
#endif
	int exchangeUStoJIS(int code, bool shift);

	// common screen
	double get_window_mode_power(int mode);
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	void set_host_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect);
	void set_vm_screen_lines(int lines);
	int get_vm_window_width()
	{
		return vm_window_width;
	}
	int get_vm_window_height()
	{
		return vm_window_height;
	}
	int get_vm_window_width_aspect()
	{
		return vm_window_width_aspect;
	}
	int get_vm_window_height_aspect()
	{
		return vm_window_height_aspect;
	}
	scrntype_t* get_vm_screen_buffer(int y);
	int draw_screen();
#ifdef ONE_BOARD_MICRO_COMPUTER
	void reload_bitmap()
	{
		first_invalidate = true;
	}
#endif
	void capture_screen();
	bool start_record_video(int fps);
	void stop_record_video();
	void restart_record_video();
	void add_extra_frames(int extra_frames);
	bool now_record_video;
#ifdef USE_SCREEN_FILTER
	bool screen_skip_line;
#endif
	
	// common sound
	void update_sound(int* extra_frames);
	void mute_sound(){};
	void stop_sound(){};
	void start_record_sound(){};
	void stop_record_sound(){};
	void restart_record_sound(){};
	bool now_record_sound;
	void reset_sound();

	// common video device
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void get_video_buffer();
	void mute_video_dev(bool l, bool r);
#endif
#ifdef USE_MOVIE_PLAYER
	bool open_movie_file(const _TCHAR* file_path);
	void close_movie_file();
	void play_movie();
	void stop_movie();
	void pause_movie();
	double get_movie_frame_rate()
	{
		return movie_frame_rate;
	}
	int get_movie_sound_rate()
	{
		return movie_sound_rate;
	}
	void set_cur_movie_frame(int frame, bool relative);
	uint32_t get_cur_movie_frame();
	bool now_movie_play, now_movie_pause;
#endif
#ifdef USE_VIDEO_CAPTURE
	int get_cur_capture_dev_index()
	{
		return cur_capture_dev_index;
	}
	int get_num_capture_devs()
	{
		return num_capture_devs;
	}
	_TCHAR* get_capture_dev_name(int index)
	{
		return capture_dev_name[index];
	}
	void open_capture_dev(int index, bool pin);
	void close_capture_dev();
	void show_capture_dev_filter();
	void show_capture_dev_pin();
	void show_capture_dev_source();
	void set_capture_dev_channel(int ch);
#endif
	
	// common printer
#ifdef USE_PRINTER
//	void create_bitmap(bitmap_t *bitmap, int width, int height);
//	void release_bitmap(bitmap_t *bitmap);
//	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
//	void release_font(font_t *font);
//	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b);
//	void release_pen(pen_t *pen);
//	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b);
//	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
//	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b);
//	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
//	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b);
//	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b);
//	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
#endif
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path);
	
	// common socket
#ifdef USE_SOCKET
	int get_socket(int ch)
	{
		return soc[ch];
	}
	void notify_socket_connected(int ch);
	void notify_socket_disconnected(int ch);
	void update_socket();
	bool initialize_socket_tcp(int ch);
	bool initialize_socket_udp(int ch);
	bool connect_socket(int ch, uint32_t ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_socket_data_tcp(int ch);
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port);
	void send_socket_data(int ch);
	void recv_socket_data(int ch);
#endif
	
	// win32 dependent
	void invalidate_screen();
	bool vista_or_later;

	bitmap_t* getScreenBuffer();

	android_app* state;

	bool soundEnable = false;
	OBOESOUND *oboeSound;
};

#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_KANA 0x15
#define VL_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_KANJI 0x19
#define VK_ESCAPE 0x1B
#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_ACCEPT 0x1E
#define VK_MODECHANGE 0x1F
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
//0xA2 = 162
#define VK_LCONTROL 0xA2
//0xA3 = 163
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define VK_PROCESSKEY 0xE5
#define VK_ATTN 0xF6
#define VK_CRSEL 0xF7
#define VK_EXSEL 0xF8
#define VK_EREOF 0xF9
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC
#define VK_PA1 0xFD
#define VK_OME_CLEAR 0xFE

#define VK_OEM_EQU    0x92
#define VK_OEM_1      0xBA
#define VK_OEM_PLUS   0xBB
#define VK_OEM_COMMA  0xBC
#define VK_OEM_MINUS  0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2      0xBF
#define VK_OEM_3      0xC0
#define VK_OEM_4      0xDB
#define VK_OEM_5      0xDC
#define VK_OEM_6      0xDD
#define VK_OEM_7      0xDE
#define VK_OEM_8      0xD
#define VK_OEM_102    0xE2

static const uint8_t usKeytoJISKey[][2] = {
        {0,0},
        {1,0},
        {2,0},
        {3,0},
        {4,0},
        {5,0},
        {6,0},
//テンキーがある機種の場合、数字キーはテンキー側を入力する。
#if defined(_HAS_TENKEY)
        {144,0}, //0
        {145,0}, //1
        {146,0}, //2
        {147,0}, //3
        {148,0}, //4
        {149,0}, //5
        {150,0}, //6
        {151,0}, //7
        {152,0},  //8
        {153,0},  //9
#else
        {7,0}, //0
        {8,0}, //1
        {9,0}, //2
        {10,0}, //3
        {11,0}, //4
        {12,0}, //5
        {13,0}, //6
        {14,0}, //7
        {15,0},  //8
        {16,0},  //9
#endif
        {17,0},
        {18,0},
        {19,0},
        {20,0},
        {21,0},
        {22,0},
        {23,0},
        {24,0},
        {25,0},
        {26,0},
        {27,0},
        {28,0},
        {29,0},
        {30,0},
        {31,0},
        {32,0},
        {33,0},
        {34,0},
        {35,0},
        {36,0},
        {37,0},
        {38,0},
        {39,0},
        {40,0},
        {41,0},
        {42,0},
        {43,0},
        {44,0},
        {45,0},
        {46,0},
        {47,0},
        {48,0},
        {49,0},
        {50,0},
        {51,0},
        {52,0},
        {53,0},
        {54,0},
        {55,0},
        {56,0},
        {57,0},
        {58,0},
        {59,0},
        {60,0},
        {61,0},
        {62,0},
        {63,0},
        {64,0},
        {65,0},
        {66,0},
        {67,0},
        {68,0},
        {69,0},
		{69,1},//=
        {72,0},//[
        {73,0},//]
        {216,0}, //￥
        {74,0},
        {14,1}, //'
        {76,0},
        {77,0},
        {78,0},
        {79,0},
        {80,0},
        {81,0},
        {82,0},
        {83,0},
        {84,0},
        {85,0},
        {86,0},
        {87,0},
        {88,0},
        {89,0},
        {90,0},
        {91,0},
        {92,0},
        {93,0},
        {94,0},
        {95,0},
        {96,0},
        {97,0},
        {98,0},
        {99,0},
        {100,0},
        {101,0},
        {102,0},
        {103,0},
        {104,0},
        {105,0},
        {106,0},
        {107,0},
        {108,0},
        {109,0},
        {110,0},
        {111,0},
        {112,0},
        {113,0},
        {114,0},
        {115,0},
        {116,0},
        {117,0},
        {118,0},
        {119,0},
        {120,0},
        {121,0},
        {122,0},
        {123,0},
        {124,0},
        {125,0},
        {126,0},
        {127,0},
        {128,0},
        {129,0},
        {130,0},
        {131,0},
        {132,0},
        {133,0},
        {134,0},
        {135,0},
        {136,0},
        {137,0},
        {138,0},
        {139,0},
        {140,0},
        {141,0},
        {142,0},
        {143,0},
        {144,0},
        {145,0},
        {146,0},
        {147,0},
        {148,0},
        {149,0},
        {150,0},
        {151,0},
        {152,0},
        {153,0},
        {154,0},
        {155,0},
        {156,0},
        {157,0},
        {158,0},
        {159,0},
        {160,0},
        {161,0},
        {162,0},
        {163,0},
        {164,0},
        {165,0},
        {166,0},
        {167,0},
        {168,0},
        {169,0},
        {170,0},
        {171,0},
        {172,0},
        {173,0},
        {174,0},
        {175,0},
        {176,0},
        {177,0},
        {178,0},
        {179,0},
        {180,0},
        {181,0},
        {182,0},
        {183,0},
        {184,0},
        {185,0},
        {186,0},
        {187,0},
        {188,0},
        {189,0},
        {190,0},
        {191,0},
        {192,0},
        {193,0},
        {194,0},
        {195,0},
        {196,0},
        {197,0},
        {198,0},
        {199,0},
        {200,0},
        {201,0},
        {202,0},
        {203,0},
        {204,0},
        {205,0},
        {206,0},
        {207,0},
        {208,0},
        {209,0},
        {210,0},
        {211,0},
        {212,0},
        {213,0},
        {214,0},
        {215,0},
        {216,0},
        {217,0},
        {218,0},
        {219,0},
        {220,0},
        {221,0},
        {221,0},
        {223,0},
        {224,0},
        {225,0},
        {226,0},
        {227,0},
        {228,0},
        {229,0},
        {230,0},
        {231,0},
        {232,0},
        {233,0},
        {234,0},
        {235,0},
        {236,0},
        {237,0},
        {238,0},
        {239,0},
        {240,0},
        {241,0},
        {242,0},
        {243,0},
        {244,0},
        {245,0},
        {246,0},
        {247,0},
        {248,0},
        {249,0},
        {250,0},
        {251,0},
        {252,0},
        {253,0},
        {254,0},
        {255,0}
};

static const uint8_t usKeytoJISKeyShift[][2] = {
        {0,1},
        {1,1},
        {2,1},
        {3,1},
        {4,1},
        {5,1},
        {6,1},
        {16,1},//)
        {8,1},
        {71,0},// @
        {10,1},
        {11,1},
        {12,1},
        {70,0}, //^
        {13,1}, //&
        {75,1}, //*
        {15,1},//(
        {17,1},
        {18,1},
        {19,1},
        {20,1},
        {21,1},
        {22,1},
        {23,1},
        {24,1},
        {25,1},
        {26,1},
        {27,1},
        {28,1},
        {29,1},
        {30,1},
        {31,1},
        {32,1},
        {33,1},
        {34,1},
        {35,1},
        {36,1},
        {37,1},
        {38,1},
        {39,1},
        {40,1},
        {41,1},
        {42,1},
        {43,1},
        {44,1},
        {45,1},
        {46,1},
        {47,1},
        {48,1},
        {49,1},
        {50,1},
        {51,1},
        {52,1},
        {53,1},
        {54,1},
        {55,1},
        {56,1},
        {57,1},
        {58,1},
        {59,1},
        {60,1},
        {61,1},
        {62,1},
        {63,1},
        {64,1},
        {65,1},
        {66,1},
        {67,1},
        {70,1}, //~
        {217,1}, //_
        {74,1}, //+
        {71,1},
        {72,1},
        {216,1}, // ￥
        {75,0}, //:
		{9,1}, //"
        {76,1},
        {77,1},
        {78,1},
        {79,1},
        {80,1},
        {81,1},
        {82,1},
        {83,1},
        {84,1},
        {85,1},
        {86,1},
        {87,1},
        {88,1},
        {89,1},
        {90,1},
        {91,1},
        {92,1},
        {93,1},
        {94,1},
        {95,1},
        {96,1},
        {97,1},
        {98,1},
        {99,1},
        {100,1},
        {101,1},
        {102,1},
        {103,1},
        {104,1},
        {105,1},
        {106,1},
        {107,1},
        {108,1},
        {109,1},
        {110,1},
        {111,1},
        {112,1},
        {113,1},
        {114,1},
        {115,1},
        {116,1},
        {117,1},
        {118,1},
        {119,1},
        {120,1},
        {121,1},
        {122,1},
        {123,1},
        {124,1},
        {125,1},
        {126,1},
        {127,1},
        {128,1},
        {129,1},
        {130,1},
        {131,1},
        {132,1},
        {133,1},
        {134,1},
        {135,1},
        {136,1},
        {137,1},
        {138,1},
        {139,1},
        {140,1},
        {141,1},
        {142,1},
        {143,1},
        {144,1},
        {145,1},
        {146,1},
        {147,1},
        {148,1},
        {149,1},
        {150,1},
        {151,1},
        {152,1},
        {153,1},
        {154,1},
        {155,1},
        {156,1},
        {157,1},
        {158,1},
        {159,1},
        {160,1},
        {161,1},
        {162,1},
        {163,1},
        {164,1},
        {165,1},
        {166,1},
        {167,1},
        {168,1},
        {169,1},
        {170,1},
        {171,1},
        {172,1},
        {173,1},
        {174,1},
        {175,1},
        {176,1},
        {177,1},
        {178,1},
        {179,1},
        {180,1},
        {181,1},
        {182,1},
        {183,1},
        {184,1},
        {185,1},
        {186,1},
        {187,1},
        {188,1},
        {189,1},
        {190,1},
        {191,1},
        {192,1},
        {193,1},
        {194,1},
        {195,1},
        {196,1},
        {197,1},
        {198,1},
        {199,1},
        {200,1},
        {201,1},
        {202,1},
        {203,1},
        {204,1},
        {205,1},
        {206,1},
        {207,1},
        {208,1},
        {209,1},
        {210,1},
        {211,1},
        {212,1},
        {213,1},
        {214,1},
        {215,1},
        {216,1},
        {217,1},
        {218,1},
        {219,1},
        {220,1},
        {221,1},
        {222,1},
        {223,1},
        {224,1},
        {225,1},
        {226,1},
        {227,1},
        {228,1},
        {229,1},
        {230,1},
        {231,1},
        {232,1},
        {233,1},
        {234,1},
        {235,1},
        {236,1},
        {237,1},
        {238,1},
        {239,1},
        {240,1},
        {241,1},
        {242,1},
        {243,1},
        {244,1},
        {245,1},
        {246,1},
        {247,1},
        {248,1},
        {249,1},
        {250,1},
        {251,1},
        {252,1},
        {253,1},
        {254,1},
        {255,1}
};

struct BitmapData{
	int width;
	int height;
	uint16_t *bmpImage;
};

enum systemIconType { SYSTEM_RESET = 0 , SYSTEM_SCREEN , SYSTEM_SOUND, SYSTEM_PCG , SYSTEM_ICON_MAX};
enum FileSelectType { FILE_SELECT_NONE = -1, FLOPPY_DISK = 0 , CASETTE_TAPE , CARTRIDGE, QUICK_DISK, FILE_SELECT_TYPE_MAX};
enum SelectDialogMode {MEDIA_SELECT = 0, DISK_BANK_SELECT = 1, BOOT_MODE_SELECT = 2, EXIT_EMULATOR = 3};

enum ScreenSize {SCREEN_SIZE_JUST = 0, SCREEN_SIZE_MAX = 1, SCREEN_SIZE_1 = 2, SCREEN_SIZE_2 = 3,  SCREEN_SIZE_SPECIAL = 4};

#define MAX_FILE_SELECT_ICON 10
struct FileSelectIconData{
	FileSelectType fileSelectType;
	int driveNo;
};

struct DeviceInfo{
    int width;
    int height;
};

class OBOESOUND:public oboe::AudioStreamCallback{

public:
	OBOESOUND()
	{
	}
	~OBOESOUND() {
	}

	#define SOUND_BUFFER_LENGTH 4800*100*2

	uint16_t soundBuffer[SOUND_BUFFER_LENGTH];

	int inputSoundBufferPos = 0;
	int outputSoundBufferPos = 0;

	oboe::ManagedStream mStream;
	oboe::Result createPlaybackStream(oboe::AudioStreamBuilder builder, int sampleRate);
	oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames);

};


#endif
