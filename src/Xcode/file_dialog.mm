//
// ファイル選択ダイアログシステム実装
// Xcode/iOS/macOS対応のファイル選択機能
//

#include "file_dialog.h"
#include <Foundation/Foundation.h>

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#endif

#if TARGET_OS_IOS || TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// ファイル拡張子とタイプ定義
////////////////////////////////////////////////////////////////////////////////

// フロッピーディスク用拡張子
static const char* floppy_extensions[] = {
    "d88", "d77", "1dd", "2d", "2dd", "2hd", "88d", "hdm", "dup", "2td", 
    "img", "ima", "dsk", "flp", "vfd", "xdf", "hdm", "hd4", "hd5", "hdb", 
    "dd6", "tfd", "fd0", "fd1", "fd2", "fd3", "fd4", "dcp", NULL
};

// カセットテープ用拡張子  
static const char* tape_extensions[] = {
    "t88", "tap", "wav", "cas", "cmt", "mzt", "m12", "tzx", "cdt", 
    "t77", "p6t", "p6", "mzf", "wav", NULL
};

// ハードディスク用拡張子
static const char* hdd_extensions[] = {
    "hdi", "hdd", "nhd", "hdf", "hdv", "2mg", "img", "dsk", "vhd", 
    "thd", "hdx", "ide", "scsi", NULL
};

// カートリッジ用拡張子
static const char* cart_extensions[] = {
    "rom", "bin", "car", "a26", "a78", "col", "cv", "sg", "sc", 
    "sms", "gg", "pce", "sgx", "tg16", "tgx", NULL
};

// ステートファイル用拡張子
static const char* state_extensions[] = {
    "sta", "state", "sav", "save", NULL
};

// 全ファイル
static const char* all_extensions[] = { NULL };

////////////////////////////////////////////////////////////////////////////////
// ユーティリティ関数
////////////////////////////////////////////////////////////////////////////////

const char** get_file_extensions(file_dialog_type_t type)
{
    switch (type) {
        case FILE_TYPE_FLOPPY_DISK:
            return floppy_extensions;
        case FILE_TYPE_TAPE:
            return tape_extensions;
        case FILE_TYPE_HARD_DISK:
            return hdd_extensions;
        case FILE_TYPE_CARTRIDGE:
            return cart_extensions;
        case FILE_TYPE_STATE:
            return state_extensions;
        case FILE_TYPE_ALL:
        default:
            return all_extensions;
    }
}

const char* get_file_type_name(file_dialog_type_t type)
{
    switch (type) {
        case FILE_TYPE_FLOPPY_DISK:
            return "フロッピーディスク";
        case FILE_TYPE_TAPE:
            return "カセットテープ";
        case FILE_TYPE_HARD_DISK:
            return "ハードディスク";
        case FILE_TYPE_CARTRIDGE:
            return "カートリッジ";
        case FILE_TYPE_STATE:
            return "ステートファイル";
        case FILE_TYPE_ALL:
        default:
            return "全てのファイル";
    }
}

////////////////////////////////////////////////////////////////////////////////
// macOS専用実装
////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

// ファイル拡張子配列をNSArrayに変換
NSArray<NSString*>* create_extensions_array(const char** extensions)
{
    if (extensions == NULL || extensions[0] == NULL) {
        return nil; // 全ファイルを許可
    }
    
    NSMutableArray<NSString*>* array = [NSMutableArray array];
    for (int i = 0; extensions[i] != NULL; i++) {
        [array addObject:[NSString stringWithUTF8String:extensions[i]]];
    }
    return [array copy];
}

// コールバック情報を保持する構造体
@interface FileDialogCallbackData : NSObject
@property (nonatomic, assign) file_dialog_callback_t callback;
@property (nonatomic, assign) file_dialog_error_callback_t error_callback;
@property (nonatomic, assign) void* user_data;
@property (nonatomic, assign) int drive_index;
@end

@implementation FileDialogCallbackData
@end

void show_file_dialog_macos(file_dialog_type_t type,
                           file_dialog_mode_t mode,
                           int drive_index,
                           file_dialog_callback_t callback,
                           file_dialog_error_callback_t error_callback,
                           void* user_data)
{
    @autoreleasepool {
        // コールバックデータを準備
        FileDialogCallbackData* callbackData = [[FileDialogCallbackData alloc] init];
        callbackData.callback = callback;
        callbackData.error_callback = error_callback;
        callbackData.user_data = user_data;
        callbackData.drive_index = drive_index;
        
        // 拡張子フィルターを取得
        const char** extensions = get_file_extensions(type);
        NSArray<NSString*>* allowedExtensions = create_extensions_array(extensions);
        
        if (mode == FILE_DIALOG_MODE_OPEN) {
            // ファイルを開くダイアログ
            NSOpenPanel* openPanel = [NSOpenPanel openPanel];
            
            // ダイアログの設定
            [openPanel setTitle:[NSString stringWithFormat:@"%s を開く", get_file_type_name(type)]];
            [openPanel setCanChooseFiles:YES];
            [openPanel setCanChooseDirectories:NO];
            [openPanel setAllowsMultipleSelection:NO];
            [openPanel setCanCreateDirectories:NO];
            
            // ファイル拡張子フィルター設定
            if (allowedExtensions != nil) {
                [openPanel setAllowedFileTypes:allowedExtensions];
            }
            
            // ダイアログを表示
            [openPanel beginWithCompletionHandler:^(NSModalResponse result) {
                if (result == NSModalResponseOK) {
                    NSURL* selectedURL = [[openPanel URLs] firstObject];
                    if (selectedURL) {
                        NSString* filePath = [selectedURL path];
                        if (callbackData.callback) {
                            callbackData.callback([filePath UTF8String], 
                                                 callbackData.drive_index, 
                                                 callbackData.user_data);
                        }
                    } else {
                        if (callbackData.error_callback) {
                            callbackData.error_callback("ファイルが選択されませんでした", 
                                                       callbackData.user_data);
                        }
                    }
                } else {
                    // ユーザーがキャンセル
                    if (callbackData.callback) {
                        callbackData.callback(NULL, callbackData.drive_index, callbackData.user_data);
                    }
                }
            }];
            
        } else {
            // ファイルを保存するダイアログ
            NSSavePanel* savePanel = [NSSavePanel savePanel];
            
            // ダイアログの設定
            [savePanel setTitle:[NSString stringWithFormat:@"%s を保存", get_file_type_name(type)]];
            [savePanel setCanCreateDirectories:YES];
            
            // ファイル拡張子フィルター設定
            if (allowedExtensions != nil && [allowedExtensions count] > 0) {
                [savePanel setAllowedFileTypes:allowedExtensions];
                [savePanel setAllowsOtherFileTypes:NO];
            }
            
            // ダイアログを表示
            [savePanel beginWithCompletionHandler:^(NSModalResponse result) {
                if (result == NSModalResponseOK) {
                    NSURL* selectedURL = [savePanel URL];
                    if (selectedURL) {
                        NSString* filePath = [selectedURL path];
                        if (callbackData.callback) {
                            callbackData.callback([filePath UTF8String], 
                                                 callbackData.drive_index, 
                                                 callbackData.user_data);
                        }
                    } else {
                        if (callbackData.error_callback) {
                            callbackData.error_callback("保存先が選択されませんでした", 
                                                       callbackData.user_data);
                        }
                    }
                } else {
                    // ユーザーがキャンセル
                    if (callbackData.callback) {
                        callbackData.callback(NULL, callbackData.drive_index, callbackData.user_data);
                    }
                }
            }];
        }
    }
}

#endif // __APPLE__

////////////////////////////////////////////////////////////////////////////////
// iOS専用実装（将来拡張用）
////////////////////////////////////////////////////////////////////////////////

#if TARGET_OS_IOS || TARGET_OS_IPHONE

void show_file_dialog_ios(file_dialog_type_t type,
                         file_dialog_mode_t mode,
                         int drive_index,
                         file_dialog_callback_t callback,
                         file_dialog_error_callback_t error_callback,
                         void* user_data)
{
    @autoreleasepool {
        // UIDocumentPickerを使用した実装（将来実装）
        if (error_callback) {
            error_callback("iOS版のファイル選択は未実装です", user_data);
        }
    }
}

#endif // TARGET_OS_IOS || TARGET_OS_IPHONE

////////////////////////////////////////////////////////////////////////////////
// クロスプラットフォーム インターフェース
////////////////////////////////////////////////////////////////////////////////

void show_file_dialog(file_dialog_type_t type,
                     file_dialog_mode_t mode,
                     int drive_index,
                     file_dialog_callback_t callback,
                     file_dialog_error_callback_t error_callback,
                     void* user_data)
{
#ifdef __APPLE__
    #if TARGET_OS_IOS || TARGET_OS_IPHONE
        show_file_dialog_ios(type, mode, drive_index, callback, error_callback, user_data);
    #else
        show_file_dialog_macos(type, mode, drive_index, callback, error_callback, user_data);
    #endif
#else
    if (error_callback) {
        error_callback("このプラットフォームではファイル選択ダイアログがサポートされていません", user_data);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
// システム初期化・クリーンアップ
////////////////////////////////////////////////////////////////////////////////

void init_file_dialog_system(void)
{
    // 現在は特別な初期化処理は不要
    printf("ファイル選択ダイアログシステムを初期化しました\n");
}

void cleanup_file_dialog_system(void)
{
    // 現在は特別なクリーンアップ処理は不要
    printf("ファイル選択ダイアログシステムをクリーンアップしました\n");
}