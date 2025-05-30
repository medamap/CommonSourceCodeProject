//
// Xcode Menu Wrapper
// 
// src/menu配下の共通メニューファイルを使用するためのラッパー
// 機種別のメニューファイルはXcodeプロジェクトまたはCMakeで直接指定
//

#ifndef XCODE_MENU_WRAPPER_H
#define XCODE_MENU_WRAPPER_H

// 共通メニューヘッダーのインクルード
// src/menu から直接インクルード（Android/Xcode共通）
#include "../menu/BaseMenu.h"
#include "../menu/menu.h"
#include "../config.h"
#include "file_dialog.h"
#include "config_dialog.h"
#include "alert_dialog.h"

// 前方宣言
class EMU;

// メニュー更新関数の宣言
// これらの関数はandroid_main.cppから抽出したものと同じインターフェース
void update_control_menu(Menu* hMenu);

#ifdef USE_FLOPPY_DISK
void update_floppy_disk_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_FD, unsigned int ID_D88_FILE_PATH, 
                            unsigned int ID_SELECT_D88_BANK, unsigned int ID_EJECT_D88_BANK, unsigned int ID_CLOSE_FD, 
                            unsigned int ID_WRITE_PROTECT_FD, unsigned int ID_CORRECT_TIMING_FD, unsigned int ID_IGNORE_CRC_FD);
#endif

#ifdef USE_CART
void update_cart_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_CART, unsigned int ID_CLOSE_CART);
#endif

#ifdef USE_TAPE
void update_tape_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_TAPE, unsigned int ID_CLOSE_TAPE, 
                     unsigned int ID_PLAY_BUTTON, unsigned int ID_STOP_BUTTON, unsigned int ID_FAST_FORWARD, 
                     unsigned int ID_FAST_REWIND, unsigned int ID_APSS_FORWARD, unsigned int ID_APSS_REWIND, 
                     unsigned int ID_USE_WAVE_SHAPER, unsigned int ID_DIRECT_LOAD_MZT, 
                     unsigned int ID_TAPE_BAUD_LOW, unsigned int ID_TAPE_BAUD_HIGH);
#endif

#ifdef USE_HARD_DISK
void update_hard_disk_menu(Menu* hMenu, int drv, unsigned int ID_RECENT_HD, unsigned int ID_CLOSE_HD);
#endif

#ifdef USE_STATE
void update_save_state_menu(Menu* hMenu);
void update_load_state_menu(Menu* hMenu);
#endif

// メニューイベント処理
void process_menu_event(int menuId, EMU* emu);

// 設定ダイアログメニューイベント処理
void handle_config_menu_event(int menuId);

// ファイル選択のコールバック関数
void on_floppy_disk_selected(const char* file_path, int drive_index, void* user_data);
void on_tape_selected(const char* file_path, int drive_index, void* user_data);
void on_hard_disk_selected(const char* file_path, int drive_index, void* user_data);
void on_cart_selected(const char* file_path, int drive_index, void* user_data);
void on_state_file_selected(const char* file_path, int drive_index, void* user_data);
void on_file_selection_error(const char* error_message, void* user_data);

// メニュー管理
Menu* create_main_menu();
void update_popup_menu(Menu** hMenu);

// C言語インターフェース（Bridge.mm等から使用）
#ifdef __cplusplus
extern "C" {
#endif

void init_menu_system(void);
void update_menu_system(EMU* emu);
Menu* get_main_menu(void);
void handle_menu_event(int menuId, EMU* emu);
void cleanup_menu_system(void);

#ifdef __cplusplus
}
#endif

#endif // XCODE_MENU_WRAPPER_H