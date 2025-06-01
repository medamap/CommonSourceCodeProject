//
// Xcode Menu Wrapper Implementation
//
// メニュー更新関数の実装（android_main.cppから抽出）
// 機種別のメニューファイルはビルドシステムで指定
//

#include "xcode_menu_wrapper.h"
#include "../emu.h"
#include "../config.h"
#include "../res/resource.h"
#include "file_dialog.h"
#ifdef __APPLE__
#include "ConfigManager.h"
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <ctime>

// グローバル変数
static Menu* g_mainMenu = nullptr;
extern EMU* emu;
extern OSD* osd;

////////////////////////////////////////////////////////////////////////////////
// メニュー更新関数（android_main.cppから移植）
////////////////////////////////////////////////////////////////////////////////

void update_control_menu(Menu* hMenu)
{
    if(config.cpu_power >= 0 && config.cpu_power < 5) {
        hMenu->CheckMenuRadioItem(ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER0 + config.cpu_power);
    }
    hMenu->CheckMenuItem(ID_FULL_SPEED, config.full_speed);
    hMenu->CheckMenuItem(ID_DRIVE_VM_IN_OPECODE, config.drive_vm_in_opecode);
    
#ifdef USE_AUTO_KEY
    bool now_paste = true, now_stop = true;
    if(emu) {
        now_paste = emu->is_auto_key_running();
        now_stop = !now_paste;
    }
    hMenu->CheckMenuItem(ID_ROMAJI_TO_KANA, config.romaji_to_kana);
#endif

#ifdef USE_DEBUGGER
    for(int i = 0; i < 8; i++) {
        hMenu->EnableMenuItem(ID_OPEN_DEBUGGER0 + i, emu && !emu->now_debugging && emu->is_debugger_enabled(i));
    }
    hMenu->EnableMenuItem(ID_CLOSE_DEBUGGER, emu && emu->now_debugging);
#endif
}

#ifdef USE_STATE
void update_save_state_menu(Menu* hMenu)
{
    int fd;
    struct stat fileInfo;
    struct tm timeInfo;
    char buf[64];
    char dateBuf[64];

    for(int i = 0; i < 10; i++) {
        const char* filePath = emu->state_file_path(i);
        fd = open(filePath, O_RDONLY);
        if(fd != -1) {
            fstat(fd, &fileInfo);
            localtime_r(&fileInfo.st_mtime, &timeInfo);
            strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d  %H:%M:%S", &timeInfo);
            snprintf(buf, sizeof(buf), "%d:  %s", i, dateBuf);
            close(fd);
        } else {
            snprintf(buf, sizeof(buf), "%d: (No Data)", i);
        }
        hMenu->SetMenuItemInfo(ID_SAVE_STATE0 + i, buf);
    }
}

void update_load_state_menu(Menu* hMenu)
{
    int fd;
    struct stat fileInfo;
    struct tm timeInfo;
    char buf[64];
    char dateBuf[64];

    for(int i = 0; i < 10; i++) {
        const char* filePath = emu->state_file_path(i);
        fd = open(filePath, O_RDONLY);
        if(fd != -1) {
            fstat(fd, &fileInfo);
            localtime_r(&fileInfo.st_mtime, &timeInfo);
            strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d  %H:%M:%S", &timeInfo);
            snprintf(buf, sizeof(buf), "%d:  %s", i, dateBuf);
            close(fd);
        } else {
            snprintf(buf, sizeof(buf), "%d: (No Data)", i);
        }
        hMenu->SetMenuItemInfo(ID_LOAD_STATE0 + i, buf);
    }
}
#endif

#ifdef USE_FLOPPY_DISK
void update_floppy_disk_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_FD, unsigned int ID_D88_FILE_PATH, 
                            unsigned int ID_SELECT_D88_BANK, unsigned int ID_EJECT_D88_BANK, unsigned int ID_CLOSE_FD, 
                            unsigned int ID_WRITE_PROTECT_FD, unsigned int ID_CORRECT_TIMING_FD, unsigned int ID_IGNORE_CRC_FD)
{
    if (emu) {
        hMenu->EnableMenuItem(ID_CLOSE_FD, emu->is_floppy_disk_inserted(drv));
        hMenu->EnableMenuItem(ID_WRITE_PROTECT_FD, emu->is_floppy_disk_inserted(drv));
        hMenu->CheckMenuItem(ID_WRITE_PROTECT_FD, emu->is_floppy_disk_protected(drv));
        hMenu->CheckMenuItem(ID_CORRECT_TIMING_FD, config.correct_disk_timing[drv]);
        hMenu->CheckMenuItem(ID_IGNORE_CRC_FD, config.ignore_disk_crc[drv]);
    }
}
#endif

#ifdef USE_CART
void update_cart_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_CART, unsigned int ID_CLOSE_CART)
{
    if (emu) {
        hMenu->EnableMenuItem(ID_CLOSE_CART, emu->is_cart_inserted(drv));
    }
}
#endif

#ifdef USE_TAPE
void update_tape_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_TAPE, unsigned int ID_CLOSE_TAPE, 
                     unsigned int ID_PLAY_BUTTON, unsigned int ID_STOP_BUTTON, unsigned int ID_FAST_FORWARD, 
                     unsigned int ID_FAST_REWIND, unsigned int ID_APSS_FORWARD, unsigned int ID_APSS_REWIND, 
                     unsigned int ID_USE_WAVE_SHAPER, unsigned int ID_DIRECT_LOAD_MZT, 
                     unsigned int ID_TAPE_BAUD_LOW, unsigned int ID_TAPE_BAUD_HIGH)
{
    if (emu) {
        hMenu->EnableMenuItem(ID_CLOSE_TAPE, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_PLAY_BUTTON, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_STOP_BUTTON, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_FAST_FORWARD, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_FAST_REWIND, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_APSS_FORWARD, emu->is_tape_inserted(drv));
        hMenu->EnableMenuItem(ID_APSS_REWIND, emu->is_tape_inserted(drv));
        hMenu->CheckMenuItem(ID_USE_WAVE_SHAPER, config.wave_shaper[drv]);
        hMenu->CheckMenuItem(ID_DIRECT_LOAD_MZT, config.direct_load_mzt[drv]);
        hMenu->CheckMenuRadioItem(ID_TAPE_BAUD_LOW, ID_TAPE_BAUD_HIGH, !config.baud_high[drv] ? ID_TAPE_BAUD_LOW : ID_TAPE_BAUD_HIGH);
    }
}
#endif

#ifdef USE_HARD_DISK
void update_hard_disk_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_HD, unsigned int ID_CLOSE_HD)
{
    if (emu) {
        hMenu->EnableMenuItem(ID_CLOSE_HD, emu->is_hard_disk_inserted(drv));
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
// メニュー管理
////////////////////////////////////////////////////////////////////////////////

Menu* create_main_menu()
{
    // Menu クラスのインスタンスは機種別の cpp ファイルで定義される
    // 例: src/menu/x1turbo.cpp の Menu::Menu() コンストラクタ
    return new Menu();
}

void update_popup_menu(Menu** hMenu)
{
    if (*hMenu == nullptr) {
        *hMenu = create_main_menu();
    }
    
    // 各種メニューの状態を更新
    update_control_menu(*hMenu);
    
#ifdef USE_STATE
    update_save_state_menu(*hMenu);
    update_load_state_menu(*hMenu);
#endif
    
#ifdef USE_FLOPPY_DISK
    for (int i = 0; i < USE_FLOPPY_DISK; i++) {
        update_floppy_disk_menu(*hMenu, i, 
            ID_RECENT_FD1 + i, ID_D88_FILE_PATH1 + i, ID_SELECT_D88_BANK1 + i, 
            ID_EJECT_D88_BANK1 + i, ID_CLOSE_FD1 + i, ID_WRITE_PROTECT_FD1 + i, 
            ID_CORRECT_TIMING_FD1 + i, ID_IGNORE_CRC_FD1 + i);
    }
#endif

#ifdef USE_TAPE
    for (int i = 0; i < USE_TAPE; i++) {
        update_tape_menu(*hMenu, i, 
            ID_RECENT_TAPE1 + i, ID_CLOSE_TAPE1 + i, ID_PLAY_BUTTON1 + i, ID_STOP_BUTTON1 + i,
            ID_FAST_FORWARD1 + i, ID_FAST_REWIND1 + i, ID_APSS_FORWARD1 + i, ID_APSS_REWIND1 + i,
            ID_USE_WAVE_SHAPER1 + i, ID_DIRECT_LOAD_MZT1 + i, ID_TAPE_BAUD_LOW1 + i, ID_TAPE_BAUD_HIGH1 + i);
    }
#endif

#ifdef USE_HARD_DISK
    for (int i = 0; i < USE_HARD_DISK; i++) {
        update_hard_disk_menu(*hMenu, i, ID_RECENT_HD1 + i, ID_CLOSE_HD1 + i);
    }
#endif

#ifdef USE_CART
    for (int i = 0; i < USE_CART; i++) {
        update_cart_menu(*hMenu, i, ID_RECENT_CART1 + i, ID_CLOSE_CART1 + i);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
// ファイル選択コールバック関数
////////////////////////////////////////////////////////////////////////////////

void on_floppy_disk_selected(const char* file_path, int drive_index, void* user_data)
{
    if (!emu) return;
    
    if (file_path == NULL) {
        printf("フロッピーディスク選択がキャンセルされました\n");
        return;
    }
    
    printf("フロッピーディスクを挿入: ドライブ%d, ファイル: %s\n", drive_index, file_path);
    emu->open_floppy_disk(drive_index, file_path, 0); // バンク0として挿入
}

void on_tape_selected(const char* file_path, int drive_index, void* user_data)
{
    if (!emu) return;
    
    if (file_path == NULL) {
        printf("テープファイル選択がキャンセルされました\n");
        return;
    }
    
    // user_dataからプレイ/レコードモードを判定
    bool is_play_mode = (user_data != NULL && *((bool*)user_data));
    
    printf("テープを挿入: ドライブ%d, ファイル: %s, モード: %s\n", 
           drive_index, file_path, is_play_mode ? "再生" : "録音");
    
    if (is_play_mode) {
        emu->play_tape(drive_index, file_path);
    } else {
        emu->rec_tape(drive_index, file_path);
    }
}

#ifdef USE_HARD_DISK
void on_hard_disk_selected(const char* file_path, int drive_index, void* user_data)
{
    if (!emu) return;
    
    if (file_path == NULL) {
        printf("ハードディスク選択がキャンセルされました\n");
        return;
    }
    
    printf("ハードディスクを挿入: ドライブ%d, ファイル: %s\n", drive_index, file_path);
    emu->open_hard_disk(drive_index, file_path);
}
#endif

#ifdef USE_CART
void on_cart_selected(const char* file_path, int drive_index, void* user_data)
{
    if (!emu) return;
    
    if (file_path == NULL) {
        printf("カートリッジ選択がキャンセルされました\n");
        return;
    }
    
    printf("カートリッジを挿入: ドライブ%d, ファイル: %s\n", drive_index, file_path);
    emu->open_cart(drive_index, file_path);
}
#endif

void on_state_file_selected(const char* file_path, int drive_index, void* user_data)
{
    if (!emu) return;
    
    if (file_path == NULL) {
        printf("ステートファイル選択がキャンセルされました\n");
        return;
    }
    
    // user_dataからロード/セーブモードを判定
    bool is_load_mode = (user_data != NULL && *((bool*)user_data));
    
    printf("ステートファイルを処理: スロット%d, ファイル: %s, モード: %s\n", 
           drive_index, file_path, is_load_mode ? "ロード" : "セーブ");
    
    if (is_load_mode) {
        emu->load_state(file_path);
    } else {
        emu->save_state(file_path);
    }
}

void on_file_selection_error(const char* error_message, void* user_data)
{
    printf("ファイル選択エラー: %s\n", error_message);
}

////////////////////////////////////////////////////////////////////////////////
// メニューイベント処理
////////////////////////////////////////////////////////////////////////////////

void process_menu_event(int menuId, EMU* emu)
{
    if (!emu) return;
    
    printf("メニューイベント処理: ID = %d\n", menuId);
    
    switch (menuId) {
        case ID_RESET:
            emu->reset();
            break;
            
        case ID_SPECIAL_RESET:
#ifdef USE_SPECIAL_RESET
            emu->special_reset();
#endif
            break;
            
        case ID_CPU_POWER0:
        case ID_CPU_POWER1:
        case ID_CPU_POWER2:
        case ID_CPU_POWER3:
        case ID_CPU_POWER4:
            config.cpu_power = menuId - ID_CPU_POWER0;
            emu->update_config();
            break;
            
        case ID_FULL_SPEED:
            config.full_speed = !config.full_speed;
            break;
            
        case ID_DRIVE_VM_IN_OPECODE:
            config.drive_vm_in_opecode = !config.drive_vm_in_opecode;
            break;
            
        case ID_SOUND_ON:
            // Android版と同じ実装：reset_sound() -> トグル -> config同期
            emu->get_osd()->reset_sound();
            emu->get_osd()->soundEnable = !(emu->get_osd()->soundEnable);
#if defined(__ANDROID__) || defined(__APPLE__)
            config.sound_on = emu->get_osd()->soundEnable;
#endif
#ifdef __APPLE__
            // ConfigManagerにも反映
            ConfigManager::getInstance().setSoundEnabled(emu->get_osd()->soundEnable);
#endif
            printf("音声切り替え: %s\n", emu->get_osd()->soundEnable ? "ON" : "OFF");
            break;

#ifdef USE_FLOPPY_DISK
        case ID_OPEN_FD1:
        case ID_OPEN_FD2:
        case ID_OPEN_FD3:
        case ID_OPEN_FD4:
            {
                int drive_index = menuId - ID_OPEN_FD1;
                printf("フロッピーディスク挿入: ドライブ %d\n", drive_index);
                show_file_dialog(FILE_TYPE_FLOPPY_DISK, FILE_DIALOG_MODE_OPEN, drive_index,
                               on_floppy_disk_selected, on_file_selection_error, NULL);
            }
            break;
            
        case ID_CLOSE_FD1:
        case ID_CLOSE_FD2:
        case ID_CLOSE_FD3:
        case ID_CLOSE_FD4:
            emu->close_floppy_disk(menuId - ID_CLOSE_FD1);
            break;
            
        case ID_WRITE_PROTECT_FD1:
        case ID_WRITE_PROTECT_FD2:
        case ID_WRITE_PROTECT_FD3:
        case ID_WRITE_PROTECT_FD4:
            {
                int drv = menuId - ID_WRITE_PROTECT_FD1;
                emu->is_floppy_disk_protected(drv, !emu->is_floppy_disk_protected(drv));
            }
            break;
#endif

#ifdef USE_TAPE
        case ID_PLAY_TAPE1:
            {
                printf("テープ挿入（再生）\n");
                static bool play_mode = true;
                show_file_dialog(FILE_TYPE_TAPE, FILE_DIALOG_MODE_OPEN, 0,
                               on_tape_selected, on_file_selection_error, &play_mode);
            }
            break;
            
        case ID_REC_TAPE1:
            {
                printf("テープ挿入（録音）\n");
                static bool play_mode = false;
                show_file_dialog(FILE_TYPE_TAPE, FILE_DIALOG_MODE_SAVE, 0,
                               on_tape_selected, on_file_selection_error, &play_mode);
            }
            break;
            
        case ID_CLOSE_TAPE1:
            emu->close_tape(0);
            break;
            
        case ID_PLAY_BUTTON1:
            emu->push_play(0);
            break;
            
        case ID_STOP_BUTTON1:
            emu->push_stop(0);
            break;
            
        case ID_FAST_FORWARD1:
            emu->push_fast_forward(0);
            break;
            
        case ID_FAST_REWIND1:
            emu->push_fast_rewind(0);
            break;
#endif

#ifdef USE_HARD_DISK
        case ID_OPEN_HD1:
        case ID_OPEN_HD2:
        case ID_OPEN_HD3:
        case ID_OPEN_HD4:
            {
                int drive_index = menuId - ID_OPEN_HD1;
                printf("ハードディスク挿入: ドライブ %d\n", drive_index);
                show_file_dialog(FILE_TYPE_HARD_DISK, FILE_DIALOG_MODE_OPEN, drive_index,
                               on_hard_disk_selected, on_file_selection_error, NULL);
            }
            break;
            
        case ID_CLOSE_HD1:
        case ID_CLOSE_HD2:
        case ID_CLOSE_HD3:
        case ID_CLOSE_HD4:
            emu->close_hard_disk(menuId - ID_CLOSE_HD1);
            break;
#endif

#ifdef USE_CART
        case ID_OPEN_CART1:
        case ID_OPEN_CART2:
            {
                int drive_index = menuId - ID_OPEN_CART1;
                printf("カートリッジ挿入: ドライブ %d\n", drive_index);
                show_file_dialog(FILE_TYPE_CARTRIDGE, FILE_DIALOG_MODE_OPEN, drive_index,
                               on_cart_selected, on_file_selection_error, NULL);
            }
            break;
            
        case ID_CLOSE_CART1:
        case ID_CLOSE_CART2:
            emu->close_cart(menuId - ID_CLOSE_CART1);
            break;
            
        // Recent cart files for drive 0
        case ID_RECENT_CART1 + 0:
        case ID_RECENT_CART1 + 1:
        case ID_RECENT_CART1 + 2:
        case ID_RECENT_CART1 + 3:
        case ID_RECENT_CART1 + 4:
        case ID_RECENT_CART1 + 5:
        case ID_RECENT_CART1 + 6:
        case ID_RECENT_CART1 + 7:
            {
                int recent_index = menuId - ID_RECENT_CART1;
                printf("最近使用したカートリッジを開く: ドライブ0, インデックス %d\n", recent_index);
                // TODO: Implement openRecentCartDialog or similar functionality
                // For now, just print a message
            }
            break;
            
#if USE_CART >= 2
        // Recent cart files for drive 1
        case ID_RECENT_CART2 + 0:
        case ID_RECENT_CART2 + 1:
        case ID_RECENT_CART2 + 2:
        case ID_RECENT_CART2 + 3:
        case ID_RECENT_CART2 + 4:
        case ID_RECENT_CART2 + 5:
        case ID_RECENT_CART2 + 6:
        case ID_RECENT_CART2 + 7:
            {
                int recent_index = menuId - ID_RECENT_CART2;
                printf("最近使用したカートリッジを開く: ドライブ1, インデックス %d\n", recent_index);
                // TODO: Implement openRecentCartDialog or similar functionality
                // For now, just print a message
            }
            break;
#endif
#endif

#ifdef USE_STATE
        case ID_SAVE_STATE0:
        case ID_SAVE_STATE1:
        case ID_SAVE_STATE2:
        case ID_SAVE_STATE3:
        case ID_SAVE_STATE4:
        case ID_SAVE_STATE5:
        case ID_SAVE_STATE6:
        case ID_SAVE_STATE7:
        case ID_SAVE_STATE8:
        case ID_SAVE_STATE9:
            emu->save_state(emu->state_file_path(menuId - ID_SAVE_STATE0));
            break;
            
        case ID_LOAD_STATE0:
        case ID_LOAD_STATE1:
        case ID_LOAD_STATE2:
        case ID_LOAD_STATE3:
        case ID_LOAD_STATE4:
        case ID_LOAD_STATE5:
        case ID_LOAD_STATE6:
        case ID_LOAD_STATE7:
        case ID_LOAD_STATE8:
        case ID_LOAD_STATE9:
            emu->load_state(emu->state_file_path(menuId - ID_LOAD_STATE0));
            break;
#endif

#ifdef USE_DEBUGGER
        case ID_OPEN_DEBUGGER0:
        case ID_OPEN_DEBUGGER1:
        case ID_OPEN_DEBUGGER2:
        case ID_OPEN_DEBUGGER3:
        case ID_OPEN_DEBUGGER4:
        case ID_OPEN_DEBUGGER5:
        case ID_OPEN_DEBUGGER6:
        case ID_OPEN_DEBUGGER7:
            emu->open_debugger(menuId - ID_OPEN_DEBUGGER0);
            break;
            
        case ID_CLOSE_DEBUGGER:
            emu->close_debugger();
            break;
#endif

        // ID_SOUND_ONは上記で既に実装済み
            
        case ID_SOUND_FREQ0: // 2000Hz
        case ID_SOUND_FREQ1: // 4000Hz  
        case ID_SOUND_FREQ2: // 8000Hz
        case ID_SOUND_FREQ3: // 11025Hz
        case ID_SOUND_FREQ4: // 22050Hz
        case ID_SOUND_FREQ5: // 44100Hz
        case ID_SOUND_FREQ6: // 62500Hz
        case ID_SOUND_FREQ7: // 96000Hz
            {
                // 音声周波数の変更
                int freq_index = menuId - ID_SOUND_FREQ0;
                int frequencies[] = {2000, 4000, 8000, 11025, 22050, 44100, 62500, 96000};
                
                if (freq_index >= 0 && freq_index < 8) {
                    int new_frequency = frequencies[freq_index];
                    config.sound_frequency = freq_index;  // インデックスを保存
                    
                    printf("音声周波数変更: インデックス%d (%dHz)\n", freq_index, new_frequency);
                    
                    // ConfigManagerを使用してリアルタイム適用
                    #ifdef __APPLE__
                    ConfigManager::getInstance().setSoundFrequency(new_frequency);  // 実際の周波数値を保存
                    ConfigManager::getInstance().setInt("sound_frequency", freq_index);  // インデックスも保存
                    #endif
                    
                    // 一時的にOSDの音声システムを直接再初期化
                    if (osd) {
                        osd->reinitialize_sound_if_needed(new_frequency, osd->sound_samples);
                    }
                    
                    // エミュレータ設定を更新
                    if (emu) {
                        emu->update_config();
                    }
                }
            }
            break;
            
        case ID_SOUND_LATE0: // 50msec
        case ID_SOUND_LATE1: // 100msec
        case ID_SOUND_LATE2: // 200msec
        case ID_SOUND_LATE3: // 300msec
        case ID_SOUND_LATE4: // 400msec
            {
                // 音声レイテンシの変更
                int latency_index = menuId - ID_SOUND_LATE0;
                int latencies[] = {50, 100, 200, 300, 400};
                
                if (latency_index >= 0 && latency_index < 5) {
                    int new_latency = latencies[latency_index];
                    config.sound_latency = new_latency;
                    
                    printf("音声レイテンシ変更: %dms\n", new_latency);
                    
                    // ConfigManagerを使用してリアルタイム適用
                    #ifdef __APPLE__
                    ConfigManager::getInstance().setSoundLatency(new_latency);
                    #endif
                    
                    // エミュレータ設定を更新
                    if (emu) {
                        emu->update_config();
                    }
                }
            }
            break;
            
        case ID_SOUND_VOLUME:
            // 音量設定ダイアログを表示
            show_volume_dialog();
            break;

        case ID_EXIT:
            // TODO: アプリケーション終了処理
            printf("終了メニューが選択されました\n");
            break;

        default:
            // 設定関連のメニューIDをチェック
            handle_config_menu_event(menuId);
            printf("未処理のメニューID: %d\n", menuId);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 設定ダイアログメニューイベント処理
////////////////////////////////////////////////////////////////////////////////

void handle_config_menu_event(int menuId)
{
    // 設定関連のメニューIDに対応
    // TODO: これらのIDは実際のresource.hから取得する必要があります
    
    // 一般的な設定メニューIDパターンに基づいて推定
    if (menuId >= 40000 && menuId < 50000) {
        printf("設定メニューイベント: ID=%d\n", menuId);
        
        // 設定の種類を推定（IDの範囲で判断）
        if (menuId >= 40000 && menuId < 41000) {
            // 一般設定系
            show_general_config_dialog();
        } else if (menuId >= 41000 && menuId < 42000) {
            // 音声設定系
            show_sound_config_dialog();
        } else if (menuId >= 42000 && menuId < 43000) {
            // 画面設定系
            show_screen_config_dialog();
        } else if (menuId >= 43000 && menuId < 44000) {
            // 入力設定系
            show_input_config_dialog();
        } else if (menuId >= 44000 && menuId < 45000) {
            // 制御設定系
            show_control_config_dialog();
        } else if (menuId >= 49000 && menuId < 50000) {
            // バージョン情報系
            show_about_dialog();
        } else {
            printf("未知の設定メニューID: %d\n", menuId);
        }
    }
    
    // 特定の設定メニューID（resource.hにある可能性のあるもの）
    switch (menuId) {
        // 音量設定ダイアログ（IDD_VOLUMEに関連）
        case 111: // IDD_VOLUME from resource.h
            show_volume_dialog();
            break;
            
        // ジョイスティック設定（IDD_JOYSTICKに関連）
        case 181: // IDD_JOYSTICK from resource.h
            show_input_config_dialog();
            break;
            
        // 一般的なメニューID（推定）
        case 1001: // CONFIG
        case 1002: // OPTIONS
        case 1003: // SETTINGS
            show_general_config_dialog();
            break;
            
        case 1011: // SOUND CONFIG
        case 1012: // AUDIO CONFIG
            show_sound_config_dialog();
            break;
            
        case 1021: // SCREEN CONFIG
        case 1022: // VIDEO CONFIG
            show_screen_config_dialog();
            break;
            
        case 1031: // INPUT CONFIG
        case 1032: // CONTROL CONFIG
            show_input_config_dialog();
            break;
            
        case 1041: // SYSTEM CONFIG
            show_control_config_dialog();
            break;
            
        case 1091: // ABOUT
        case 1092: // VERSION
            show_about_dialog();
            break;
            
        default:
            // 何もしない（通常のメニューイベント）
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// C言語インターフェース
////////////////////////////////////////////////////////////////////////////////

extern "C" void init_menu_system(void)
{
    printf("メニューシステムを初期化中...\n");
    
    // アラートシステム初期化
    init_alert_system();
    
    // ファイルダイアログシステム初期化
    init_file_dialog_system();
    
    if (g_mainMenu) {
        delete g_mainMenu;
    }
    
    g_mainMenu = create_main_menu();
    if (g_mainMenu) {
        printf("メニューシステム初期化完了\n");
    } else {
        printf("メニューシステム初期化失敗\n");
        show_error("初期化エラー", "メニューシステムの初期化に失敗しました。");
    }
}

extern "C" void update_menu_system(EMU* emu)
{
    if (!g_mainMenu) {
        init_menu_system();
        return;
    }
    
    update_popup_menu(&g_mainMenu);
}

extern "C" Menu* get_main_menu(void)
{
    if (!g_mainMenu) {
        init_menu_system();
    }
    return g_mainMenu;
}

extern "C" void handle_menu_event(int menuId, EMU* emu)
{
    if (!g_mainMenu) {
        printf("メニューシステムが初期化されていません\n");
        return;
    }
    
    process_menu_event(menuId, emu);
    update_menu_system(emu);
}

extern "C" void cleanup_menu_system(void)
{
    if (g_mainMenu) {
        delete g_mainMenu;
        g_mainMenu = nullptr;
        printf("メニューシステムをクリーンアップしました\n");
    }
    
    // ファイルダイアログシステムクリーンアップ
    cleanup_file_dialog_system();
    
    // アラートシステムクリーンアップ
    cleanup_alert_system();
}