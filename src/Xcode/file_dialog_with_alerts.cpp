//
// file_dialog_with_alerts.cpp
// ファイルダイアログとアラートシステムの統合例
//
// Author: Medamap and Claude
// Date: 2025.01.29
//

#include "file_dialog.h"
#include "alert_dialog.h"
#include "../emu.h"
#include <stdio.h>
#include <string.h>

extern EMU* emu;

////////////////////////////////////////////////////////////////////////////////
// ファイル選択とアラートの統合例
////////////////////////////////////////////////////////////////////////////////

// ファイル読み込み時のエラーハンドリング付きコールバック
void on_floppy_disk_selected_with_alerts(const char* file_path, int drive_index, void* user_data)
{
    if (!file_path || !emu) {
        show_error("ファイル読み込みエラー", "ファイルパスまたはエミュレータが無効です。");
        return;
    }
    
    printf("フロッピーディスク選択: %s (ドライブ %d)\n", file_path, drive_index);
    
    // ファイルの存在確認
    FILE* test_file = fopen(file_path, "rb");
    if (!test_file) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), 
                 "ファイル '%s' を開けませんでした。\n\nファイルが存在するか、読み取り権限があるかを確認してください。", 
                 file_path);
        show_error("ファイル読み込みエラー", error_msg);
        return;
    }
    fclose(test_file);
    
    // エミュレータに読み込み
    bool success = emu->open_floppy_disk(file_path, drive_index);
    
    if (success) {
        // 成功通知
        char success_msg[256];
        const char* filename = strrchr(file_path, '/');
        if (!filename) filename = strrchr(file_path, '\\');
        if (!filename) filename = file_path;
        else filename++; // パス区切り文字をスキップ
        
        snprintf(success_msg, sizeof(success_msg), 
                 "ディスク '%s' をドライブ %d に挿入しました。", 
                 filename, drive_index);
        show_info("ディスク挿入完了", success_msg);
    } else {
        // エラー通知
        show_file_error(file_path, "ディスク挿入", "ファイル形式が不正か、ディスクが破損している可能性があります。");
    }
}

// テープファイル選択時のアラート統合
void on_tape_selected_with_alerts(const char* file_path, int drive_index, void* user_data)
{
    if (!file_path || !emu) {
        show_error("ファイル読み込みエラー", "ファイルパスまたはエミュレータが無効です。");
        return;
    }
    
    printf("テープファイル選択: %s (ドライブ %d)\n", file_path, drive_index);
    
    // ファイルサイズチェック（大きなファイルの場合は警告）
    FILE* test_file = fopen(file_path, "rb");
    if (!test_file) {
        show_file_error(file_path, "テープ読み込み", "ファイルを開けませんでした。");
        return;
    }
    
    fseek(test_file, 0, SEEK_END);
    long file_size = ftell(test_file);
    fclose(test_file);
    
    // 10MB以上の場合は警告
    if (file_size > 10 * 1024 * 1024) {
        char warning_msg[256];
        snprintf(warning_msg, sizeof(warning_msg), 
                 "選択されたファイル '%s' のサイズが大きい(%.1f MB)です。\n\n読み込みに時間がかかる可能性があります。続行しますか？", 
                 file_path, file_size / (1024.0 * 1024.0));
        
        alert_response_t response = ask_yes_no("大きなファイル", warning_msg);
        if (response != ALERT_RESPONSE_YES) {
            printf("テープ読み込みがキャンセルされました\n");
            return;
        }
    }
    
    // エミュレータに読み込み
    bool success = emu->play_tape(file_path);
    
    if (success) {
        const char* filename = strrchr(file_path, '/');
        if (!filename) filename = strrchr(file_path, '\\');
        if (!filename) filename = file_path;
        else filename++;
        
        char success_msg[256];
        snprintf(success_msg, sizeof(success_msg), 
                 "テープ '%s' の再生を開始しました。", filename);
        show_info("テープ再生開始", success_msg);
    } else {
        show_file_error(file_path, "テープ再生", "テープファイルの形式が不正です。");
    }
}

// ハードディスク選択時のアラート統合
void on_hard_disk_selected_with_alerts(const char* file_path, int drive_index, void* user_data)
{
    if (!file_path || !emu) {
        show_error("ファイル読み込みエラー", "ファイルパスまたはエミュレータが無効です。");
        return;
    }
    
    printf("ハードディスク選択: %s (ドライブ %d)\n", file_path, drive_index);
    
    // 既にハードディスクが挿入されている場合は確認
    if (emu->is_hard_disk_inserted(drive_index)) {
        char confirm_msg[256];
        snprintf(confirm_msg, sizeof(confirm_msg), 
                 "ドライブ %d には既にハードディスクが挿入されています。\n\n交換しますか？", 
                 drive_index);
        
        alert_response_t response = ask_yes_no("ハードディスク交換確認", confirm_msg);
        if (response != ALERT_RESPONSE_YES) {
            printf("ハードディスク交換がキャンセルされました\n");
            return;
        }
    }
    
    // エミュレータに読み込み
    bool success = emu->open_hard_disk(file_path, drive_index);
    
    if (success) {
        const char* filename = strrchr(file_path, '/');
        if (!filename) filename = strrchr(file_path, '\\');
        if (!filename) filename = file_path;
        else filename++;
        
        char success_msg[256];
        snprintf(success_msg, sizeof(success_msg), 
                 "ハードディスク '%s' をドライブ %d に接続しました。", 
                 filename, drive_index);
        show_info("ハードディスク接続完了", success_msg);
    } else {
        show_file_error(file_path, "ハードディスク接続", "ハードディスクファイルの形式が不正です。");
    }
}

// ステートファイル保存時の上書き確認
void save_state_with_confirmation(int slot)
{
    if (!emu) {
        show_error("保存エラー", "エミュレータが初期化されていません。");
        return;
    }
    
    const char* state_path = emu->state_file_path(slot);
    if (!state_path) {
        show_error("保存エラー", "ステートファイルパスが無効です。");
        return;
    }
    
    // ファイルが既に存在する場合は確認
    FILE* existing_file = fopen(state_path, "rb");
    if (existing_file) {
        fclose(existing_file);
        
        char confirm_msg[256];
        snprintf(confirm_msg, sizeof(confirm_msg), 
                 "スロット %d のステートファイルは既に存在します。\n\n上書きしますか？", 
                 slot);
        
        alert_response_t response = ask_yes_no("ステート上書き確認", confirm_msg);
        if (response != ALERT_RESPONSE_YES) {
            printf("ステート保存がキャンセルされました\n");
            return;
        }
    }
    
    // ステート保存実行
    bool success = emu->save_state(state_path);
    
    if (success) {
        char success_msg[128];
        snprintf(success_msg, sizeof(success_msg), 
                 "ステートをスロット %d に保存しました。", slot);
        show_info("ステート保存完了", success_msg);
    } else {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), 
                 "スロット %d へのステート保存に失敗しました。", slot);
        show_error("ステート保存エラー", error_msg);
    }
}

// ステートファイル読み込み時の確認
void load_state_with_confirmation(int slot)
{
    if (!emu) {
        show_error("読み込みエラー", "エミュレータが初期化されていません。");
        return;
    }
    
    const char* state_path = emu->state_file_path(slot);
    if (!state_path) {
        show_error("読み込みエラー", "ステートファイルパスが無効です。");
        return;
    }
    
    // ファイルの存在確認
    FILE* state_file = fopen(state_path, "rb");
    if (!state_file) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), 
                 "スロット %d にはステートファイルが保存されていません。", slot);
        show_warning("ステート読み込み", error_msg);
        return;
    }
    fclose(state_file);
    
    // 現在の状態が失われることを警告
    char confirm_msg[256];
    snprintf(confirm_msg, sizeof(confirm_msg), 
             "スロット %d のステートを読み込みます。\n\n現在の実行状態は失われますが、よろしいですか？", 
             slot);
    
    alert_response_t response = ask_yes_no("ステート読み込み確認", confirm_msg);
    if (response != ALERT_RESPONSE_YES) {
        printf("ステート読み込みがキャンセルされました\n");
        return;
    }
    
    // ステート読み込み実行
    bool success = emu->load_state(state_path);
    
    if (success) {
        char success_msg[128];
        snprintf(success_msg, sizeof(success_msg), 
                 "スロット %d からステートを読み込みました。", slot);
        show_info("ステート読み込み完了", success_msg);
    } else {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), 
                 "スロット %d からのステート読み込みに失敗しました。", slot);
        show_error("ステート読み込みエラー", error_msg);
    }
}

////////////////////////////////////////////////////////////////////////////////
// テスト・デモ関数
////////////////////////////////////////////////////////////////////////////////

void demo_alert_integration(void)
{
    printf("アラート統合デモを開始します\n");
    
    // 各種アラートのテスト
    show_info("デモ開始", "アラートシステムの統合デモを開始します。");
    
    alert_response_t response = ask_yes_no("デモ継続", "アラートタイプのテストを実行しますか？");
    if (response == ALERT_RESPONSE_YES) {
        test_all_alert_types();
    }
    
    // エミュレータ固有アラートのテスト
    show_emulator_error("テスト操作", "これはテスト用のエラーメッセージです。");
    show_disk_insert_notification("test_disk.d88", 1);
    show_disk_eject_notification(1);
    
    show_info("デモ完了", "アラート統合デモが完了しました。");
    printf("アラート統合デモを終了します\n");
}