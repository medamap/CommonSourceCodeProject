//
// alert_dialog.h
// 汎用アラートダイアログシステム
//
// Author: Medamap and Claude
// Date: 2025.01.29
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// アラートの種類
typedef enum {
    ALERT_TYPE_INFO = 0,        // 情報表示
    ALERT_TYPE_WARNING,         // 警告
    ALERT_TYPE_ERROR,           // エラー
    ALERT_TYPE_QUESTION,        // 質問（Yes/No）
    ALERT_TYPE_CONFIRMATION,    // 確認（OK/Cancel）
    ALERT_TYPE_CUSTOM           // カスタム
} alert_type_t;

// アラートボタンの種類
typedef enum {
    ALERT_BUTTON_OK = 0,        // OKボタンのみ
    ALERT_BUTTON_OK_CANCEL,     // OK/キャンセル
    ALERT_BUTTON_YES_NO,        // はい/いいえ
    ALERT_BUTTON_YES_NO_CANCEL, // はい/いいえ/キャンセル
    ALERT_BUTTON_RETRY_CANCEL,  // 再試行/キャンセル
    ALERT_BUTTON_CUSTOM         // カスタムボタン
} alert_button_type_t;

// アラート応答
typedef enum {
    ALERT_RESPONSE_OK = 0,      // OK選択
    ALERT_RESPONSE_CANCEL,      // キャンセル選択
    ALERT_RESPONSE_YES,         // はい選択
    ALERT_RESPONSE_NO,          // いいえ選択
    ALERT_RESPONSE_RETRY,       // 再試行選択
    ALERT_RESPONSE_CUSTOM1,     // カスタムボタン1
    ALERT_RESPONSE_CUSTOM2,     // カスタムボタン2
    ALERT_RESPONSE_CUSTOM3,     // カスタムボタン3
    ALERT_RESPONSE_ERROR        // エラー/タイムアウト
} alert_response_t;

// アラートコールバック関数型
typedef void (*alert_callback_t)(alert_response_t response, void* user_data);

// アラート設定構造体
typedef struct {
    const char* title;                  // タイトル
    const char* message;                // メッセージ本文
    alert_type_t type;                  // アラートタイプ
    alert_button_type_t button_type;    // ボタンタイプ
    const char* custom_button1;         // カスタムボタン1のテキスト
    const char* custom_button2;         // カスタムボタン2のテキスト
    const char* custom_button3;         // カスタムボタン3のテキスト
    alert_callback_t callback;          // 応答コールバック
    void* user_data;                    // ユーザーデータ
    bool modal;                         // モーダル表示するか
    int timeout_seconds;                // タイムアウト（秒、0=無制限）
} alert_config_t;

// アラートシステムの初期化・クリーンアップ
void init_alert_system(void);
void cleanup_alert_system(void);

// 基本アラート表示関数
alert_response_t show_alert(const char* title, const char* message, alert_type_t type);
alert_response_t show_alert_with_buttons(const char* title, const char* message, alert_button_type_t button_type);
void show_alert_async(const char* title, const char* message, alert_type_t type, alert_callback_t callback, void* user_data);

// 詳細設定アラート
alert_response_t show_custom_alert(const alert_config_t* config);
void show_custom_alert_async(const alert_config_t* config);

// よく使われるアラート（簡易関数）
void show_info(const char* title, const char* message);
void show_warning(const char* title, const char* message);
void show_error(const char* title, const char* message);
alert_response_t ask_yes_no(const char* title, const char* message);
alert_response_t ask_ok_cancel(const char* title, const char* message);

// エミュレータ固有のアラート
void show_emulator_error(const char* operation, const char* details);
void show_file_error(const char* filename, const char* operation, const char* error);
alert_response_t ask_overwrite_file(const char* filename);
alert_response_t ask_save_before_exit(void);
void show_disk_insert_notification(const char* disk_name, int drive_number);
void show_disk_eject_notification(int drive_number);

// 進行状況表示（将来拡張用）
void show_progress_alert(const char* title, const char* message, int progress_percent);
void hide_progress_alert(void);

// プラットフォーム固有の実装
#ifdef __APPLE__
// macOS/iOS用NSAlert/UIAlertControllerベースの実装
void* create_native_alert(const alert_config_t* config);
alert_response_t present_modal_alert(void* alert_controller);
void present_async_alert(void* alert_controller, alert_callback_t callback, void* user_data);
void dismiss_alert(void* alert_controller);

// プラットフォーム固有のヘルパー関数
const char* get_platform_default_title(alert_type_t type);
const char* get_platform_button_text(alert_response_t response);
#endif

// デバッグ・テスト用
void test_all_alert_types(void);
void show_debug_alert(const char* debug_info);

#ifdef __cplusplus
}
#endif