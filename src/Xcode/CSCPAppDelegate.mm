/*
    App Delegate for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [macOS application delegate]
*/

#import "CSCPAppDelegate.h"
#import "CSCPViewController.h"
#include "xcode_mainloop.h"
#include "../emu.h"
#include "../config.h"
#include "../menu/BaseMenu.h"
#include "../menu/menu.h"
#include "../res/resource.h"

@interface CSCPAppDelegate ()
// プライベートプロパティがあればここに追加
@end

@implementation CSCPAppDelegate

- (instancetype)initWithArgc:(int)argc argv:(char**)argv {
    self = [super init];
    if (self) {
        self.argc = argc;
        self.argv = argv;
    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // エミュレータインスタンスを作成
    EMU* emu = new EMU();
    self.emuInstance = emu;
    
    // コマンドライン引数を解析してメディアをマウント
    emulator_args_t args;
    parse_command_line_args(self.argc, self.argv, &args);
    
    if (args.show_help) {
        show_help();
        [NSApp terminate:self];
        return;
    }
    
    // 設定のロード
    load_ini_file(emu, args.machine_name);
    
    // メディアマウントを遅延実行するため、引数を保存
    emulator_args_t* saved_args = (emulator_args_t*)malloc(sizeof(emulator_args_t));
    memcpy(saved_args, &args, sizeof(emulator_args_t));
    self.delayedArgs = saved_args;
    
    // ウィンドウを作成
    NSRect frame = NSMakeRect(0, 0, 640, 400);
    NSWindowStyleMask style = NSWindowStyleMaskTitled |
                             NSWindowStyleMaskClosable |
                             NSWindowStyleMaskMiniaturizable |
                             NSWindowStyleMaskResizable;
    
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    
    // ウィンドウタイトル設定
    NSString* title = [NSString stringWithFormat:@"CSCP - %s", args.machine_name];
    [self.window setTitle:title];
    
    // ビューコントローラーを作成して設定
    self.viewController = [[CSCPViewController alloc] initWithEmuInstance:emu];
    [self.window setContentViewController:self.viewController];
    
    // メニューバーを作成
    [self createMenuBar:emu];
    
    // ウィンドウのキーイベント設定
    [self.window setAcceptsMouseMovedEvents:YES];
    self.window.delegate = self.viewController;
    
    // ウィンドウを中央に配置して表示
    [self.window center];
    [self.window makeKeyAndOrderFront:nil];
    
    // エミュレータが完全に初期化された後にメディアをマウント（少し遅延）
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), 
                   dispatch_get_main_queue(), ^{
        [self performDelayedMediaMount];
    });
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // エミュレータを停止してクリーンアップ
    if (self.viewController) {
        [self.viewController stopEmulation];
    }
    
    if (self.emuInstance) {
        EMU* emu = (EMU*)self.emuInstance;
        delete emu;
        self.emuInstance = nullptr;
    }
    
    // メニューシステムのクリーンアップ
    if (self.menuSystem) {
        Menu* menuSystem = (Menu*)self.menuSystem;
        delete menuSystem;
        self.menuSystem = nullptr;
    }
    
    // 遅延引数のクリーンアップ
    if (self.delayedArgs) {
        free(self.delayedArgs);
        self.delayedArgs = nullptr;
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

// メニューバー作成メソッド
- (void)createMenuBar:(EMU*)emu {
    // メインメニューバーを作成
    NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
    
    // アプリケーションメニュー
    NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
    NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"CSCP"];
    
    // About メニュー
    NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:@"CSCPについて"
                                                       action:@selector(showAbout:)
                                                keyEquivalent:@""];
    [aboutItem setTarget:self];
    [appMenu addItem:aboutItem];
    
    [appMenu addItem:[NSMenuItem separatorItem]];
    
    // Quit メニュー
    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"CSCPを終了"
                                                      action:@selector(terminate:)
                                               keyEquivalent:@"q"];
    [quitItem setTarget:NSApp];
    [appMenu addItem:quitItem];
    
    [appMenuItem setSubmenu:appMenu];
    [mainMenu addItem:appMenuItem];
    
    // BaseMenuシステムからメニューを構築
    Menu* menuSystem = new Menu();
    
    // まずRootノードを見つける
    std::vector<MenuNode> allRootNodes = menuSystem->getRootNodes();
    int actualRootId = -1;
    for (const auto& node : allRootNodes) {
        if (node.getCaption() == "Root") {
            actualRootId = node.getNodeId();
            break;
        }
    }
    
    if (actualRootId != -1) {
        // Rootの子要素を取得してメニューを作成
        std::vector<MenuNode> rootChildren = menuSystem->getNodes(actualRootId);
        
        for (const auto& node : rootChildren) {
            if (node.getItemType() == Category) {
                NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:node.getCaption().c_str()]
                                                                  action:nil
                                                           keyEquivalent:@""];
                NSMenu *subMenu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:node.getCaption().c_str()]];
                
                // サブメニューアイテムを追加
                [self addMenuItems:subMenu fromMenu:menuSystem forParentId:node.getNodeId()];
                
                [menuItem setSubmenu:subMenu];
                [mainMenu addItem:menuItem];
            }
        }
    }
    
    // ヘルプメニューを追加
    NSMenuItem *helpMenuItem = [[NSMenuItem alloc] init];
    NSMenu *helpMenu = [[NSMenu alloc] initWithTitle:@"ヘルプ"];
    
    NSMenuItem *helpItem = [[NSMenuItem alloc] initWithTitle:@"ヘルプ"
                                                      action:@selector(showHelp:)
                                               keyEquivalent:@"?"];
    [helpItem setTarget:self];
    [helpMenu addItem:helpItem];
    
    [helpMenuItem setSubmenu:helpMenu];
    [mainMenu addItem:helpMenuItem];
    
    // メニューバーに設定
    [NSApp setMainMenu:mainMenu];
    
    // EMUインスタンスを保存（メニューアクションで使用）
    self.emuInstance = emu;
    
    // メニューシステムを保存（後で削除）
    self.menuSystem = menuSystem;
}

// 再帰的にメニューアイテムを追加するヘルパーメソッド
- (void)addMenuItems:(NSMenu*)menu fromMenu:(Menu*)menuSystem forParentId:(int)parentId {
    std::vector<MenuNode> children = menuSystem->getNodes(parentId);
    
    for (const auto& node : children) {
        if (node.getItemType() == Category) {
            // サブメニューとして追加
            NSMenuItem *categoryItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:node.getCaption().c_str()]
                                                                  action:nil
                                                           keyEquivalent:@""];
            NSMenu *subMenu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:node.getCaption().c_str()]];
            
            [self addMenuItems:subMenu fromMenu:menuSystem forParentId:node.getNodeId()];
            
            [categoryItem setSubmenu:subMenu];
            [menu addItem:categoryItem];
            
        } else if (node.getItemType() == Property) {
            // プロパティアイテムとして追加
            NSMenuItem *propertyItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:node.getCaption().c_str()]
                                                                  action:@selector(handleMenuAction:)
                                                           keyEquivalent:@""];
            [propertyItem setTarget:self];
            [propertyItem setTag:node.getReturnValue()]; // returnValueをtagに保存
            [menu addItem:propertyItem];
        }
    }
}

// メニューアクションハンドラー（BaseMenuから生成されたメニューアイテム用）
- (void)handleMenuAction:(id)sender {
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    int actionId = (int)[menuItem tag];
    
    if (self.emuInstance) {
        EMU* emu = (EMU*)self.emuInstance;
        
        // actionIdに基づいてエミュレータの機能を実行
        switch (actionId) {
            case ID_RESET:
                printf("リセットが実行されました\n");
                emu->reset();
                break;
                
            case ID_SPECIAL_RESET:
                printf("NMIが実行されました\n");
#ifdef USE_SPECIAL_RESET
                emu->special_reset();
#endif
                break;
                
            case ID_CPU_POWER0:
            case ID_CPU_POWER1:
            case ID_CPU_POWER2:
            case ID_CPU_POWER3:
            case ID_CPU_POWER4:
                printf("CPU倍速設定が変更されました: %d\n", actionId - ID_CPU_POWER0 + 1);
                // CPU倍速設定の実装
                break;
                
            case ID_FULL_SPEED:
                printf("フルスピードモードが切り替えられました\n");
                // TODO: フルスピードモードの実装
                break;
                
            case ID_DRIVE_VM_IN_OPECODE:
                printf("M1/R/W サイクルでVMドライブが切り替えられました\n");
                // TODO: VMドライブ制御の実装
                break;
                
            case ID_AUTOKEY_START:
                printf("オートキー（ペースト）が開始されました\n");
                // TODO: オートキー開始の実装
                break;
                
            case ID_AUTOKEY_STOP:
                printf("オートキーが停止されました\n");
                // TODO: オートキー停止の実装
                break;
                
            case ID_ROMAJI_TO_KANA:
                printf("ローマ字→かな変換が切り替えられました\n");
                // TODO: ローマ字→かな変換の実装
                break;
                
            // State Save/Load actions
            case ID_SAVE_STATE0:
            case ID_SAVE_STATE1:
            case ID_SAVE_STATE2:
            case ID_SAVE_STATE3:
            case ID_SAVE_STATE4:
            case ID_SAVE_STATE5:
            case ID_SAVE_STATE6:
            case ID_SAVE_STATE7:
            case ID_SAVE_STATE8:
            case ID_SAVE_STATE9:
                printf("ステートセーブ: State %d\n", actionId - ID_SAVE_STATE0);
                // TODO: ステートセーブの実装
                break;
                
            case ID_LOAD_STATE0:
            case ID_LOAD_STATE1:
            case ID_LOAD_STATE2:
            case ID_LOAD_STATE3:
            case ID_LOAD_STATE4:
            case ID_LOAD_STATE5:
            case ID_LOAD_STATE6:
            case ID_LOAD_STATE7:
            case ID_LOAD_STATE8:
            case ID_LOAD_STATE9:
                printf("ステートロード: State %d\n", actionId - ID_LOAD_STATE0);
                // TODO: ステートロードの実装
                break;
                
            // Debugger actions
            case ID_OPEN_DEBUGGER0:
                printf("メインCPUデバッガーが開かれました\n");
                // TODO: メインCPUデバッガーの実装
                break;
                
            case ID_OPEN_DEBUGGER1:
                printf("サブCPUデバッガーが開かれました\n");
                // TODO: サブCPUデバッガーの実装
                break;
                
            case ID_OPEN_DEBUGGER2:
                printf("キーボードCPUデバッガーが開かれました\n");
                // TODO: キーボードCPUデバッガーの実装
                break;
                
            case ID_CLOSE_DEBUGGER:
                printf("デバッガーが閉じられました\n");
                // TODO: デバッガー終了の実装
                break;
                
            case ID_OPEN_FD1:
            case ID_OPEN_FD2:
            case ID_OPEN_FD3:
            case ID_OPEN_FD4:
                printf("フロッピーディスク挿入: FD%d\n", actionId - ID_OPEN_FD1);
                [self openFloppyDisk:sender];
                break;
                
            case ID_CLOSE_FD1:
            case ID_CLOSE_FD2:
            case ID_CLOSE_FD3:
            case ID_CLOSE_FD4:
                printf("フロッピーディスク排出: FD%d\n", actionId - ID_CLOSE_FD1);
                emu->close_floppy_disk(actionId - ID_CLOSE_FD1);
                break;
                
            // Blank Disk creation
            case ID_OPEN_BLANK_2D_FD1:
            case ID_OPEN_BLANK_2D_FD2:
            case ID_OPEN_BLANK_2D_FD3:
            case ID_OPEN_BLANK_2D_FD4:
                printf("2Dブランクディスク作成: FD%d\n", actionId - ID_OPEN_BLANK_2D_FD1);
                // TODO: 2Dブランクディスク作成の実装
                break;
                
            case ID_OPEN_BLANK_2DD_FD1:
            case ID_OPEN_BLANK_2DD_FD2:
            case ID_OPEN_BLANK_2DD_FD3:
            case ID_OPEN_BLANK_2DD_FD4:
                printf("2DDブランクディスク作成: FD%d\n", actionId - ID_OPEN_BLANK_2DD_FD1);
                // TODO: 2DDブランクディスク作成の実装
                break;
                
            case ID_OPEN_BLANK_2HD_FD1:
            case ID_OPEN_BLANK_2HD_FD2:
            case ID_OPEN_BLANK_2HD_FD3:
            case ID_OPEN_BLANK_2HD_FD4:
                printf("2HDブランクディスク作成: FD%d\n", actionId - ID_OPEN_BLANK_2HD_FD1);
                // TODO: 2HDブランクディスク作成の実装
                break;
                
            // Write Protection
            case ID_WRITE_PROTECT_FD1:
            case ID_WRITE_PROTECT_FD2:
            case ID_WRITE_PROTECT_FD3:
            case ID_WRITE_PROTECT_FD4:
                printf("ライトプロテクト切り替え: FD%d\n", actionId - ID_WRITE_PROTECT_FD1);
                // TODO: ライトプロテクト切り替えの実装
                break;
                
            // Correct Timing
            case ID_CORRECT_TIMING_FD1:
            case ID_CORRECT_TIMING_FD2:
            case ID_CORRECT_TIMING_FD3:
            case ID_CORRECT_TIMING_FD4:
                printf("正確なタイミング切り替え: FD%d\n", actionId - ID_CORRECT_TIMING_FD1);
                // TODO: 正確なタイミング切り替えの実装
                break;
                
            // Ignore CRC Errors
            case ID_IGNORE_CRC_FD1:
            case ID_IGNORE_CRC_FD2:
            case ID_IGNORE_CRC_FD3:
            case ID_IGNORE_CRC_FD4:
                printf("CRCエラー無視切り替え: FD%d\n", actionId - ID_IGNORE_CRC_FD1);
                // TODO: CRCエラー無視切り替えの実装
                break;
                
            // Recent files
            case ID_RECENT_FD1:
            case ID_RECENT_FD2:
            case ID_RECENT_FD3:
            case ID_RECENT_FD4:
                printf("最近使用したファイル: FD%d\n", actionId - ID_RECENT_FD1);
                // TODO: 最近使用したファイルの実装
                break;
                
#ifdef USE_HARD_DISK
            case ID_OPEN_HD1:
            case ID_OPEN_HD2:
            case ID_OPEN_HD3:
            case ID_OPEN_HD4:
                printf("ハードディスクマウント: HD%d\n", actionId - ID_OPEN_HD1);
                [self openHardDisk:sender];
                break;
                
            case ID_CLOSE_HD1:
            case ID_CLOSE_HD2:
            case ID_CLOSE_HD3:
            case ID_CLOSE_HD4:
                printf("ハードディスクアンマウント: HD%d\n", actionId - ID_CLOSE_HD1);
                emu->close_hard_disk(actionId - ID_CLOSE_HD1);
                break;
                
            // Blank HDD creation
            case ID_OPEN_BLANK_20MB_HD1:
            case ID_OPEN_BLANK_20MB_HD2:
            case ID_OPEN_BLANK_20MB_HD3:
            case ID_OPEN_BLANK_20MB_HD4:
                printf("20MBブランクハードディスク作成: HD%d\n", actionId - ID_OPEN_BLANK_20MB_HD1);
                // TODO: 20MBブランクハードディスク作成の実装
                break;
                
            // Recent HDD files
            case ID_RECENT_HD1:
            case ID_RECENT_HD2:
            case ID_RECENT_HD3:
            case ID_RECENT_HD4:
                printf("最近使用したハードディスク: HD%d\n", actionId - ID_RECENT_HD1);
                // TODO: 最近使用したハードディスクの実装
                break;
#endif
                
            // CMT Recent
            case ID_RECENT_TAPE1:
                printf("最近使用したカセットテープ\n");
                // TODO: 最近使用したカセットテープの実装
                break;
                
            case ID_EXIT:
                printf("終了が選択されました\n");
                [NSApp terminate:self];
                break;
                
            // Boot Device actions
            case ID_VM_DRIVE_TYPE0:
            case ID_VM_DRIVE_TYPE1:
            case ID_VM_DRIVE_TYPE2:
            case ID_VM_DRIVE_TYPE6:
            case ID_VM_DRIVE_TYPE7:
                {
                    int driveType = 0;
                    switch(actionId) {
                        case ID_VM_DRIVE_TYPE0: driveType = 0; break; // 5/3-inch 2D
                        case ID_VM_DRIVE_TYPE1: driveType = 1; break; // 5/3-inch 2DD
                        case ID_VM_DRIVE_TYPE2: driveType = 2; break; // 5/3-inch 2HD
                        case ID_VM_DRIVE_TYPE6: driveType = 6; break; // 8-inch 1S
                        case ID_VM_DRIVE_TYPE7: driveType = 7; break; // HARD DISK
                    }
                    printf("ブートデバイス設定が変更されました: %d\n", driveType);
#ifdef USE_DRIVE_TYPE
                    config.drive_type = driveType;
#endif
                    emu->update_config();
                    
                    // 設定を保存
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // Keyboard actions
            case ID_VM_KEYBOARD_TYPE0:
            case ID_VM_KEYBOARD_TYPE1:
                {
                    int keyboardType = (actionId == ID_VM_KEYBOARD_TYPE0) ? 0 : 1;
                    printf("キーボードタイプが変更されました: Mode %c\n", keyboardType ? 'B' : 'A');
#ifdef USE_KEYBOARD_TYPE
                    config.keyboard_type = keyboardType;
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // Sound actions
            case ID_VM_SOUND_TYPE0:
            case ID_VM_SOUND_TYPE1:
            case ID_VM_SOUND_TYPE2:
                {
                    int soundType = actionId - ID_VM_SOUND_TYPE0;
                    printf("サウンド設定が変更されました: %d\n", soundType);
#ifdef USE_SOUND_TYPE
                    config.sound_type = soundType;
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            case ID_VM_SOUND_NOISE_FDD:
                {
                    config.sound_noise_fdd = !config.sound_noise_fdd;
                    printf("FDDノイズ設定が切り替えられました: %s\n", config.sound_noise_fdd ? "ON" : "OFF");
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            case ID_VM_SOUND_NOISE_CMT:
                {
                    config.sound_noise_cmt = !config.sound_noise_cmt;
                    printf("CMTノイズ設定が切り替えられました: %s\n", config.sound_noise_cmt ? "ON" : "OFF");
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            case ID_VM_SOUND_TAPE_SIGNAL:
                {
                    config.sound_tape_signal = !config.sound_tape_signal;
                    printf("CMTシグナル設定が切り替えられました: %s\n", config.sound_tape_signal ? "ON" : "OFF");
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            case ID_VM_SOUND_TAPE_VOICE:
                {
                    config.sound_tape_voice = !config.sound_tape_voice;
                    printf("CMTボイス設定が切り替えられました: %s\n", config.sound_tape_voice ? "ON" : "OFF");
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // Display actions
            case ID_VM_MONITOR_TYPE0:
            case ID_VM_MONITOR_TYPE1:
                {
                    int monitorType = (actionId == ID_VM_MONITOR_TYPE0) ? 0 : 1;
                    printf("モニタータイプが変更されました: %s\n", monitorType ? "Standard" : "High Resolution");
#ifdef USE_MONITOR_TYPE
                    config.monitor_type = monitorType;
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            case ID_VM_MONITOR_SCANLINE:
                {
#ifdef USE_SCANLINE
                    config.scan_line = !config.scan_line;
                    printf("スキャンライン設定が切り替えられました: %s\n", config.scan_line ? "ON" : "OFF");
#else
                    printf("スキャンライン設定はこの機種ではサポートされていません\n");
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // Printer actions
            case ID_VM_PRINTER_TYPE0:
            case ID_VM_PRINTER_TYPE1:
            case ID_VM_PRINTER_TYPE2:
            case ID_VM_PRINTER_TYPE3:
            case ID_VM_PRINTER_TYPE4:
                {
                    int printerType = actionId - ID_VM_PRINTER_TYPE0;
                    const char* printerNames[] = {"File", "MZ-1P17", "PC-PR201", "JAST SOUND", "None"};
                    printf("プリンタータイプが変更されました: %s\n", printerNames[printerType]);
#ifdef USE_PRINTER_TYPE
                    config.printer_type = printerType;
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // Serial actions
            case ID_VM_SERIAL_TYPE0:
            case ID_VM_SERIAL_TYPE1:
            case ID_VM_SERIAL_TYPE2:
            case ID_VM_SERIAL_TYPE3:
                {
                    int serialType = actionId - ID_VM_SERIAL_TYPE0;
                    const char* serialNames[] = {"Physical Comm Port", "Named Pipe", "MIDI Device", "None"};
                    printf("シリアル設定が変更されました: %s\n", serialNames[serialType]);
#ifdef USE_SERIAL_TYPE
                    config.serial_type = serialType;
#endif
                    emu->update_config();
                    
                    emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
                    if (args) {
                        save_ini_file(emu, args->machine_name);
                    }
                }
                break;
                
            // CMT actions
            case ID_PLAY_TAPE1:
                printf("CMT再生が開始されました\n");
                // TODO: CMT再生の実装
                break;
                
            case ID_REC_TAPE1:
                printf("CMT録音が開始されました\n");
                // TODO: CMT録音の実装
                break;
                
            case ID_CLOSE_TAPE1:
                printf("CMTが排出されました\n");
                // TODO: CMT排出の実装
                break;
                
            case ID_PLAY_BUTTON1:
                printf("CMT再生ボタンが押されました\n");
                // TODO: CMT再生ボタンの実装
                break;
                
            case ID_STOP_BUTTON1:
                printf("CMT停止ボタンが押されました\n");
                // TODO: CMT停止ボタンの実装
                break;
                
            case ID_FAST_FORWARD1:
                printf("CMT早送りが開始されました\n");
                // TODO: CMT早送りの実装
                break;
                
            case ID_FAST_REWIND1:
                printf("CMT巻き戻しが開始されました\n");
                // TODO: CMT巻き戻しの実装
                break;
                
            case ID_APSS_FORWARD1:
                printf("CMT APSS前進が開始されました\n");
                // TODO: CMT APSS前進の実装
                break;
                
            case ID_APSS_REWIND1:
                printf("CMT APSS巻き戻しが開始されました\n");
                // TODO: CMT APSS巻き戻しの実装
                break;
                
            case ID_USE_WAVE_SHAPER1:
                printf("CMTウェーブシェイパー設定が切り替えられました\n");
                // TODO: CMTウェーブシェイパー設定の実装
                break;
                
            // Host Recording actions
            case ID_HOST_REC_MOVIE_60FPS:
            case ID_HOST_REC_MOVIE_30FPS:
            case ID_HOST_REC_MOVIE_15FPS:
                printf("動画録画が開始されました: %dfps\n", (actionId == ID_HOST_REC_MOVIE_60FPS) ? 60 : (actionId == ID_HOST_REC_MOVIE_30FPS) ? 30 : 15);
                // TODO: 動画録画の実装
                break;
                
            case ID_HOST_REC_SOUND:
                printf("音声録音が開始されました\n");
                // TODO: 音声録音の実装
                break;
                
            case ID_HOST_REC_STOP:
                printf("録画/録音が停止されました\n");
                // TODO: 録画/録音停止の実装
                break;
                
            case ID_HOST_CAPTURE_SCREEN:
                printf("スクリーンキャプチャが実行されました\n");
                // TODO: スクリーンキャプチャの実装
                break;
                
            // Mouse sensitivity actions
            case ID_MOUSE_SENSITIVE_0:
            case ID_MOUSE_SENSITIVE_1:
            case ID_MOUSE_SENSITIVE_2:
            case ID_MOUSE_SENSITIVE_3:
            case ID_MOUSE_SENSITIVE_4:
            case ID_MOUSE_SENSITIVE_5:
            case ID_MOUSE_SENSITIVE_6:
            case ID_MOUSE_SENSITIVE_7:
            case ID_MOUSE_SENSITIVE_8:
            case ID_MOUSE_SENSITIVE_9:
            case ID_MOUSE_SENSITIVE_10:
                printf("マウス感度が変更されました: %d\n", actionId - ID_MOUSE_SENSITIVE_0);
                // TODO: マウス感度設定の実装
                break;
                
            // Screen rotation actions
            case ID_SCREEN_ROTATE_0:
            case ID_SCREEN_ROTATE_90:
            case ID_SCREEN_ROTATE_180:
            case ID_SCREEN_ROTATE_270:
                printf("画面回転が変更されました: %d度\n", (actionId - ID_SCREEN_ROTATE_0) * 90);
                // TODO: 画面回転の実装
                break;
                
            // Icon Size settings
            case ID_SCREEN_HS_ICON_SIZE_12:
            case ID_SCREEN_HS_ICON_SIZE_19:
            case ID_SCREEN_HS_ICON_SIZE_26:
            case ID_SCREEN_HS_ICON_SIZE_33:
            case ID_SCREEN_HS_ICON_SIZE_40:
            case ID_SCREEN_HS_ICON_SIZE_47:
            case ID_SCREEN_HS_ICON_SIZE_54:
            case ID_SCREEN_HS_ICON_SIZE_61:
                printf("水平システムアイコンサイズが変更されました\n");
                // TODO: 水平システムアイコンサイズ設定の実装
                break;
                
            case ID_SCREEN_HF_ICON_SIZE_12:
            case ID_SCREEN_HF_ICON_SIZE_19:
            case ID_SCREEN_HF_ICON_SIZE_26:
            case ID_SCREEN_HF_ICON_SIZE_33:
            case ID_SCREEN_HF_ICON_SIZE_40:
            case ID_SCREEN_HF_ICON_SIZE_47:
            case ID_SCREEN_HF_ICON_SIZE_54:
            case ID_SCREEN_HF_ICON_SIZE_61:
                printf("水平ファイルアイコンサイズが変更されました\n");
                // TODO: 水平ファイルアイコンサイズ設定の実装
                break;
                
            case ID_SCREEN_VS_ICON_SIZE_12:
            case ID_SCREEN_VS_ICON_SIZE_19:
            case ID_SCREEN_VS_ICON_SIZE_26:
            case ID_SCREEN_VS_ICON_SIZE_33:
            case ID_SCREEN_VS_ICON_SIZE_40:
            case ID_SCREEN_VS_ICON_SIZE_47:
            case ID_SCREEN_VS_ICON_SIZE_54:
            case ID_SCREEN_VS_ICON_SIZE_61:
                printf("垂直システムアイコンサイズが変更されました\n");
                // TODO: 垂直システムアイコンサイズ設定の実装
                break;
                
            case ID_SCREEN_VF_ICON_SIZE_12:
            case ID_SCREEN_VF_ICON_SIZE_19:
            case ID_SCREEN_VF_ICON_SIZE_26:
            case ID_SCREEN_VF_ICON_SIZE_33:
            case ID_SCREEN_VF_ICON_SIZE_40:
            case ID_SCREEN_VF_ICON_SIZE_47:
            case ID_SCREEN_VF_ICON_SIZE_54:
            case ID_SCREEN_VF_ICON_SIZE_61:
                printf("垂直ファイルアイコンサイズが変更されました\n");
                // TODO: 垂直ファイルアイコンサイズ設定の実装
                break;
                
            // Screen Margin settings
            case ID_SCREEN_TOP_MARGIN_0:
            case ID_SCREEN_TOP_MARGIN_30:
            case ID_SCREEN_TOP_MARGIN_60:
            case ID_SCREEN_TOP_MARGIN_90:
            case ID_SCREEN_TOP_MARGIN_120:
            case ID_SCREEN_TOP_MARGIN_150:
            case ID_SCREEN_TOP_MARGIN_180:
            case ID_SCREEN_TOP_MARGIN_210:
            case ID_SCREEN_TOP_MARGIN_240:
            case ID_SCREEN_TOP_MARGIN_270:
                printf("画面上部マージンが変更されました\n");
                // TODO: 画面上部マージン設定の実装
                break;
                
            case ID_SCREEN_BOTTOM_MARGIN_0:
            case ID_SCREEN_BOTTOM_MARGIN_30:
            case ID_SCREEN_BOTTOM_MARGIN_60:
            case ID_SCREEN_BOTTOM_MARGIN_90:
            case ID_SCREEN_BOTTOM_MARGIN_120:
            case ID_SCREEN_BOTTOM_MARGIN_150:
            case ID_SCREEN_BOTTOM_MARGIN_180:
            case ID_SCREEN_BOTTOM_MARGIN_210:
            case ID_SCREEN_BOTTOM_MARGIN_240:
            case ID_SCREEN_BOTTOM_MARGIN_270:
                printf("画面下部マージンが変更されました\n");
                // TODO: 画面下部マージン設定の実装
                break;
                
            // Filter actions
            case ID_FILTER_GREEN:
            case ID_FILTER_RGB:
            case ID_FILTER_BLUR:
            case ID_FILTER_NONE:
            case ID_FILTER_DOT:
            case ID_FILTER_SUPERIMPOSE:
                printf("フィルター設定が変更されました: %d\n", actionId - ID_FILTER_GREEN);
                // TODO: フィルター設定の実装
                break;
                
            // Sound settings actions
            case ID_SOUND_ON:
                printf("サウンドのオン/オフが切り替えられました\n");
                // TODO: サウンドオン/オフの実装
                break;
                
            case ID_SOUND_FREQ0:
            case ID_SOUND_FREQ1:
            case ID_SOUND_FREQ2:
            case ID_SOUND_FREQ3:
            case ID_SOUND_FREQ4:
            case ID_SOUND_FREQ5:
            case ID_SOUND_FREQ6:
            case ID_SOUND_FREQ7:
                printf("サウンド周波数が変更されました: %d\n", actionId - ID_SOUND_FREQ0);
                // TODO: サウンド周波数設定の実装
                break;
                
            case ID_SOUND_LATE0:
            case ID_SOUND_LATE1:
            case ID_SOUND_LATE2:
            case ID_SOUND_LATE3:
            case ID_SOUND_LATE4:
                printf("サウンド遅延が変更されました: %d\n", actionId - ID_SOUND_LATE0);
                // TODO: サウンド遅延設定の実装
                break;
                
            case ID_SOUND_STRICT_RENDER:
                printf("リアルタイムミックスが有効になりました\n");
                // TODO: リアルタイムミックスの実装
                break;
                
            case ID_SOUND_LIGHT_RENDER:
                printf("軽量ミックスが有効になりました\n");
                // TODO: 軽量ミックスの実装
                break;
                
            case ID_SOUND_VOLUME:
                printf("音量設定が変更されました\n");
                // TODO: 音量設定の実装
                break;
                
            // Input actions
            case ID_INPUT_JOYSTICK0:
            case ID_INPUT_JOYSTICK1:
                printf("ジョイスティック設定が変更されました: #%d\n", (actionId - ID_INPUT_JOYSTICK0) + 1);
                // TODO: ジョイスティック設定の実装
                break;
                
            case ID_INPUT_JOYTOKEY:
                printf("ジョイスティック→キーボード変換が切り替えられました\n");
                // TODO: ジョイスティック→キーボード変換の実装
                break;
                
            // Host system settings
            case ID_HOST_USE_D2D1:
                printf("Direct2D1使用設定が切り替えられました\n");
                // TODO: Direct2D1設定の実装（macOSでは無効）
                break;
                
            case ID_HOST_USE_D3D9:
                printf("Direct3D9使用設定が切り替えられました\n");
                // TODO: Direct3D9設定の実装（macOSでは無効）
                break;
                
            case ID_HOST_WAIT_VSYNC:
                printf("Vsync待機設定が切り替えられました\n");
                // TODO: Vsync待機設定の実装
                break;
                
            case ID_HOST_USE_DINPUT:
                printf("DirectInput使用設定が切り替えられました\n");
                // TODO: DirectInput設定の実装（macOSでは無効）
                break;
                
            case ID_HOST_DISABLE_DWM:
                printf("Windows 8 DWM無効設定が切り替えられました\n");
                // TODO: DWM無効設定の実装（macOSでは無効）
                break;
                
            case ID_HOST_SHOW_STATUS_BAR:
                printf("ステータスバー表示設定が切り替えられました\n");
                // TODO: ステータスバー表示設定の実装
                break;
                
            default:
                printf("未対応のメニューアクション: %d\n", actionId);
                break;
        }
    }
}

// メニューアクションメソッド
- (void)showAbout:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"CSCP エミュレータ"];
    [alert setInformativeText:@"CommonSourceCodeProject\nmacOS/iOS 版\n\nAuthor: Medamap & Claude"];
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
}

- (void)openFloppyDisk:(id)sender {
    // ファイルダイアログを表示してフロッピーディスクイメージを選択
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setTitle:@"フロッピーディスクイメージを選択"];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setAllowedFileTypes:@[@"d88", @"d77", @"2d", @"img", @"dsk"]];
    
    [openPanel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result == NSModalResponseOK) {
            NSURL *selectedURL = [[openPanel URLs] firstObject];
            if (selectedURL && self.emuInstance) {
                EMU* emu = (EMU*)self.emuInstance;
                const char* path = [[selectedURL path] UTF8String];
                printf("フロッピーディスクをマウント: %s\n", path);
                
                // EMUクラスのopen_floppy_diskメソッドを呼び出し
                _TCHAR tchar_path[512];
                for(int i = 0; i < 512 && path[i]; i++) {
                    tchar_path[i] = path[i];
                }
                tchar_path[strlen(path)] = 0;
                
                try {
                    emu->open_floppy_disk(0, tchar_path, 0);
                    printf("フロッピーディスクマウント成功\n");
                } catch (...) {
                    printf("フロッピーディスクマウント失敗\n");
                    NSAlert *alert = [[NSAlert alloc] init];
                    [alert setMessageText:@"エラー"];
                    [alert setInformativeText:@"フロッピーディスクのマウントに失敗しました"];
                    [alert addButtonWithTitle:@"OK"];
                    [alert runModal];
                }
            }
        }
    }];
}

- (void)openHardDisk:(id)sender {
    printf("ハードディスクメニューが選択されました\n");
    // TODO: ハードディスク選択の実装
}

- (void)resetEmulator:(id)sender {
    if (self.emuInstance) {
        EMU* emu = (EMU*)self.emuInstance;
        printf("エミュレータをリセット\n");
        emu->reset();
    }
}

- (void)togglePause:(id)sender {
    if (self.viewController) {
        // TODO: 一時停止/再開の実装
        printf("一時停止/再開メニューが選択されました\n");
    }
}

- (void)showConfig:(id)sender {
    printf("設定メニューが選択されました\n");
    // TODO: 設定ダイアログの実装
}

- (void)showHelp:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"CSCP ヘルプ"];
    [alert setInformativeText:@"キーボード操作:\n- ESC: メニュー表示\n- F1-F5: ファンクションキー\n- Cmd+O: フロッピーディスク挿入\n- Cmd+R: リセット"];
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
}

// 遅延メディアマウント実行
- (void)performDelayedMediaMount {
    if (self.delayedArgs && self.emuInstance) {
        printf("遅延メディアマウントを実行中...\n");
        emulator_args_t* args = (emulator_args_t*)self.delayedArgs;
        EMU* emu = (EMU*)self.emuInstance;
        
        mount_media(emu, args);
        
        // マウント完了後にメモリを解放
        free(self.delayedArgs);
        self.delayedArgs = nullptr;
        
        printf("遅延メディアマウント完了\n");
    }
}

@end