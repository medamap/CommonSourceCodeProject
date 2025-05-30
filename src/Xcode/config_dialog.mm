//
// config_dialog.mm
// エミュレータ設定ダイアログシステム実装
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

#include "config_dialog.h"
#include "../config.h"
#include "../emu.h"
#include <stdio.h>
#include <string.h>

// グローバル変数
static config_change_callback_t g_config_callback = nullptr;
static config_t g_temp_config; // 一時的な設定保存用

extern EMU* emu;

////////////////////////////////////////////////////////////////////////////////
// 設定ダイアログシステムの初期化・クリーンアップ
////////////////////////////////////////////////////////////////////////////////

void init_config_dialog_system(void)
{
    printf("設定ダイアログシステムを初期化中...\\n");
    
    // 現在の設定を一時保存領域にコピー
    memcpy(&g_temp_config, &config, sizeof(config_t));
    
#ifdef __APPLE__
    // Apple固有の初期化
    load_config_from_user_defaults();
#endif
    
    printf("設定ダイアログシステム初期化完了\\n");
}

void cleanup_config_dialog_system(void)
{
    printf("設定ダイアログシステムをクリーンアップ中...\\n");
    
    g_config_callback = nullptr;
    
    printf("設定ダイアログシステムクリーンアップ完了\\n");
}

void set_config_change_callback(config_change_callback_t callback)
{
    g_config_callback = callback;
}

////////////////////////////////////////////////////////////////////////////////
// 設定の適用・キャンセル
////////////////////////////////////////////////////////////////////////////////

void apply_config_changes(void)
{
    printf("設定変更を適用中...\\n");
    
    // 一時設定を実際の設定にコピー
    memcpy(&config, &g_temp_config, sizeof(config_t));
    
    // エミュレータに設定変更を通知
    if (emu) {
        emu->update_config();
        printf("エミュレータ設定を更新しました\\n");
    }
    
#ifdef __APPLE__
    // Apple固有の設定保存
    save_config_to_user_defaults();
#endif
    
    // コールバック通知
    if (g_config_callback) {
        g_config_callback();
    }
    
    printf("設定変更適用完了\\n");
}

void cancel_config_changes(void)
{
    printf("設定変更をキャンセル中...\\n");
    
    // 一時設定を元の設定に戻す
    memcpy(&g_temp_config, &config, sizeof(config_t));
    
    printf("設定変更キャンセル完了\\n");
}

////////////////////////////////////////////////////////////////////////////////
// 音声設定の取得・設定
////////////////////////////////////////////////////////////////////////////////

int get_sound_frequency(void)
{
    return config.sound_frequency;
}

void set_sound_frequency(int frequency)
{
    g_temp_config.sound_frequency = frequency;
    printf("音声周波数設定: %d Hz\\n", frequency);
}

int get_sound_latency(void)
{
    return config.sound_latency;
}

void set_sound_latency(int latency)
{
    g_temp_config.sound_latency = latency;
    printf("音声レイテンシ設定: %d ms\\n", latency);
}

bool get_sound_enabled(void)
{
#if defined(__ANDROID__)
    return config.sound_on;
#else
    return true; // macOS/iOSでは常に有効
#endif
}

void set_sound_enabled(bool enabled)
{
#if defined(__ANDROID__)
    g_temp_config.sound_on = enabled;
#endif
    printf("音声有効設定: %s\\n", enabled ? "有効" : "無効");
}

#ifdef USE_SOUND_VOLUME
int get_sound_volume_l(int channel)
{
    if (channel < 0 || channel >= MAX_VOLUME_TMP) return 0;
    return config.sound_volume_l[channel];
}

void set_sound_volume_l(int channel, int volume)
{
    if (channel < 0 || channel >= MAX_VOLUME_TMP) return;
    g_temp_config.sound_volume_l[channel] = volume;
    printf("音量設定 L[%d]: %d\\n", channel, volume);
}

int get_sound_volume_r(int channel)
{
    if (channel < 0 || channel >= MAX_VOLUME_TMP) return 0;
    return config.sound_volume_r[channel];
}

void set_sound_volume_r(int channel, int volume)
{
    if (channel < 0 || channel >= MAX_VOLUME_TMP) return;
    g_temp_config.sound_volume_r[channel] = volume;
    printf("音量設定 R[%d]: %d\\n", channel, volume);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// 画面設定の取得・設定
////////////////////////////////////////////////////////////////////////////////

int get_window_mode(void)
{
    return config.window_mode;
}

void set_window_mode(int mode)
{
    g_temp_config.window_mode = mode;
    printf("ウィンドウモード設定: %d\\n", mode);
}

int get_window_stretch_type(void)
{
    return config.window_stretch_type;
}

void set_window_stretch_type(int type)
{
    g_temp_config.window_stretch_type = type;
    printf("ウィンドウ拡大タイプ設定: %d\\n", type);
}

int get_fullscreen_stretch_type(void)
{
    return config.fullscreen_stretch_type;
}

void set_fullscreen_stretch_type(int type)
{
    g_temp_config.fullscreen_stretch_type = type;
    printf("フルスクリーン拡大タイプ設定: %d\\n", type);
}

int get_rotate_type(void)
{
    return config.rotate_type;
}

void set_rotate_type(int type)
{
    g_temp_config.rotate_type = type;
    printf("回転タイプ設定: %d\\n", type);
}

#ifdef USE_SCREEN_FILTER
int get_filter_type(void)
{
    return config.filter_type;
}

void set_filter_type(int type)
{
    g_temp_config.filter_type = type;
    printf("フィルタータイプ設定: %d\\n", type);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// 制御設定の取得・設定
////////////////////////////////////////////////////////////////////////////////

int get_cpu_power(void)
{
    return config.cpu_power;
}

void set_cpu_power(int power)
{
    g_temp_config.cpu_power = power;
    printf("CPU電力設定: %d\\n", power);
}

bool get_full_speed(void)
{
    return config.full_speed;
}

void set_full_speed(bool enabled)
{
    g_temp_config.full_speed = enabled;
    printf("フルスピード設定: %s\\n", enabled ? "有効" : "無効");
}

bool get_drive_vm_in_opecode(void)
{
    return config.drive_vm_in_opecode;
}

void set_drive_vm_in_opecode(bool enabled)
{
    g_temp_config.drive_vm_in_opecode = enabled;
    printf("オペコード実行設定: %s\\n", enabled ? "有効" : "無効");
}

////////////////////////////////////////////////////////////////////////////////
// Apple固有の実装
////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

#if TARGET_OS_OSX
// macOS用NSAlert実装
void* create_config_alert_controller(config_dialog_type_t type)
{
    NSAlert* alert = [[NSAlert alloc] init];
    
    switch (type) {
        case CONFIG_DIALOG_GENERAL:
            [alert setMessageText:@"一般設定"];
            [alert setInformativeText:@"エミュレータの一般的な設定を変更します。"];
            [alert addButtonWithTitle:@"OK"];
            [alert addButtonWithTitle:@"キャンセル"];
            break;
            
        case CONFIG_DIALOG_SOUND:
            [alert setMessageText:@"音声設定"];
            [alert setInformativeText:@"音声の周波数、レイテンシ、音量を設定します。"];
            [alert addButtonWithTitle:@"音量ダイアログを開く"];
            [alert addButtonWithTitle:@"キャンセル"];
            break;
            
        case CONFIG_DIALOG_SCREEN:
            [alert setMessageText:@"画面設定"];
            [alert setInformativeText:@"画面表示の設定を変更します。"];
            [alert addButtonWithTitle:@"OK"];
            [alert addButtonWithTitle:@"キャンセル"];
            break;
            
        case CONFIG_DIALOG_INPUT:
            [alert setMessageText:@"入力設定"];
            [alert setInformativeText:@"キーボード、マウス、ジョイスティックの設定を変更します。"];
            [alert addButtonWithTitle:@"OK"];
            [alert addButtonWithTitle:@"キャンセル"];
            break;
            
        case CONFIG_DIALOG_CONTROL:
            [alert setMessageText:@"制御設定"];
            [alert setInformativeText:@"CPU電力、実行速度などの制御設定を変更します。"];
            [alert addButtonWithTitle:@"OK"];
            [alert addButtonWithTitle:@"キャンセル"];
            break;
            
        case CONFIG_DIALOG_ABOUT:
            [alert setMessageText:@"バージョン情報"];
            [alert setInformativeText:@"CommonSourceCodeProject エミュレータ\\nXcode版 v1.0\\n\\n開発: Medamap and Claude\\n2025年1月"];
            [alert addButtonWithTitle:@"OK"];
            break;
            
        default:
            [alert setMessageText:@"設定"];
            [alert setInformativeText:@"設定ダイアログ"];
            [alert addButtonWithTitle:@"OK"];
            break;
    }
    
    return (__bridge_retained void*)alert;
}

void present_config_alert_controller(void* controller)
{
    NSAlert* alert = (__bridge NSAlert*)controller;
    
    // モーダルダイアログとして表示
    NSModalResponse response = [alert runModal];
    
    switch (response) {
        case NSAlertFirstButtonReturn:
            printf("設定ダイアログ: OK選択\\n");
            apply_config_changes();
            break;
        case NSAlertSecondButtonReturn:
            printf("設定ダイアログ: キャンセル選択\\n");
            cancel_config_changes();
            break;
        default:
            printf("設定ダイアログ: その他選択 (%ld)\\n", (long)response);
            break;
    }
}

void dismiss_config_alert_controller(void* controller)
{
    NSAlert* alert = (__bridge_transfer NSAlert*)controller;
    // ARCが自動的にメモリ管理
    alert = nil;
}

#elif TARGET_OS_IOS
// iOS用UIAlertController実装
void* create_config_alert_controller(config_dialog_type_t type)
{
    UIAlertController* alert = nil;
    
    switch (type) {
        case CONFIG_DIALOG_GENERAL:
            alert = [UIAlertController alertControllerWithTitle:@"一般設定"
                                                       message:@"エミュレータの一般的な設定を変更します。"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        case CONFIG_DIALOG_SOUND:
            alert = [UIAlertController alertControllerWithTitle:@"音声設定"
                                                       message:@"音声の周波数、レイテンシ、音量を設定します。"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        case CONFIG_DIALOG_SCREEN:
            alert = [UIAlertController alertControllerWithTitle:@"画面設定"
                                                       message:@"画面表示の設定を変更します。"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        case CONFIG_DIALOG_INPUT:
            alert = [UIAlertController alertControllerWithTitle:@"入力設定"
                                                       message:@"キーボード、マウス、ジョイスティックの設定を変更します。"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        case CONFIG_DIALOG_CONTROL:
            alert = [UIAlertController alertControllerWithTitle:@"制御設定"
                                                       message:@"CPU電力、実行速度などの制御設定を変更します。"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        case CONFIG_DIALOG_ABOUT:
            alert = [UIAlertController alertControllerWithTitle:@"バージョン情報"
                                                       message:@"CommonSourceCodeProject エミュレータ\\nXcode版 v1.0\\n\\n開発: Medamap and Claude\\n2025年1月"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
            
        default:
            alert = [UIAlertController alertControllerWithTitle:@"設定"
                                                       message:@"設定ダイアログ"
                                                preferredStyle:UIAlertControllerStyleAlert];
            break;
    }
    
    // OKアクション
    UIAlertAction* okAction = [UIAlertAction actionWithTitle:@"OK"
                                                       style:UIAlertActionStyleDefault
                                                     handler:^(UIAlertAction * action) {
                                                         printf("設定ダイアログ: OK選択\\n");
                                                         apply_config_changes();
                                                     }];
    [alert addAction:okAction];
    
    // キャンセルアクション（バージョン情報以外）
    if (type != CONFIG_DIALOG_ABOUT) {
        UIAlertAction* cancelAction = [UIAlertAction actionWithTitle:@"キャンセル"
                                                               style:UIAlertActionStyleCancel
                                                             handler:^(UIAlertAction * action) {
                                                                 printf("設定ダイアログ: キャンセル選択\\n");
                                                                 cancel_config_changes();
                                                             }];
        [alert addAction:cancelAction];
    }
    
    return (__bridge_retained void*)alert;
}

void present_config_alert_controller(void* controller)
{
    UIAlertController* alert = (__bridge UIAlertController*)controller;
    
    // 現在のView Controllerを取得してモーダル表示
    UIViewController* rootVC = [UIApplication sharedApplication].keyWindow.rootViewController;
    if (rootVC) {
        [rootVC presentViewController:alert animated:YES completion:nil];
        printf("設定ダイアログを表示しました\\n");
    } else {
        printf("エラー: Root View Controllerが見つかりません\\n");
    }
}

void dismiss_config_alert_controller(void* controller)
{
    UIAlertController* alert = (__bridge_transfer UIAlertController*)controller;
    // ARCが自動的にメモリ管理
    alert = nil;
}
#endif

// Apple共通のUserDefaults実装
void save_config_to_user_defaults(void)
{
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    
    // 音声設定
    [defaults setInteger:config.sound_frequency forKey:@"sound_frequency"];
    [defaults setInteger:config.sound_latency forKey:@"sound_latency"];
    
    // 画面設定
    [defaults setInteger:config.window_mode forKey:@"window_mode"];
    [defaults setInteger:config.window_stretch_type forKey:@"window_stretch_type"];
    [defaults setInteger:config.fullscreen_stretch_type forKey:@"fullscreen_stretch_type"];
    [defaults setInteger:config.rotate_type forKey:@"rotate_type"];
    
    // 制御設定
    [defaults setInteger:config.cpu_power forKey:@"cpu_power"];
    [defaults setBool:config.full_speed forKey:@"full_speed"];
    [defaults setBool:config.drive_vm_in_opecode forKey:@"drive_vm_in_opecode"];
    
#ifdef USE_SCREEN_FILTER
    [defaults setInteger:config.filter_type forKey:@"filter_type"];
#endif
    
    [defaults synchronize];
    printf("設定をUserDefaultsに保存しました\\n");
}

void load_config_from_user_defaults(void)
{
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    
    // 音声設定（デフォルト値付き）
    if ([defaults objectForKey:@"sound_frequency"]) {
        config.sound_frequency = (int)[defaults integerForKey:@"sound_frequency"];
    }
    if ([defaults objectForKey:@"sound_latency"]) {
        config.sound_latency = (int)[defaults integerForKey:@"sound_latency"];
    }
    
    // 画面設定
    if ([defaults objectForKey:@"window_mode"]) {
        config.window_mode = (int)[defaults integerForKey:@"window_mode"];
    }
    if ([defaults objectForKey:@"window_stretch_type"]) {
        config.window_stretch_type = (int)[defaults integerForKey:@"window_stretch_type"];
    }
    if ([defaults objectForKey:@"fullscreen_stretch_type"]) {
        config.fullscreen_stretch_type = (int)[defaults integerForKey:@"fullscreen_stretch_type"];
    }
    if ([defaults objectForKey:@"rotate_type"]) {
        config.rotate_type = (int)[defaults integerForKey:@"rotate_type"];
    }
    
    // 制御設定
    if ([defaults objectForKey:@"cpu_power"]) {
        config.cpu_power = (int)[defaults integerForKey:@"cpu_power"];
    }
    if ([defaults objectForKey:@"full_speed"]) {
        config.full_speed = [defaults boolForKey:@"full_speed"];
    }
    if ([defaults objectForKey:@"drive_vm_in_opecode"]) {
        config.drive_vm_in_opecode = [defaults boolForKey:@"drive_vm_in_opecode"];
    }
    
#ifdef USE_SCREEN_FILTER
    if ([defaults objectForKey:@"filter_type"]) {
        config.filter_type = (int)[defaults integerForKey:@"filter_type"];
    }
#endif
    
    printf("設定をUserDefaultsから読み込みました\\n");
}

#endif // __APPLE__

////////////////////////////////////////////////////////////////////////////////
// 各種設定ダイアログの表示
////////////////////////////////////////////////////////////////////////////////

void show_config_dialog(config_dialog_type_t type)
{
    printf("設定ダイアログ表示: タイプ %d\\n", (int)type);
    
#ifdef __APPLE__
    void* controller = create_config_alert_controller(type);
    if (controller) {
        present_config_alert_controller(controller);
        dismiss_config_alert_controller(controller);
    }
#else
    printf("設定ダイアログ: プラットフォーム未対応\\n");
#endif
}

void show_general_config_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_GENERAL);
}

void show_sound_config_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_SOUND);
}

void show_screen_config_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_SCREEN);
}

void show_input_config_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_INPUT);
}

void show_control_config_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_CONTROL);
}

void show_about_dialog(void)
{
    show_config_dialog(CONFIG_DIALOG_ABOUT);
}

void show_volume_dialog(void)
{
    printf("音量ダイアログ表示\\n");
    
#ifdef __APPLE__
    // 音量専用ダイアログの実装
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"音量設定"];
    [alert setInformativeText:@"各チャンネルの音量を調整します。\\n\\n注意: 詳細な音量調整は将来の版で実装予定です。"];
    [alert addButtonWithTitle:@"OK"];
    
    NSModalResponse response = [alert runModal];
    printf("音量ダイアログ応答: %ld\\n", (long)response);
#endif
}