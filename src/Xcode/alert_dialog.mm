//
// alert_dialog.mm
// 汎用アラートダイアログシステム実装
//
// Author: Medamap and Claude
// Date: 2025.01.29
//

#ifdef __APPLE__
// Objective-Cヘッダーを先に読み込む（BOOLの競合を避けるため）
#import <Foundation/Foundation.h>
#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#elif TARGET_OS_IOS
#import <UIKit/UIKit.h>
#endif
#endif

#ifdef BOOL
#undef BOOL
#endif
#define BOOL int

#include "alert_dialog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// グローバル変数
static bool g_alert_system_initialized = false;
static void* g_current_progress_alert = nullptr;

////////////////////////////////////////////////////////////////////////////////
// アラートシステムの初期化・クリーンアップ
////////////////////////////////////////////////////////////////////////////////

void init_alert_system(void)
{
    if (g_alert_system_initialized) {
        printf("警告: アラートシステムは既に初期化されています\n");
        return;
    }
    
    printf("アラートシステムを初期化中...\n");
    
    g_current_progress_alert = nullptr;
    g_alert_system_initialized = true;
    
    printf("アラートシステム初期化完了\n");
}

void cleanup_alert_system(void)
{
    if (!g_alert_system_initialized) {
        printf("警告: アラートシステムは初期化されていません\n");
        return;
    }
    
    printf("アラートシステムをクリーンアップ中...\n");
    
    // 進行状況アラートが表示中の場合は閉じる
    if (g_current_progress_alert) {
        hide_progress_alert();
    }
    
    g_alert_system_initialized = false;
    
    printf("アラートシステムクリーンアップ完了\n");
}

////////////////////////////////////////////////////////////////////////////////
// プラットフォーム固有のヘルパー関数
////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

const char* get_platform_default_title(alert_type_t type)
{
    switch (type) {
        case ALERT_TYPE_INFO:
            return "情報";
        case ALERT_TYPE_WARNING:
            return "警告";
        case ALERT_TYPE_ERROR:
            return "エラー";
        case ALERT_TYPE_QUESTION:
            return "確認";
        case ALERT_TYPE_CONFIRMATION:
            return "確認";
        case ALERT_TYPE_CUSTOM:
        default:
            return "通知";
    }
}

const char* get_platform_button_text(alert_response_t response)
{
    switch (response) {
        case ALERT_RESPONSE_OK:
            return "OK";
        case ALERT_RESPONSE_CANCEL:
            return "キャンセル";
        case ALERT_RESPONSE_YES:
            return "はい";
        case ALERT_RESPONSE_NO:
            return "いいえ";
        case ALERT_RESPONSE_RETRY:
            return "再試行";
        default:
            return "OK";
    }
}

#if TARGET_OS_OSX
// macOS用NSAlert実装
void* create_native_alert(const alert_config_t* config)
{
    NSAlert* alert = [[NSAlert alloc] init];
    
    // タイトル設定
    [alert setMessageText:[NSString stringWithUTF8String:config->title ? config->title : get_platform_default_title(config->type)]];
    
    // メッセージ設定
    if (config->message) {
        [alert setInformativeText:[NSString stringWithUTF8String:config->message]];
    }
    
    // アラートスタイル設定
    switch (config->type) {
        case ALERT_TYPE_INFO:
            [alert setAlertStyle:NSAlertStyleInformational];
            break;
        case ALERT_TYPE_WARNING:
            [alert setAlertStyle:NSAlertStyleWarning];
            break;
        case ALERT_TYPE_ERROR:
            [alert setAlertStyle:NSAlertStyleCritical];
            break;
        case ALERT_TYPE_QUESTION:
        case ALERT_TYPE_CONFIRMATION:
        case ALERT_TYPE_CUSTOM:
        default:
            [alert setAlertStyle:NSAlertStyleInformational];
            break;
    }
    
    // ボタン設定
    switch (config->button_type) {
        case ALERT_BUTTON_OK:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]];
            break;
            
        case ALERT_BUTTON_OK_CANCEL:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_CANCEL)]];
            break;
            
        case ALERT_BUTTON_YES_NO:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_YES)]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_NO)]];
            break;
            
        case ALERT_BUTTON_YES_NO_CANCEL:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_YES)]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_NO)]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_CANCEL)]];
            break;
            
        case ALERT_BUTTON_RETRY_CANCEL:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_RETRY)]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_CANCEL)]];
            break;
            
        case ALERT_BUTTON_CUSTOM:
            if (config->custom_button1) {
                [alert addButtonWithTitle:[NSString stringWithUTF8String:config->custom_button1]];
            }
            if (config->custom_button2) {
                [alert addButtonWithTitle:[NSString stringWithUTF8String:config->custom_button2]];
            }
            if (config->custom_button3) {
                [alert addButtonWithTitle:[NSString stringWithUTF8String:config->custom_button3]];
            }
            break;
            
        default:
            [alert addButtonWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]];
            break;
    }
    
    return (__bridge_retained void*)alert;
}

alert_response_t present_modal_alert(void* alert_controller)
{
    NSAlert* alert = (__bridge NSAlert*)alert_controller;
    
    NSModalResponse response = [alert runModal];
    
    // レスポンスコードをalert_response_tに変換
    alert_response_t result = ALERT_RESPONSE_OK;
    
    switch (response) {
        case NSAlertFirstButtonReturn:
            result = ALERT_RESPONSE_OK; // または最初のボタンに対応
            break;
        case NSAlertSecondButtonReturn:
            result = ALERT_RESPONSE_CANCEL; // または2番目のボタンに対応
            break;
        case NSAlertThirdButtonReturn:
            result = ALERT_RESPONSE_NO; // または3番目のボタンに対応
            break;
        default:
            result = ALERT_RESPONSE_ERROR;
            break;
    }
    
    printf("アラート応答: %d\\n", (int)result);
    return result;
}

void present_async_alert(void* alert_controller, alert_callback_t callback, void* user_data)
{
    NSAlert* alert = (__bridge NSAlert*)alert_controller;
    
    // 非同期でアラートを表示（macOSではbeginSheetModalForWindowを使用）
    // 簡単のため、ここではモーダル表示を使用
    NSModalResponse response = [alert runModal];
    
    // コールバック呼び出し
    if (callback) {
        alert_response_t result = ALERT_RESPONSE_OK;
        
        switch (response) {
            case NSAlertFirstButtonReturn:
                result = ALERT_RESPONSE_OK;
                break;
            case NSAlertSecondButtonReturn:
                result = ALERT_RESPONSE_CANCEL;
                break;
            case NSAlertThirdButtonReturn:
                result = ALERT_RESPONSE_NO;
                break;
            default:
                result = ALERT_RESPONSE_ERROR;
                break;
        }
        
        callback(result, user_data);
    }
}

void dismiss_alert(void* alert_controller)
{
    NSAlert* alert = (__bridge_transfer NSAlert*)alert_controller;
    // ARCが自動的にメモリ管理
    alert = nil;
}

#elif TARGET_OS_IOS
// iOS用UIAlertController実装
void* create_native_alert(const alert_config_t* config)
{
    UIAlertController* alert = [UIAlertController
        alertControllerWithTitle:[NSString stringWithUTF8String:config->title ? config->title : get_platform_default_title(config->type)]
        message:config->message ? [NSString stringWithUTF8String:config->message] : nil
        preferredStyle:UIAlertControllerStyleAlert];
    
    // ボタン設定（クロージャでコールバックを呼び出し）
    switch (config->button_type) {
        case ALERT_BUTTON_OK:
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]
                                                      style:UIAlertActionStyleDefault
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_OK, config->user_data);
                                                        }
                                                    }]];
            break;
            
        case ALERT_BUTTON_OK_CANCEL:
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]
                                                      style:UIAlertActionStyleDefault
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_OK, config->user_data);
                                                        }
                                                    }]];
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_CANCEL)]
                                                      style:UIAlertActionStyleCancel
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_CANCEL, config->user_data);
                                                        }
                                                    }]];
            break;
            
        case ALERT_BUTTON_YES_NO:
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_YES)]
                                                      style:UIAlertActionStyleDefault
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_YES, config->user_data);
                                                        }
                                                    }]];
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_NO)]
                                                      style:UIAlertActionStyleDefault
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_NO, config->user_data);
                                                        }
                                                    }]];
            break;
            
        // 他のボタンタイプも同様に実装...
        
        default:
            [alert addAction:[UIAlertAction actionWithTitle:[NSString stringWithUTF8String:get_platform_button_text(ALERT_RESPONSE_OK)]
                                                      style:UIAlertActionStyleDefault
                                                    handler:^(UIAlertAction * action) {
                                                        if (config->callback) {
                                                            config->callback(ALERT_RESPONSE_OK, config->user_data);
                                                        }
                                                    }]];
            break;
    }
    
    return (__bridge_retained void*)alert;
}

alert_response_t present_modal_alert(void* alert_controller)
{
    // iOSではモーダル表示は非同期のため、簡単のため同期的な応答を返す
    printf("iOS: モーダルアラート表示（同期応答は制限あり）\\n");
    return ALERT_RESPONSE_OK;
}

void present_async_alert(void* alert_controller, alert_callback_t callback, void* user_data)
{
    UIAlertController* alert = (__bridge UIAlertController*)alert_controller;
    
    // 現在のView Controllerを取得してモーダル表示
    UIViewController* rootVC = [UIApplication sharedApplication].keyWindow.rootViewController;
    if (rootVC) {
        [rootVC presentViewController:alert animated:YES completion:nil];
        printf("iOS: 非同期アラート表示完了\\n");
    } else {
        printf("エラー: iOS Root View Controllerが見つかりません\\n");
        if (callback) {
            callback(ALERT_RESPONSE_ERROR, user_data);
        }
    }
}

void dismiss_alert(void* alert_controller)
{
    UIAlertController* alert = (__bridge_transfer UIAlertController*)alert_controller;
    // ARCが自動的にメモリ管理
    alert = nil;
}

#endif

#endif // __APPLE__

////////////////////////////////////////////////////////////////////////////////
// 基本アラート表示関数
////////////////////////////////////////////////////////////////////////////////

alert_response_t show_alert(const char* title, const char* message, alert_type_t type)
{
    if (!g_alert_system_initialized) {
        init_alert_system();
    }
    
    printf("アラート表示: タイトル='%s', メッセージ='%s', タイプ=%d\\n", 
           title ? title : "(なし)", 
           message ? message : "(なし)", 
           (int)type);
    
    alert_config_t config = {0};
    config.title = title;
    config.message = message;
    config.type = type;
    config.button_type = ALERT_BUTTON_OK;
    config.modal = true;
    
    return show_custom_alert(&config);
}

alert_response_t show_alert_with_buttons(const char* title, const char* message, alert_button_type_t button_type)
{
    if (!g_alert_system_initialized) {
        init_alert_system();
    }
    
    alert_config_t config = {0};
    config.title = title;
    config.message = message;
    config.type = ALERT_TYPE_INFO;
    config.button_type = button_type;
    config.modal = true;
    
    return show_custom_alert(&config);
}

void show_alert_async(const char* title, const char* message, alert_type_t type, alert_callback_t callback, void* user_data)
{
    if (!g_alert_system_initialized) {
        init_alert_system();
    }
    
    alert_config_t config = {0};
    config.title = title;
    config.message = message;
    config.type = type;
    config.button_type = ALERT_BUTTON_OK;
    config.callback = callback;
    config.user_data = user_data;
    config.modal = false;
    
    show_custom_alert_async(&config);
}

alert_response_t show_custom_alert(const alert_config_t* config)
{
    if (!config) {
        printf("エラー: アラート設定がnullです\\n");
        return ALERT_RESPONSE_ERROR;
    }
    
#ifdef __APPLE__
    void* alert_controller = create_native_alert(config);
    if (!alert_controller) {
        printf("エラー: ネイティブアラートの作成に失敗しました\\n");
        return ALERT_RESPONSE_ERROR;
    }
    
    alert_response_t response = present_modal_alert(alert_controller);
    dismiss_alert(alert_controller);
    
    return response;
#else
    printf("アラート: %s - %s\\n", 
           config->title ? config->title : "通知", 
           config->message ? config->message : "");
    return ALERT_RESPONSE_OK;
#endif
}

void show_custom_alert_async(const alert_config_t* config)
{
    if (!config) {
        printf("エラー: アラート設定がnullです\\n");
        return;
    }
    
#ifdef __APPLE__
    void* alert_controller = create_native_alert(config);
    if (!alert_controller) {
        printf("エラー: ネイティブアラートの作成に失敗しました\\n");
        if (config->callback) {
            config->callback(ALERT_RESPONSE_ERROR, config->user_data);
        }
        return;
    }
    
    present_async_alert(alert_controller, config->callback, config->user_data);
    // alert_controllerは非同期処理で自動的に解放される
#else
    printf("非同期アラート: %s - %s\\n", 
           config->title ? config->title : "通知", 
           config->message ? config->message : "");
    if (config->callback) {
        config->callback(ALERT_RESPONSE_OK, config->user_data);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
// よく使われるアラート（簡易関数）
////////////////////////////////////////////////////////////////////////////////

void show_info(const char* title, const char* message)
{
    show_alert(title, message, ALERT_TYPE_INFO);
}

void show_warning(const char* title, const char* message)
{
    show_alert(title, message, ALERT_TYPE_WARNING);
}

void show_error(const char* title, const char* message)
{
    show_alert(title, message, ALERT_TYPE_ERROR);
}

alert_response_t ask_yes_no(const char* title, const char* message)
{
    return show_alert_with_buttons(title, message, ALERT_BUTTON_YES_NO);
}

alert_response_t ask_ok_cancel(const char* title, const char* message)
{
    return show_alert_with_buttons(title, message, ALERT_BUTTON_OK_CANCEL);
}

////////////////////////////////////////////////////////////////////////////////
// エミュレータ固有のアラート
////////////////////////////////////////////////////////////////////////////////

void show_emulator_error(const char* operation, const char* details)
{
    char message[512];
    snprintf(message, sizeof(message), 
             "操作: %s\\n\\nエラーの詳細:\\n%s", 
             operation ? operation : "不明", 
             details ? details : "詳細情報なし");
    
    show_error("エミュレータエラー", message);
}

void show_file_error(const char* filename, const char* operation, const char* error)
{
    char message[512];
    snprintf(message, sizeof(message), 
             "ファイル: %s\\n操作: %s\\n\\nエラー: %s", 
             filename ? filename : "不明", 
             operation ? operation : "不明", 
             error ? error : "不明なエラー");
    
    show_error("ファイルエラー", message);
}

alert_response_t ask_overwrite_file(const char* filename)
{
    char message[256];
    snprintf(message, sizeof(message), 
             "ファイル '%s' は既に存在します。\\n\\n上書きしますか？", 
             filename ? filename : "不明");
    
    return ask_yes_no("ファイル上書き確認", message);
}

alert_response_t ask_save_before_exit(void)
{
    return show_alert_with_buttons("保存確認", 
                                   "変更が保存されていません。\\n\\n終了前に保存しますか？", 
                                   ALERT_BUTTON_YES_NO_CANCEL);
}

void show_disk_insert_notification(const char* disk_name, int drive_number)
{
    char message[256];
    snprintf(message, sizeof(message), 
             "ディスク '%s' をドライブ %d に挿入しました。", 
             disk_name ? disk_name : "不明", 
             drive_number);
    
    show_info("ディスク挿入", message);
}

void show_disk_eject_notification(int drive_number)
{
    char message[128];
    snprintf(message, sizeof(message), 
             "ドライブ %d からディスクを取り出しました。", 
             drive_number);
    
    show_info("ディスク取り出し", message);
}

////////////////////////////////////////////////////////////////////////////////
// 進行状況表示（将来拡張用）
////////////////////////////////////////////////////////////////////////////////

void show_progress_alert(const char* title, const char* message, int progress_percent)
{
    // TODO: 進行状況バー付きアラートの実装
    printf("進行状況: %s - %s (%d%%)\\n", 
           title ? title : "処理中", 
           message ? message : "", 
           progress_percent);
}

void hide_progress_alert(void)
{
    if (g_current_progress_alert) {
        // TODO: 進行状況アラートを閉じる
        g_current_progress_alert = nullptr;
        printf("進行状況アラートを閉じました\\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
// デバッグ・テスト用
////////////////////////////////////////////////////////////////////////////////

void test_all_alert_types(void)
{
    printf("アラート型テスト開始\\n");
    
    show_info("情報テスト", "これは情報アラートのテストです。");
    show_warning("警告テスト", "これは警告アラートのテストです。");
    show_error("エラーテスト", "これはエラーアラートのテストです。");
    
    alert_response_t response = ask_yes_no("質問テスト", "これは質問アラートのテストです。\\n\\nテストを続行しますか？");
    printf("Yes/No応答: %d\\n", (int)response);
    
    response = ask_ok_cancel("確認テスト", "これは確認アラートのテストです。\\n\\n続行しますか？");
    printf("OK/Cancel応答: %d\\n", (int)response);
    
    printf("アラート型テスト完了\\n");
}

void show_debug_alert(const char* debug_info)
{
    char message[512];
    snprintf(message, sizeof(message), 
             "デバッグ情報:\\n\\n%s", 
             debug_info ? debug_info : "情報なし");
    
    show_info("デバッグ", message);
}