//
// Created by Claude Code for menu integration demonstration  
// Date: 2025/01/29
//
// [ Menu Integration Example for Xcode Project ]
//
// This file shows how to integrate the separated menu system
// into the existing Xcode emulator project

#include "xcode_menu.h"
#include "xcode_mainloop.h"
#include "emu.h"
#include <cstdio>

// Global menu instance
static Menu* g_mainMenu = nullptr;

// Initialize menu system
extern "C" void init_menu_system()
{
    printf("メニューシステムを初期化中...\n");
    
    if (g_mainMenu) {
        delete g_mainMenu;
    }
    
    g_mainMenu = create_main_menu();
    if (g_mainMenu) {
        printf("メニューシステム初期化完了\n");
    } else {
        printf("メニューシステム初期化失敗\n");
    }
}

// Update menu based on current emulator state
extern "C" void update_menu_system(EMU* emu)
{
    if (!g_mainMenu) {
        init_menu_system();
        return;
    }
    
    // Update menu state based on emulator
    update_popup_menu(&g_mainMenu);
}

// Get menu for macOS NSMenu integration
extern "C" Menu* get_main_menu()
{
    if (!g_mainMenu) {
        init_menu_system();
    }
    return g_mainMenu;
}

// Process menu events
extern "C" void handle_menu_event(int menuId, EMU* emu)
{
    if (!g_mainMenu) {
        printf("Menu system not initialized\n");
        return;
    }
    
    printf("Processing menu event: %d\n", menuId);
    process_menu_event(menuId, emu);
    
    // Update menu state after processing event
    update_menu_system(emu);
}

// Cleanup menu system
extern "C" void cleanup_menu_system()
{
    if (g_mainMenu) {
        delete g_mainMenu;
        g_mainMenu = nullptr;
        printf("メニューシステムをクリーンアップしました\n");
    }
}

// Example of enhanced emulator main loop with menu integration
extern "C" void run_emulator_mainloop_with_menu(int argc, char** argv)
{
    // コマンドライン引数解析
    emulator_args_t args;
    parse_command_line_args(argc, argv, &args);
    
    // ヘルプ表示
    if (args.show_help) {
        show_help();
        return;
    }
    
    printf("エミュレータを初期化中...\n");
    printf("使用機種: %s\n", args.machine_name);

    // EMUオブジェクトの生成
    EMU* emu = new EMU();
    if (!emu) {
        printf("EMU オブジェクトの生成に失敗しました\n");
        return;
    }

    // メニューシステム初期化
    init_menu_system();
    
    // 設定ファイル読込み
    load_ini_file(emu, args.machine_name);
    
    // メディアマウント
    mount_media(emu, &args);
    
    // メニューシステム更新
    update_menu_system(emu);

    printf("メインループを開始します...\n");

    // メインループ（60FPS）
    const int target_fps = 60;
    const auto frame_duration = std::chrono::milliseconds(1000 / target_fps);
    
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();
    
    // メインループ（通常は無限ループですが、テスト用に300フレーム）
    for (int i = 0; i < 300; i++) {
        auto frame_start = std::chrono::steady_clock::now();
        
        // エミュレーターを1フレーム実行
        emu->run();
        
        frame_count++;
        
        // 60フレームごとにメニューシステムを更新
        if (frame_count % 60 == 0) {
            update_menu_system(emu);
        }
        
        // フレームレート制御
        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;
        
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
        
        // 1秒ごとに状況を出力
        if (frame_count % 60 == 0) {
            auto current_time = std::chrono::steady_clock::now();
            auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            printf("フレーム %d実行完了 (経過時間: %lld ms)\n", frame_count, total_elapsed.count());
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    printf("メインループ終了: %d フレーム実行, 総経過時間: %lld ms\n", frame_count, total_elapsed.count());

    printf("リソース解放中...\n");
    cleanup_menu_system();
    delete emu;
}

// Demonstrate menu functionality
extern "C" void demo_menu_functionality()
{
    printf("\n=== Menu Functionality Demo ===\n");
    
    // Initialize menu
    init_menu_system();
    
    Menu* menu = get_main_menu();
    if (!menu) {
        printf("Failed to get main menu\n");
        return;
    }
    
    // Show menu structure
    auto rootNodes = menu->getRootNodes();
    printf("Main menu contains %lu top-level items:\n", rootNodes.size());
    
    for (const auto& node : rootNodes) {
        printf("- %s", node.getCaption().c_str());
        if (node.getItemType() == Category) {
            auto children = menu->getNodes(node.getNodeId());
            printf(" (%lu items)", children.size());
        }
        printf("\n");
    }
    
    // Simulate some menu events
    printf("\nSimulating menu events:\n");
    
    // Test reset
    printf("- Reset command\n");
    handle_menu_event(ID_RESET, nullptr);
    
    // Test CPU power change
    printf("- Set CPU power to x4\n");
    handle_menu_event(ID_CPU_POWER2, nullptr);
    
    // Test full speed toggle
    printf("- Toggle full speed\n");
    handle_menu_event(ID_FULL_SPEED, nullptr);
    
    cleanup_menu_system();
    printf("Menu demo completed\n");
}