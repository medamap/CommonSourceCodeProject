#import <Foundation/Foundation.h>
#ifdef BOOL
#undef BOOL
#endif
#define BOOL int

#include "Bridge.h"
#include "xcode_mainloop.h"
#include "xcode_menu_wrapper.h"
#include "file_dialog.h"

// C言語インターフェース関数の宣言
extern "C" {
    void osd_key_down_native(int code, bool repeat);
    void osd_key_up_native(int code);
#ifdef USE_MOUSE
    // マウス関連のosd_*関数は未実装
#endif
#ifdef USE_TOUCH
    void osd_touch_down(int id, int x, int y);
    void osd_touch_up(int id);
    void osd_touch_move(int id, int x, int y);
#endif
    // 音声システム関数
    void osd_start_sound();
    void osd_stop_sound();
    void osd_mute_sound();
    void osd_unmute_sound();
    void osd_set_sound_volume(int volume);
    
    // Metal画面システム関数
    void osd_set_metal_view(void* view);
    int osd_draw_screen();
}

void bridge_run_emulator_mainloop() {
    run_emulator_mainloop(); // iOS用、引数なし
}

// メニューシステム統合関数
void bridge_init_menu_system() {
    init_menu_system();
}

void bridge_cleanup_menu_system() {
    cleanup_menu_system();
}

void* bridge_get_main_menu() {
    return get_main_menu();
}

void bridge_handle_menu_event(int menuId) {
    extern EMU* emu;
    handle_menu_event(menuId, emu);
}

void bridge_update_menu_system() {
    extern EMU* emu;
    update_menu_system(emu);
}

// 入力システム統合関数
void bridge_key_down(int nsKeyCode, bool repeat) {
    osd_key_down_native(nsKeyCode, repeat);
}

void bridge_key_up(int nsKeyCode) {
    osd_key_up_native(nsKeyCode);
}

void bridge_mouse_down(int button) {
#ifdef USE_MOUSE
    // mouse_downは未実装のため、ボタン押下の処理は別途実装
    printf("マウスボタン押下: Button=%d\n", button);
#endif
}

void bridge_mouse_up(int button) {
#ifdef USE_MOUSE
    // mouse_upは未実装のため、ボタン離上の処理は別途実装
    printf("マウスボタン離上: Button=%d\n", button);
#endif
}

void bridge_mouse_move(int x, int y) {
#ifdef USE_MOUSE
    // mouse_moveは未実装のため、マウス座標の管理は別途実装
    printf("マウス移動: X=%d, Y=%d\n", x, y);
#endif
}

void bridge_touch_down(int id, int x, int y) {
#ifdef USE_TOUCH
    osd_touch_down(id, x, y);
#endif
}

void bridge_touch_up(int id) {
#ifdef USE_TOUCH
    osd_touch_up(id);
#endif
}

void bridge_touch_move(int id, int x, int y) {
#ifdef USE_TOUCH
    osd_touch_move(id, x, y);
#endif
}

// 画面表示システム統合関数
void bridge_set_metal_view(void* view) {
    osd_set_metal_view(view);
}

int bridge_draw_screen() {
    return osd_draw_screen();
}

// 設定ダイアログシステム統合関数
void bridge_init_config_system() {
    init_config_dialog_system();
}

void bridge_cleanup_config_system() {
    cleanup_config_dialog_system();
}

void bridge_show_config_dialog(int type) {
    show_config_dialog((config_dialog_type_t)type);
}

void bridge_show_volume_dialog() {
    show_volume_dialog();
}

void bridge_show_about_dialog() {
    show_about_dialog();
}

// アラートダイアログシステム統合関数
void bridge_init_alert_system() {
    init_alert_system();
}

void bridge_cleanup_alert_system() {
    cleanup_alert_system();
}

void bridge_show_info(const char* title, const char* message) {
    show_info(title, message);
}

void bridge_show_warning(const char* title, const char* message) {
    show_warning(title, message);
}

void bridge_show_error(const char* title, const char* message) {
    show_error(title, message);
}

int bridge_ask_yes_no(const char* title, const char* message) {
    return (int)ask_yes_no(title, message);
}

int bridge_ask_ok_cancel(const char* title, const char* message) {
    return (int)ask_ok_cancel(title, message);
}

// ファイル選択システム統合関数
void bridge_init_file_dialog_system() {
    init_file_dialog_system();
}

void bridge_cleanup_file_dialog_system() {
    cleanup_file_dialog_system();
}

void bridge_show_file_dialog(int file_type, int mode, int drive_index) {
    file_dialog_type_t dialog_type = (file_dialog_type_t)file_type;
    file_dialog_mode_t dialog_mode = (file_dialog_mode_t)mode;
    
    // タイプに応じたコールバック関数を選択
    file_dialog_callback_t callback = NULL;
    switch (dialog_type) {
        case FILE_TYPE_FLOPPY_DISK:
            callback = on_floppy_disk_selected;
            break;
        case FILE_TYPE_TAPE:
            callback = on_tape_selected;
            break;
        case FILE_TYPE_HARD_DISK:
            callback = on_hard_disk_selected;
            break;
        case FILE_TYPE_CARTRIDGE:
            callback = on_cart_selected;
            break;
        case FILE_TYPE_STATE:
            callback = on_state_file_selected;
            break;
        default:
            printf("未対応のファイルタイプ: %d\n", file_type);
            return;
    }
    
    show_file_dialog(dialog_type, dialog_mode, drive_index,
                    callback, on_file_selection_error, NULL);
}
