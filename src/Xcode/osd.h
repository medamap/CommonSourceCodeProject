/*
	Skelton for retropc emulator

	Author : MedamaP
	Date   : 2025.05.06

*/

#ifndef _OSD_H_
#define _OSD_H_

// 二重インクルード防止
#pragma once

// 標準 C ライブラリ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

// UNIX / macOS 系ライブラリ
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// CSCP エミュレータ共通ヘッダ
#include "../common.h"
#include "../config.h"
#include "../vm/vm_template.h"
#include "../vm/vm.h"

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
#if defined(__ANDROID__) || defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <winsock.h>
#pragma comment(lib, "wsock32.lib")
#endif
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

#if defined(_USE_OPENGL_ES20) || defined(_USE_OPENGL_ES30)
#if defined(_USE_OPENGL_ES20)
#include <GLES2/gl2.h>
#elif defined(_USE_OPENGL_ES30)
#include <GLES3/gl3.h>
#endif // _USE_OPENGL_ES30
#include <EGL/egl.h>
#include <android/sensor.h>
#include <android/native_window.h>
#include <cmath>
#endif // _USE_OPENGL_ES20 || _USE_OPENGL_ES30

#include<string>
// #include <android/keycodes.h>
#include "Android/windows_define.h"

#ifdef USE_SOCKET
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000

// POSIX 標準では無効なソケットとエラーは -1 で表される
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
#endif

//#define SUPPORT_WIN32_DLL

#define SCREEN_FILTER_NONE	        0
#define SCREEN_FILTER_BLUR	        1
#define SCREEN_FILTER_RGB	        2
#define SCREEN_FILTER_GREEN         3

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

// Forward declaration for Core Audio sound system
class COREAUDIO_SOUND;

#ifdef USE_MIDI
#include <thread>
#include <mutex>

typedef struct midi_thread_params_s {
    FIFO *send_buffer;
    FIFO *recv_buffer;
    bool terminate;
} midi_thread_params_t;

extern midi_thread_params_t midi_thread_params;
#endif

typedef struct pen_s {
    // common
    inline bool initialized()
    {
        return false;
        //return (hPen != NULL);
    }
    int width;
    uint8_t r, g, b;
    // win32 dependent
    //HPEN hPen;
} pen_t;

// Medamap and Claude: font_s is already defined in common.h

class OSD
{
private:
	int lock_count;
    // console
    void initialize_console();
    void release_console();

    //HANDLE hStdIn, hStdOut;
    int console_count;

    void open_telnet(const _TCHAR* title);
    void close_telnet();
    void send_telnet(const char* buffer);

    bool use_telnet, telnet_closed;
    int svr_socket, cli_socket;

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
        UINT device_id;
		UINT wNumAxes;
		float dwXposLo, dwXposHi;
		float dwYposLo, dwYposHi;
		float dwZposLo, dwZposHi;
		float dwRposLo, dwRposHi;
		float dwUposLo, dwUposHi;
		float dwVposLo, dwVposHi;
		DWORD dwButtonsMask;
	} joy_caps[4];
	bool joy_to_key_status[256];
    float input_joy_info[32*4];
    float input_joy_status[6*4];
    uint32_t input_joy_button[4];
#endif

#ifdef USE_MOUSE
	int32_t mouse_status[3];	// x, y, button (b0 = left, b1 = right)
    int32_t input_mouse_status[3];	// x, y, button (b0 = left, b1 = right)
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

    //midi
#ifdef USE_MIDI
    std::thread midi_thread;
#endif

public:
	OSD()
	{
		lock_count = 0;
    }
	~OSD() {}
	
	// common
	VM_TEMPLATE* vm;
	
#ifdef __APPLE__
	// Metal rendering
	void* metal_view;  // CSCPMetalView instance
	bool metal_initialized;
#endif
	
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
	void sleep(uint32_t ms) {
        //Sleep(ms);
        usleep(ms * 1000);
    }
	
	// common debugger
#ifdef USE_DEBUGGER
	void start_waiting_in_debugger();
	void finish_waiting_in_debugger();
	void process_waiting_in_debugger();
#endif
	
	// common console
	//void open_console(const _TCHAR* title);
    void open_console(int width, int height, const _TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(const _TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer, unsigned int length);
	bool is_console_key_pressed(int vk);
    bool is_console_closed();
	void close_debugger_console();

    // input
    void initialize_joystick();

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
		// キーバッファの参照は非常に頻繁なので、一定間隔でのみログ出力
		static int call_count = 0;
		static bool first_call = true;
		if (first_call) {
			printf("[DEBUG] get_key_buffer 初回呼び出し\n");
			first_call = false;
		}
		if (++call_count % 300 == 0) { // 60fpsで約5秒ごと
			int pressed_keys = 0;
			for (int i = 0; i < 256; i++) {
				if (key_status[i] & 0x80) pressed_keys++;
			}
			printf("[DEBUG] get_key_buffer 呼び出し: %d個のキーが押下中\n", pressed_keys);
		}
		return key_status;
	}
#ifdef USE_JOYSTICK
	uint32_t* get_joy_buffer()
	{
		return joy_status;
	}
    float* get_input_joy_info()
    {
        return input_joy_info;
    }
    float* get_input_joy_status()
    {
        return input_joy_status;
    }
    uint32_t* get_input_joy_button()
    {
        return input_joy_button;
    }
#endif
#ifdef USE_MOUSE
	int32_t* get_mouse_buffer()
	{
		return mouse_status;
	}
    int32_t* get_input_mouse_buffer()
    {
        return input_mouse_status;
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
	void mute_sound();
	void unmute_sound();
	void stop_sound();
	void start_sound();
	void set_sound_volume(int volume);
	void supply_sound_data(int16_t* data, int samples);
	void start_record_sound(){};
	void stop_record_sound(){};
	void restart_record_sound(){};
	bool now_record_sound;
	void reset_sound();
	
	// Phase 1: Core Audio互換性機能
	void reinitialize_sound_if_needed(int new_rate, int new_samples);
	
	// sound file loading (for noise etc.)
	void load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size);
	void free_sound_file(int id, int16_t **data);
	
	// sound member variables
	int sound_rate, sound_samples;
	bool sound_available, sound_started, sound_muted;
	
	// Core Audio sound system
	COREAUDIO_SOUND *coreAudioSound = nullptr;

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
	// printer-specific functions here
#endif
	
	// bitmap functions (needed by all machines)
	void create_bitmap(bitmap_t *bitmap, int width, int height);
	void release_bitmap(bitmap_t *bitmap);
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
	void release_font(font_t *font);
	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b);
	void release_pen(pen_t *pen);
	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b);
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b);
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b);
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b);
	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
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

    // common midi
#ifdef USE_MIDI
    void initialize_midi();
    void release_midi();
    void send_to_midi(uint8_t data);
    bool recv_from_midi(uint8_t *data);
#endif

	// win32 dependent
	void invalidate_screen();
	bool vista_or_later;

	bitmap_t* getScreenBuffer();


	bool soundEnable = false;
};

//Androidキーコード→Asciiコード
//http://faq.creasus.net/04/0131/CharCode.html
static const uint8_t AndroidToAsciiCode[][2] = {
        { 0, 0 }, //  AKEYCODE_UNKNOWN 	0
        { 0, 0 }, //  AKEYCODE_SOFT_LEFT 	1
        { 0, 0 }, //  AKEYCODE_SOFT_RIGHT 	2
        { 0, 0 }, //  AKEYCODE_HOME 	3
        { 0, 0 }, //  AKEYCODE_BACK 	4
        { 0, 0 }, //  AKEYCODE_CALL 	5
        { 0, 0 }, //  AKEYCODE_ENDCALL 	6
        { 0x30, 0x29 }, //  AKEYCODE_0 	7
        { 0x31, 0x21 }, //  AKEYCODE_1 	8
        { 0x32, 0x40 }, //  AKEYCODE_2 	9
        { 0x33, 0x23 }, //  AKEYCODE_3 	10
        { 0x34, 0x24 }, //  AKEYCODE_4 	11
        { 0x35, 0x25 }, //  AKEYCODE_5 	12
        { 0x36, 0x5e }, //  AKEYCODE_6 	13
        { 0x37, 0x26 }, //  AKEYCODE_7 	14
        { 0x38, 0x2a }, //  AKEYCODE_8 	15
        { 0x39, 0x28 }, //  AKEYCODE_9 	16
        { 0, 0 }, //  AKEYCODE_STAR 	17
        { 0, 0 }, //  AKEYCODE_POUND 	18
        { 0x1e, 0x1e }, //  AKEYCODE_DPAD_UP 	19
        { 0x1f, 0x1f }, //  AKEYCODE_DPAD_DOWN 	20
        { 0x1d, 0x1d }, //  AKEYCODE_DPAD_LEFT 	21
        { 0x1c, 0x1d }, //  AKEYCODE_DPAD_RIGHT 	22
        { 0, 0 }, //  AKEYCODE_DPAD_CENTER 	23
        { 0, 0 }, //  AKEYCODE_VOLUME_UP 	24
        { 0, 0 }, //  AKEYCODE_VOLUME_DOWN 	25
        { 0, 0 }, //  AKEYCODE_POWER 	26
        { 0, 0 }, //  AKEYCODE_CAMERA 	27
        { 0, 0 }, //  AKEYCODE_CLEAR 	28
        { 0x41, 0x61 }, //  AKEYCODE_A 	29
        { 0x42, 0x62 }, //  AKEYCODE_B 	30
        { 0x43, 0x63 }, //  AKEYCODE_C 	31
        { 0x44, 0x64 }, //  AKEYCODE_D 	32
        { 0x45, 0x65 }, //  AKEYCODE_E 	33
        { 0x46, 0x66 }, //  AKEYCODE_F 	34
        { 0x47, 0x67 }, //  AKEYCODE_G 	35
        { 0x48, 0x68 }, //  AKEYCODE_H 	36
        { 0x49, 0x69 }, //  AKEYCODE_I 	37
        { 0x4a, 0x6a }, //  AKEYCODE_J 	38
        { 0x4b, 0x6b }, //  AKEYCODE_K 	39
        { 0x4c, 0x6c }, //  AKEYCODE_L 	40
        { 0x4d, 0x6d }, //  AKEYCODE_M 	41
        { 0x4e, 0x6e }, //  AKEYCODE_N 	42
        { 0x4f, 0x6f }, //  AKEYCODE_O 	43
        { 0x50, 0x70 }, //  AKEYCODE_P 	44
        { 0x51, 0x71 }, //  AKEYCODE_Q 	45
        { 0x52, 0x72 }, //  AKEYCODE_R 	46
        { 0x53, 0x73 }, //  AKEYCODE_S 	47
        { 0x54, 0x74 }, //  AKEYCODE_T 	48
        { 0x55, 0x75 }, //  AKEYCODE_U 	49
        { 0x56, 0x76 }, //  AKEYCODE_V 	50
        { 0x57, 0x77 }, //  AKEYCODE_W 	51
        { 0x58, 0x78 }, //  AKEYCODE_X 	52
        { 0x59, 0x79 }, //  AKEYCODE_Y 	53
        { 0x5a, 0x7a }, //  AKEYCODE_Z 	54
        { 0x2c, 0x3c }, //  AKEYCODE_COMMA 	55
        { 0x2e, 0x3e }, //  AKEYCODE_PERIOD 	56
        { 0, 0 }, //  AKEYCODE_ALT_LEFT 	57
        { 0, 0 }, //  AKEYCODE_ALT_RIGHT 	58
        { 0, 0 }, //  AKEYCODE_SHIFT_LEFT 	59
        { 0, 0 }, //  AKEYCODE_SHIFT_RIGHT 	60
        { 0x09, 0x09 }, //  AKEYCODE_TAB 	61
        { 0x20, 0x20 }, //  AKEYCODE_SPACE 	62
        { 0, 0 }, //  AKEYCODE_SYM 	63
        { 0, 0 }, //  AKEYCODE_EXPLORER 	64
        { 0, 0 }, //  AKEYCODE_ENVELOPE 	65
        { 0x0d, 0x0d }, //  AKEYCODE_ENTER 	66
        { 0x08, 0x08 }, //  AKEYCODE_DEL 	67
        { 0x60, 0x7e }, //  AKEYCODE_GRAVE 	68
        { 0x2d, 0x5f }, //  AKEYCODE_MINUS 	69
        { 0x3d, 0x2b }, //  AKEYCODE_EQUALS 	70
        { 0x5b, 0x7b }, //  AKEYCODE_LEFT_BRACKET 	71
        { 0x5d, 0x7d }, //  AKEYCODE_RIGHT_BRACKET 	72
        { 0x5c, 0x7c }, //  AKEYCODE_BACKSLASH 	73
        { 0x3b, 0x3a }, //  AKEYCODE_SEMICOLON 	74
        { 0x27, 0x22 }, //  AKEYCODE_APOSTROPHE 	75
        { 0x2f, 0x3f }, //  AKEYCODE_SLASH 	76
        { 0x40, 0 }, //  AKEYCODE_AT 	77
        { 0, 0 }, //  AKEYCODE_NUM 	78
        { 0, 0 }, //  AKEYCODE_HEADSETHOOK 	79
        { 0, 0 }, //  AKEYCODE_FOCUS 	80
        { 0, 0 }, //  AKEYCODE_PLUS 	81
        { 0, 0 }, //  AKEYCODE_MENU 	82
        { 0, 0 }, //  AKEYCODE_NOTIFICATION 	83
        { 0, 0 }, //  AKEYCODE_SEARCH 	84
        { 0, 0 }, //  AKEYCODE_MEDIA_PLAY_PAUSE 	85
        { 0, 0 }, //  AKEYCODE_MEDIA_STOP 	86
        { 0, 0 }, //  AKEYCODE_MEDIA_NEXT 	87
        { 0, 0 }, //  AKEYCODE_MEDIA_PREVIOUS 	88
        { 0, 0 }, //  AKEYCODE_MEDIA_REWIND 	89
        { 0, 0 }, //  AKEYCODE_MEDIA_FAST_FORWARD 	90
        { 0, 0 }, //  AKEYCODE_MUTE 	91
        { 0, 0 }, //  AKEYCODE_PAGE_UP 	92
        { 0, 0 }, //  AKEYCODE_PAGE_DOWN 	93
        { 0, 0 }, //  AKEYCODE_PICTSYMBOLS 	94
        { 0, 0 }, //  AKEYCODE_SWITCH_CHARSET 	95
        { 0, 0 }, //  AKEYCODE_BUTTON_A 	96
        { 0, 0 }, //  AKEYCODE_BUTTON_B 	97
        { 0, 0 }, //  AKEYCODE_BUTTON_C 	98
        { 0, 0 }, //  AKEYCODE_BUTTON_X 	99
        { 0, 0 }, //  AKEYCODE_BUTTON_Y 	100
        { 0, 0 }, //  AKEYCODE_BUTTON_Z 	101
        { 0, 0 }, //  AKEYCODE_BUTTON_L1 	102
        { 0, 0 }, //  AKEYCODE_BUTTON_R1 	103
        { 0, 0 }, //  AKEYCODE_BUTTON_L2 	104
        { 0, 0 }, //  AKEYCODE_BUTTON_R2 	105
        { 0, 0 }, //  AKEYCODE_BUTTON_THUMBL 	106
        { 0, 0 }, //  AKEYCODE_BUTTON_THUMBR 	107
        { 0, 0 }, //  AKEYCODE_BUTTON_START 	108
        { 0, 0 }, //  AKEYCODE_BUTTON_SELECT 	109
        { 0, 0 }, //  AKEYCODE_BUTTON_MODE 	110
        { 0x1b, 0x1b }, //  AKEYCODE_ESCAPE 	111
        { 0, 0 }, //  AKEYCODE_FORWARD_DEL 	112
        { 0, 0 }, //  AKEYCODE_CTRL_LEFT 	113
        { 0, 0 }, //  AKEYCODE_CTRL_RIGHT 	114
        { 0, 0 }, //  AKEYCODE_CAPS_LOCK 	115
        { 0, 0 }, //  AKEYCODE_SCROLL_LOCK 	116
        { 0, 0 }, //  AKEYCODE_META_LEFT 	117
        { 0, 0 }, //  AKEYCODE_META_RIGHT 	118
        { 0, 0 }, //  AKEYCODE_FUNCTION 	119
        { 0, 0 }, //  AKEYCODE_SYSRQ 	120
        { 0, 0 }, //  AKEYCODE_BREAK 	121
        { 0, 0 }, //  AKEYCODE_MOVE_HOME 	122
        { 0, 0 }, //  AKEYCODE_MOVE_END 	123
        { 0, 0 }, //  AKEYCODE_INSERT 	124
        { 0, 0 }, //  AKEYCODE_FORWARD 	125
        { 0, 0 }, //  AKEYCODE_MEDIA_PLAY 	126
        { 0, 0 }, //  AKEYCODE_MEDIA_PAUSE 	127
        { 0, 0 }, //  AKEYCODE_MEDIA_CLOSE 	128
        { 0, 0 }, //  AKEYCODE_MEDIA_EJECT 	129
        { 0, 0 }, //  AKEYCODE_MEDIA_RECORD 	130
        { 0, 0 }, //  AKEYCODE_F1 	131
        { 0, 0 }, //  AKEYCODE_F2 	132
        { 0, 0 }, //  AKEYCODE_F3 	133
        { 0, 0 }, //  AKEYCODE_F4 	134
        { 0, 0 }, //  AKEYCODE_F5 	135
        { 0, 0 }, //  AKEYCODE_F6 	136
        { 0, 0 }, //  AKEYCODE_F7 	137
        { 0, 0 }, //  AKEYCODE_F8 	138
        { 0, 0 }, //  AKEYCODE_F9 	139
        { 0, 0 }, //  AKEYCODE_F10 	140
        { 0, 0 }, //  AKEYCODE_F11 	141
        { 0, 0 }, //  AKEYCODE_F12 	142
        { 0, 0 }, //  AKEYCODE_NUM_LOCK 	143
        { 0x30, 0x30 }, //  AKEYCODE_NUMPAD_0 	144
        { 0x31, 0x31 }, //  AKEYCODE_NUMPAD_1 	145
        { 0x32, 0x32 }, //  AKEYCODE_NUMPAD_2 	146
        { 0x33, 0x33 }, //  AKEYCODE_NUMPAD_3 	147
        { 0x34, 0x34 }, //  AKEYCODE_NUMPAD_4 	148
        { 0x35, 0x35 }, //  AKEYCODE_NUMPAD_5 	149
        { 0x36, 0x36 }, //  AKEYCODE_NUMPAD_6 	150
        { 0x37, 0x37 }, //  AKEYCODE_NUMPAD_7 	151
        { 0x38, 0x38 }, //  AKEYCODE_NUMPAD_8 	152
        { 0x39, 0x39 }, //  AKEYCODE_NUMPAD_9 	153
        { 0x2f, 0x2f }, //  AKEYCODE_NUMPAD_DIVIDE 	154
        { 0x2a, 0x2a }, //  AKEYCODE_NUMPAD_MULTIPLY 	155
        { 0x2d, 0x2d }, //  AKEYCODE_NUMPAD_SUBTRACT 	156
        { 0x2b, 0x2b }, //  AKEYCODE_NUMPAD_ADD 	157
        { 0x2e, 0x2e }, //  AKEYCODE_NUMPAD_DOT 	158
        { 0x2c, 0x2c }, //  AKEYCODE_NUMPAD_COMMA 	159
        { 0x0d, 0x0d }, //  AKEYCODE_NUMPAD_ENTER 	160
        { 0x3d, 0x3d }, //  AKEYCODE_NUMPAD_EQUALS 	161
        { 0, 0 }, //  AKEYCODE_NUMPAD_LEFT_PAREN 	162
        { 0, 0 }, //  AKEYCODE_NUMPAD_RIGHT_PAREN 	163
        { 0, 0 }, //  AKEYCODE_VOLUME_MUTE 	164
        { 0, 0 }, //  AKEYCODE_INFO 	165
        { 0, 0 }, //  AKEYCODE_CHANNEL_UP 	166
        { 0, 0 }, //  AKEYCODE_CHANNEL_DOWN 	167
        { 0, 0 }, //  AKEYCODE_ZOOM_IN 	168
        { 0, 0 }, //  AKEYCODE_ZOOM_OUT 	169
        { 0, 0 }, //  AKEYCODE_TV 	170
        { 0, 0 }, //  AKEYCODE_WINDOW 	171
        { 0, 0 }, //  AKEYCODE_GUIDE 	172
        { 0, 0 }, //  AKEYCODE_DVR 	173
        { 0, 0 }, //  AKEYCODE_BOOKMARK 	174
        { 0, 0 }, //  AKEYCODE_CAPTIONS 	175
        { 0, 0 }, //  AKEYCODE_SETTINGS 	176
        { 0, 0 }, //  AKEYCODE_TV_POWER 	177
        { 0, 0 }, //  AKEYCODE_TV_INPUT 	178
        { 0, 0 }, //  AKEYCODE_STB_POWER 	179
        { 0, 0 }, //  AKEYCODE_STB_INPUT 	180
        { 0, 0 }, //  AKEYCODE_AVR_POWER 	181
        { 0, 0 }, //  AKEYCODE_AVR_INPUT 	182
        { 0, 0 }, //  AKEYCODE_PROG_RED 	183
        { 0, 0 }, //  AKEYCODE_PROG_GREEN 	184
        { 0, 0 }, //  AKEYCODE_PROG_YELLOW 	185
        { 0, 0 }, //  AKEYCODE_PROG_BLUE 	186
        { 0, 0 }, //  AKEYCODE_APP_SWITCH 	187
        { 0, 0 }, //  AKEYCODE_BUTTON_1 	188
        { 0, 0 }, //  AKEYCODE_BUTTON_2 	189
        { 0, 0 }, //  AKEYCODE_BUTTON_3 	190
        { 0, 0 }, //  AKEYCODE_BUTTON_4 	191
        { 0, 0 }, //  AKEYCODE_BUTTON_5 	192
        { 0, 0 }, //  AKEYCODE_BUTTON_6 	193
        { 0, 0 }, //  AKEYCODE_BUTTON_7 	194
        { 0, 0 }, //  AKEYCODE_BUTTON_8 	195
        { 0, 0 }, //  AKEYCODE_BUTTON_9 	196
        { 0, 0 }, //  AKEYCODE_BUTTON_10 	197
        { 0, 0 }, //  AKEYCODE_BUTTON_11 	198
        { 0, 0 }, //  AKEYCODE_BUTTON_12 	199
        { 0, 0 }, //  AKEYCODE_BUTTON_13 	200
        { 0, 0 }, //  AKEYCODE_BUTTON_14 	201
        { 0, 0 }, //  AKEYCODE_BUTTON_15 	202
        { 0, 0 }, //  AKEYCODE_BUTTON_16 	203
        { 0, 0 }, //  AKEYCODE_LANGUAGE_SWITCH 	204
        { 0, 0 }, //  AKEYCODE_MANNER_MODE 	205
        { 0, 0 }, //  AKEYCODE_3D_MODE 	206
        { 0, 0 }, //  AKEYCODE_CONTACTS 	207
        { 0, 0 }, //  AKEYCODE_CALENDAR 	208
        { 0, 0 }, //  AKEYCODE_MUSIC 	209
        { 0, 0 }, //  AKEYCODE_CALCULATOR 	210
        { 0, 0 }, //  AKEYCODE_ZENKAKU_HANKAKU 	211
        { 0, 0 }, //  AKEYCODE_EISU 	212
        { 0, 0 }, //  AKEYCODE_MUHENKAN 	213
        { 0, 0 }, //  AKEYCODE_HENKAN 	214
        { 0, 0 }, //  AKEYCODE_KATAKANA_HIRAGANA 	215
        { 0x7c, 0x7c }, //  AKEYCODE_YEN 	216    -> \ + |
        { 0x5f, 0x7c }, //  AKEYCODE_RO 	217       -> \ + _
        { 0, 0 }, //  AKEYCODE_KANA 	218
        { 0, 0 }, //  AKEYCODE_ASSIST 	219
        { 0, 0 }, //  AKEYCODE_BRIGHTNESS_DOWN 	220
        { 0, 0 }, //  AKEYCODE_BRIGHTNESS_UP 	221
        { 0, 0 }, //  AKEYCODE_MEDIA_AUDIO_TRACK 	222
        { 0, 0 }, //  AKEYCODE_SLEEP 	223
        { 0, 0 }, //  AKEYCODE_WAKEUP 	224
        { 0, 0 }, //  AKEYCODE_PAIRING 	225
        { 0, 0 }, //  AKEYCODE_MEDIA_TOP_MENU 	226
        { 0, 0 }, //  AKEYCODE_11 	227
        { 0, 0 }, //  AKEYCODE_12 	228
        { 0, 0 }, //  AKEYCODE_LAST_CHANNEL 	229
        { 0, 0 }, //  AKEYCODE_TV_DATA_SERVICE 	230
        { 0, 0 }, //  AKEYCODE_VOICE_ASSIST 	231
        { 0, 0 }, //  AKEYCODE_TV_RADIO_SERVICE 	232
        { 0, 0 }, //  AKEYCODE_TV_TELETEXT 	233
        { 0, 0 }, //  AKEYCODE_TV_NUMBER_ENTRY 	234
        { 0, 0 }, //  AKEYCODE_TV_TERRESTRIAL_ANALOG 	235
        { 0, 0 }, //  AKEYCODE_TV_TERRESTRIAL_DIGITAL 	236
        { 0, 0 }, //  AKEYCODE_TV_SATELLITE 	237
        { 0, 0 }, //  AKEYCODE_TV_SATELLITE_BS 	238
        { 0, 0 }, //  AKEYCODE_TV_SATELLITE_CS 	239
        { 0, 0 }, //  AKEYCODE_TV_SATELLITE_SERVICE 	240
        { 0, 0 }, //  AKEYCODE_TV_NETWORK 	241
        { 0, 0 }, //  AKEYCODE_TV_ANTENNA_CABLE 	242
        { 0, 0 }, //  AKEYCODE_TV_INPUT_HDMI_1 	243
        { 0, 0 }, //  AKEYCODE_TV_INPUT_HDMI_2 	244
        { 0, 0 }, //  AKEYCODE_TV_INPUT_HDMI_3 	245
        { 0, 0 }, //  AKEYCODE_TV_INPUT_HDMI_4 	246
        { 0, 0 }, //  AKEYCODE_TV_INPUT_COMPOSITE_1 	247
        { 0, 0 }, //  AKEYCODE_TV_INPUT_COMPOSITE_2 	248
        { 0, 0 }, //  AKEYCODE_TV_INPUT_COMPONENT_1 	249
        { 0, 0 }, //  AKEYCODE_TV_INPUT_COMPONENT_2 	250
        { 0, 0 }, //  AKEYCODE_TV_INPUT_VGA_1 	251
        { 0, 0 }, //  AKEYCODE_TV_AUDIO_DESCRIPTION 	252
        { 0, 0 }, //  AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP 	253
        { 0, 0 }, //  AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN 	254
        { 0, 0 }, //  AKEYCODE_TV_ZOOM_MODE 	255
        { 0, 0 }, //  AKEYCODE_TV_CONTENTS_MENU 	256
        { 0, 0 }, //  AKEYCODE_TV_MEDIA_CONTEXT_MENU 	257
        { 0, 0 }, //  AKEYCODE_TV_TIMER_PROGRAMMING 	258
        { 0, 0 }, //  AKEYCODE_HELP 	259
        { 0, 0 }, //  AKEYCODE_NAVIGATE_PREVIOUS 	260
        { 0, 0 }, //  AKEYCODE_NAVIGATE_NEXT 	261
        { 0, 0 }, //  AKEYCODE_NAVIGATE_IN 	262
        { 0, 0 }, //  AKEYCODE_NAVIGATE_OUT 	263
        { 0, 0 }, //  AKEYCODE_STEM_PRIMARY 	264
        { 0, 0 }, //  AKEYCODE_STEM_1 	265
        { 0, 0 }, //  AKEYCODE_STEM_2 	266
        { 0, 0 }, //  AKEYCODE_STEM_3 	267
        { 0, 0 }, //  AKEYCODE_DPAD_UP_LEFT 	268
        { 0, 0 }, //  AKEYCODE_DPAD_DOWN_LEFT 	269
        { 0, 0 }, //  AKEYCODE_DPAD_UP_RIGHT 	270
        { 0, 0 }, //  AKEYCODE_DPAD_DOWN_RIGHT 	271
        { 0, 0 }, //  AKEYCODE_MEDIA_SKIP_FORWARD 	272
        { 0, 0 }, //  AKEYCODE_MEDIA_SKIP_BACKWARD 	273
        { 0, 0 }, //  AKEYCODE_MEDIA_STEP_FORWARD 	274
        { 0, 0 }, //  AKEYCODE_MEDIA_STEP_BACKWARD 	275
        { 0, 0 }, //  AKEYCODE_SOFT_SLEEP 	276
        { 0, 0 }, //  AKEYCODE_CUT 	277
        { 0, 0 }, //  AKEYCODE_COPY 	278
        { 0, 0 }, //  AKEYCODE_PASTE 	279
        { 0, 0 }, //  AKEYCODE_SYSTEM_NAVIGATION_UP 	280
        { 0, 0 }, //  AKEYCODE_SYSTEM_NAVIGATION_DOWN 	281
        { 0, 0 }, //  AKEYCODE_SYSTEM_NAVIGATION_LEFT 	282
        { 0, 0 }, //  AKEYCODE_SYSTEM_NAVIGATION_RIGHT 	283
        { 0, 0 }, //  AKEYCODE_ALL_APPS 	284
        { 0, 0 }, //  AKEYCODE_REFRESH 	285
        { 0, 0 }, //  AKEYCODE_THUMBS_UP 	286
        { 0, 0 }, //  AKEYCODE_THUMBS_DOWN 	287
        { 0, 0 }, //  AKEYCODE_PROFILE_SWITCH 	288
};


struct BitmapData{
	int width;
	int height;
	uint16_t *bmpImage;
};

// IconType enum is defined in IconRenderer.h using NS_ENUM
// to avoid conflicts with that definition
#if !defined(__OBJC__) && !defined(ICON_TYPE_DEFINED)
#define ICON_TYPE_DEFINED
enum IconType {
    NONE_ICON = -1,
    SYSTEM_ICON = 0,
    FILE_ICON,
};
#endif // !__OBJC__ && !ICON_TYPE_DEFINED

enum systemIconType {
    SYSTEM_NONE = -1,
    SYSTEM_EXIT = 0 ,
    SYSTEM_RESET ,  // SYSTEM_SCREEN ,
    SYSTEM_SOUND,
    SYSTEM_PCG ,
    SYSTEM_CONFIG ,
    SYSTEM_KEYBOARD ,
    SYSTEM_MOUSE ,
    SYSTEM_WALLPAPER ,
    SYSTEM_JOYSTICK ,
    SYSTEM_SCREENSHOT,
    SYSTEM_MIDI,
    SYSTEM_COLORBLIND,
    SYSTEM_ICON_MAX
};
enum FileSelectType {
    FILE_SELECT_NONE = -1,
    FLOPPY_DISK = 0,
    CASETTE_TAPE,
    CARTRIDGE,
    QUICK_DISK,
    HARD_DISK,
    COMPACT_DISC,
    BUBBLE_CASETTE,
    BINARY,
    FILE_SELECT_TYPE_MAX
};
enum SelectDialogMode {
    MEDIA_SELECT = 0,
    DISK_BANK_SELECT = 1,
    BOOT_MODE_SELECT = 2,
    EXIT_EMULATOR = 3
};

enum ScreenSize {
    SCREEN_SIZE_JUST = 0,
    SCREEN_SIZE_MAX = 1,
    SCREEN_SIZE_1 = 2,
    SCREEN_SIZE_2 = 3,
    SCREEN_SIZE_SPECIAL = 4
};

#define MAX_FILE_SELECT_ICON 20
struct FileSelectIconData{
	FileSelectType fileSelectType;
	int driveNo;
};

struct DeviceInfo{
    int width;
    int height;
};

#define     MAX_FRAME_STATS     200
#define     MAX_PERIOD_MS       1500
#define     LOWORD(l)           l

/* simple stats management */
typedef struct {
    double renderTime;
    double frameTime;
} FrameStats;

typedef struct {
    double firstTime;
    double lastTime;
    double frameTime;

    int firstFrame;
    int numFrames;
    FrameStats frames[MAX_FRAME_STATS];
} Stats;

#if defined(_USE_OPENGL_ES20) || defined(_USE_OPENGL_ES30)
typedef struct {
    int viewPortX;
    int viewPortY;
    int viewPortWidth;
    int viewPortHeight;
    int topOffsetSystem;
    int bottomOffsetSystem;
    int topOffsetProgress;
    int topOffsetIcon;
    int bottomOffsetIcon;
    int leftOffsetIcon;
    int rightOffsetIcon;
    int scrWidth;
    int scrHeight;
    int emuWidthBase;
    int emuHeightBase;
    int emuWidth;
    int emuHeight;
    float emuAspect;
    float screenAspect;
    float widthRate;
    float heightRate;
    float screenRate;
    int realScreenWidth;
    int realScreenHeight;
    int leftOffset;
    float topEmuProgressOffset;
    float topEmuScreenOffset;
    float bottomEmuScreenOffset;
    float leftEmuScreenOffset;
    float rightEmuScreenOffset;
} ScreenInfo;
#endif

struct engine {
    struct android_app *app;
    Stats stats;
    int animating;
    bool emu_initialized;
#if defined(_USE_OPENGL_ES20) || defined(_USE_OPENGL_ES30)
    EGLConfig eglConfig;
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;
    std::vector<GLuint> shaderProgram;
    std::vector<GLuint> textureId;
    ScreenInfo screenInfo;
#endif
};

#define EMULATOR_SCREEN_TYPE_DEFAULT     0
#define EMULATOR_SCREEN_TYPE_RGB565      1
#define EMULATOR_SCREEN_TYPE_RGBA8888    2

#if defined(USE_SCREEN_FILTER)
#define     SET_SCREEN_FILTER(FILTER_TYPE)               \
int shader_type = config.shader_type
#else
#define     SET_SCREEN_FILTER(FILTER_TYPE)               \
int shader_type = FILTER_TYPE
#endif

// Core Audio sound system class (similar to Android OBOESOUND)
#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

// Sound buffer length (same as Android implementation)
#define SOUND_BUFFER_LENGTH 4800 * 100 * 2

class COREAUDIO_SOUND {
public:
	COREAUDIO_SOUND() {
		audioUnit = NULL;
		inputSoundBufferPos = 0;
		outputSoundBufferPos = 0;
		inputLoopCount = 0;
		outputLoopCount = 0;
	}
	
	~COREAUDIO_SOUND();
	
	// Ring buffer (same structure as Android OBOESOUND)
	uint16_t soundBuffer[SOUND_BUFFER_LENGTH];
	
	int inputSoundBufferPos = 0;
	int outputSoundBufferPos = 0;
	int inputLoopCount = 0;
	int outputLoopCount = 0;
	
	// Core Audio AudioUnit
	AudioUnit audioUnit;
	
	// Core Audio methods
	OSStatus createAudioUnit(int sampleRate);
	OSStatus startAudioUnit();
	OSStatus stopAudioUnit();
};

// Core Audio callback function declaration
OSStatus CoreAudioRenderCallback(void *inRefCon,
                                AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inBusNumber,
                                UInt32 inNumberFrames,
                                AudioBufferList *ioData);

#endif // __APPLE__

#endif // _OSD_H_
