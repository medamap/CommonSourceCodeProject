//
// ファイル選択ダイアログシステム
// Xcode/iOS/macOS対応のファイル選択機能
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ファイルタイプ定義
typedef enum {
    FILE_TYPE_FLOPPY_DISK,    // フロッピーディスク (.d88, .2d, .flp等)
    FILE_TYPE_TAPE,           // カセットテープ (.t88, .tap, .wav等)  
    FILE_TYPE_HARD_DISK,      // ハードディスク (.hdi, .hdd等)
    FILE_TYPE_CARTRIDGE,      // カートリッジ (.rom, .bin等)
    FILE_TYPE_STATE,          // ステートファイル (.sta等)
    FILE_TYPE_ALL             // 全てのファイル
} file_dialog_type_t;

// ファイル選択モード
typedef enum {
    FILE_DIALOG_MODE_OPEN,    // ファイルを開く
    FILE_DIALOG_MODE_SAVE     // ファイルを保存
} file_dialog_mode_t;

// ファイル選択結果のコールバック関数型
typedef void (*file_dialog_callback_t)(const char* file_path, int drive_index, void* user_data);

// エラーハンドリングのコールバック関数型
typedef void (*file_dialog_error_callback_t)(const char* error_message, void* user_data);

// ファイル選択ダイアログを表示
// type: ファイルタイプ
// mode: 開く/保存モード
// drive_index: ドライブインデックス（フロッピー、テープ等で使用）
// callback: 選択完了時のコールバック
// error_callback: エラー時のコールバック  
// user_data: コールバックに渡すユーザーデータ
void show_file_dialog(file_dialog_type_t type, 
                     file_dialog_mode_t mode,
                     int drive_index,
                     file_dialog_callback_t callback,
                     file_dialog_error_callback_t error_callback,
                     void* user_data);

// macOS NSOpenPanel/NSSavePanel用の実装
#ifdef __APPLE__
void show_file_dialog_macos(file_dialog_type_t type,
                           file_dialog_mode_t mode, 
                           int drive_index,
                           file_dialog_callback_t callback,
                           file_dialog_error_callback_t error_callback,
                           void* user_data);
#endif

// iOS UIDocumentPicker用の実装（将来拡張用）
#ifdef TARGET_OS_IPHONE
void show_file_dialog_ios(file_dialog_type_t type,
                         file_dialog_mode_t mode,
                         int drive_index, 
                         file_dialog_callback_t callback,
                         file_dialog_error_callback_t error_callback,
                         void* user_data);
#endif

// ファイル拡張子フィルター取得
const char** get_file_extensions(file_dialog_type_t type);

// ファイルタイプ名取得
const char* get_file_type_name(file_dialog_type_t type);

// ファイル選択ダイアログシステムの初期化
void init_file_dialog_system(void);

// ファイル選択ダイアログシステムのクリーンアップ  
void cleanup_file_dialog_system(void);

#ifdef __cplusplus
}
#endif