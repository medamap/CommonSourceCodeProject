//
// config_dialog.h
// エミュレータ設定ダイアログシステム
//
// Author: Medamap and Claude
// Date: 2025.01.29
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 設定ダイアログの種類
typedef enum {
    CONFIG_DIALOG_GENERAL = 0,      // 一般設定
    CONFIG_DIALOG_SOUND,            // 音声設定
    CONFIG_DIALOG_SCREEN,           // 画面設定
    CONFIG_DIALOG_INPUT,            // 入力設定
    CONFIG_DIALOG_CONTROL,          // 制御設定
    CONFIG_DIALOG_ABOUT,            // バージョン情報
    CONFIG_DIALOG_TYPE_MAX
} config_dialog_type_t;

// 設定変更通知コールバック
typedef void (*config_change_callback_t)(void);

// 設定ダイアログの表示
void show_config_dialog(config_dialog_type_t type);

// 設定の初期化・保存・読み込み
void init_config_dialog_system(void);
void cleanup_config_dialog_system(void);
void apply_config_changes(void);
void cancel_config_changes(void);

// 個別設定ダイアログ
void show_general_config_dialog(void);
void show_sound_config_dialog(void);
void show_screen_config_dialog(void);
void show_input_config_dialog(void);
void show_control_config_dialog(void);
void show_about_dialog(void);

// 音量設定専用ダイアログ
void show_volume_dialog(void);

// 設定変更コールバックの設定
void set_config_change_callback(config_change_callback_t callback);

// 設定値の取得・設定（主要項目）
// 音声設定
int get_sound_frequency(void);
void set_sound_frequency(int frequency);
int get_sound_latency(void);
void set_sound_latency(int latency);
bool get_sound_enabled(void);
void set_sound_enabled(bool enabled);
int get_sound_volume_l(int channel);
void set_sound_volume_l(int channel, int volume);
int get_sound_volume_r(int channel);
void set_sound_volume_r(int channel, int volume);

// 画面設定
int get_window_mode(void);
void set_window_mode(int mode);
int get_window_stretch_type(void);
void set_window_stretch_type(int type);
int get_fullscreen_stretch_type(void);
void set_fullscreen_stretch_type(int type);
int get_rotate_type(void);
void set_rotate_type(int type);

// 制御設定
int get_cpu_power(void);
void set_cpu_power(int power);
bool get_full_speed(void);
void set_full_speed(bool enabled);
bool get_drive_vm_in_opecode(void);
void set_drive_vm_in_opecode(bool enabled);

// フィルター設定
#ifdef USE_SCREEN_FILTER
int get_filter_type(void);
void set_filter_type(int type);
#endif

// プラットフォーム固有の実装
#ifdef __APPLE__
// macOS/iOS用NSAlert/UIAlertControllerベースの実装
void* create_config_alert_controller(config_dialog_type_t type);
void present_config_alert_controller(void* controller);
void dismiss_config_alert_controller(void* controller);

// NSUserDefaults/UserDefaultsを使った設定保存
void save_config_to_user_defaults(void);
void load_config_from_user_defaults(void);
#endif

#ifdef __cplusplus
}
#endif