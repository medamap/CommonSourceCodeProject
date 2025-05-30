#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bridge_run_emulator_mainloop();

// メニューシステム統合関数
void bridge_init_menu_system();
void bridge_cleanup_menu_system();
void* bridge_get_main_menu();
void bridge_handle_menu_event(int menuId);
void bridge_update_menu_system();

// 入力システム統合関数
void bridge_key_down(int nsKeyCode, bool repeat);
void bridge_key_up(int nsKeyCode);
void bridge_mouse_down(int button);
void bridge_mouse_up(int button);
void bridge_mouse_move(int x, int y);
void bridge_touch_down(int id, int x, int y);
void bridge_touch_up(int id);
void bridge_touch_move(int id, int x, int y);

// 画面表示システム統合関数
void bridge_set_metal_view(void* view);
int bridge_draw_screen();

// 設定ダイアログシステム統合関数
void bridge_init_config_system();
void bridge_cleanup_config_system();
void bridge_show_config_dialog(int type);
void bridge_show_volume_dialog();
void bridge_show_about_dialog();

// アラートダイアログシステム統合関数
void bridge_init_alert_system();
void bridge_cleanup_alert_system();
void bridge_show_info(const char* title, const char* message);
void bridge_show_warning(const char* title, const char* message);
void bridge_show_error(const char* title, const char* message);
int bridge_ask_yes_no(const char* title, const char* message);
int bridge_ask_ok_cancel(const char* title, const char* message);

// ファイル選択システム統合関数
void bridge_init_file_dialog_system();
void bridge_cleanup_file_dialog_system();
void bridge_show_file_dialog(int file_type, int mode, int drive_index);

#ifdef __cplusplus
}
#endif
