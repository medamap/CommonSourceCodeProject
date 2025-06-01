/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]

    [for Xcode]
    Author : MedamaP
    Date   : 2025.05.06
*/

#include "osd.h"
#include <cstring>
#include <unistd.h>

// ConfigManager (Note: Apple固有の実装のため条件付きインクルード)
#ifdef __APPLE__
#include "ConfigManager.h"
#endif

// グローバルOSDインスタンス
OSD* osd = nullptr;

void OSD::initialize(int rate, int samples)
{
    // 初期化
    lock_count = 0;
    
    // ConfigManagerを最初に初期化
    ConfigManager& configManager = ConfigManager::getInstance();
    
    // EMU側からrateとsamplesが渡されるので、ConfigManagerは使用せず
    // 音声有効/無効フラグのみConfigManagerから取得
    bool sound_enabled = configManager.getBool("sound_enabled", false);      // 音声デフォルト無効
    
    // EMU側から渡されたrateとsamplesを使用
    if (rate == 0) {
        rate = 44100;  // 最終的なフォールバック
    }
    
    if (samples == 0) {
        // デフォルトサンプル数を計算
        samples = (rate * 100) / 1000;  // 100msec
        if (samples < 256) samples = 256;
        if (samples > 4096) samples = 4096;
    }
    
    printf("Apple プラットフォーム用OSD初期化中: rate=%dHz, samples=%d\n", rate, samples);
    
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#ifdef USE_SOCKET
    initialize_socket();
#endif
#ifdef USE_MIDI
    initialize_midi();
#endif
// #ifdef USE_PRINTER
//     initialize_printer();
// #endif
    
    // [DEBUG_START - claude code verification]
    printf("[DEBUG] OSD::initialize: サウンドを有効化してstart_sound()を呼び出します\n");
    // [DEBUG_END - claude code verification]
    
    // Enable sound and start audio playback (ConfigManagerの設定に基づく)
    soundEnable = sound_enabled;
    
#if defined(__ANDROID__) || defined(__APPLE__)
    // config.sound_onとの同期
    config.sound_on = soundEnable;
#endif
    
    if (soundEnable) {
        start_sound();
    }
}

void OSD::release()
{
	release_input();
	release_screen();
	release_sound();
#ifdef USE_SOCKET
    release_socket();
#endif
#ifdef USE_MIDI
    release_midi();
#endif
// #ifdef USE_PRINTER
//     release_printer();
// #endif
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

// Medamap and Claude: sleep関数はosd.hでインライン定義済みのためコメントアウト
// void OSD::sleep(uint32_t ms)
// {
// 	// Medamap and Claude: macOS/iOS用のスリープ実装
// 	usleep(ms * 1000);
// }

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

