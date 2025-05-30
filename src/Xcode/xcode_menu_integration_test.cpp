//
// Xcode Menu Integration Test
// 
// 新しいメニューシステムの統合テスト
//

#include "xcode_menu_wrapper.h"
#include <iostream>
#include <cassert>

// テスト用のモックEMUクラス
class MockEMU {
public:
    bool is_floppy_disk_inserted(int drv) { return drv == 0; }
    bool is_floppy_disk_protected(int drv) { return false; }
    bool is_tape_inserted(int drv) { return drv == 0; }
    bool is_hard_disk_inserted(int drv) { return false; }
    bool is_cart_inserted(int drv) { return false; }
    void reset() { printf("MockEMU: リセット実行\n"); }
    void special_reset() { printf("MockEMU: 特殊リセット実行\n"); }
    void update_config() { printf("MockEMU: 設定更新\n"); }
    void close_floppy_disk(int drv) { printf("MockEMU: FD%dをイジェクト\n", drv); }
    void close_tape(int drv) { printf("MockEMU: テープ%dをイジェクト\n", drv); }
    void push_play(int drv) { printf("MockEMU: テープ%d再生\n", drv); }
    void push_stop(int drv) { printf("MockEMU: テープ%d停止\n", drv); }
    void push_fast_forward(int drv) { printf("MockEMU: テープ%d早送り\n", drv); }
    void push_fast_rewind(int drv) { printf("MockEMU: テープ%d巻き戻し\n", drv); }
};

// グローバルのモックEMU（テスト用）
MockEMU* mockEmu = nullptr;
EMU* emu = (EMU*)mockEmu;

// モック設定
config_t config = {
    .cpu_power = 0,
    .full_speed = false,
    .drive_vm_in_opecode = false,
    .correct_disk_timing = {true, true, false, false},
    .ignore_disk_crc = {false, false, false, false}
};

void test_menu_creation()
{
    printf("\n=== メニュー作成テスト ===\n");
    
    init_menu_system();
    Menu* menu = get_main_menu();
    
    assert(menu != nullptr);
    printf("✓ メニューが正常に作成されました\n");
    
    // ルートノードの確認
    auto rootNodes = menu->getRootNodes();
    printf("✓ ルートメニュー項目数: %lu\n", rootNodes.size());
    
    // 各メニュー項目の確認
    for (const auto& node : rootNodes) {
        printf("  - %s (ID: %d)\n", node.getCaption().c_str(), node.getNodeId());
    }
}

void test_menu_state_update()
{
    printf("\n=== メニュー状態更新テスト ===\n");
    
    Menu* menu = get_main_menu();
    
    // CPU power設定
    config.cpu_power = 2;
    update_control_menu(menu);
    printf("✓ CPU power x4に設定\n");
    
    // Full speed設定
    config.full_speed = true;
    update_control_menu(menu);
    printf("✓ Full speedを有効化\n");
    
    // メニュー全体を更新
    update_menu_system(emu);
    printf("✓ メニューシステム全体を更新\n");
}

void test_menu_events()
{
    printf("\n=== メニューイベント処理テスト ===\n");
    
    // リセットイベント
    printf("- リセットイベントをテスト:\n");
    handle_menu_event(ID_RESET, emu);
    
    // CPU power変更
    printf("- CPU power変更イベントをテスト:\n");
    handle_menu_event(ID_CPU_POWER3, emu);
    
    // フロッピーディスクイジェクト
    printf("- フロッピーディスクイジェクトをテスト:\n");
    handle_menu_event(ID_CLOSE_FD1, emu);
    
    // テープ操作
    printf("- テープ操作をテスト:\n");
    handle_menu_event(ID_PLAY_BUTTON1, emu);
    handle_menu_event(ID_STOP_BUTTON1, emu);
}

void test_nsmeenu_integration()
{
    printf("\n=== NSMenu統合サンプル ===\n");
    printf("// Objective-C++での使用例:\n");
    printf("/*\n");
    printf("@implementation MenuController\n");
    printf("\n");
    printf("- (void)setupMainMenu {\n");
    printf("    init_menu_system();\n");
    printf("    Menu* cppMenu = get_main_menu();\n");
    printf("    \n");
    printf("    NSMenu* mainMenu = [[NSMenu alloc] init];\n");
    printf("    auto rootNodes = cppMenu->getRootNodes();\n");
    printf("    \n");
    printf("    for (const auto& node : rootNodes) {\n");
    printf("        NSString* title = [NSString stringWithUTF8String:node.getCaption().c_str()];\n");
    printf("        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title\n");
    printf("                                                      action:@selector(menuAction:)\n");
    printf("                                               keyEquivalent:@\"\"];\n");
    printf("        item.tag = node.getReturnValue();\n");
    printf("        \n");
    printf("        if (node.getItemType() == Category) {\n");
    printf("            NSMenu* submenu = [self createSubmenuForNode:node.getNodeId() fromMenu:cppMenu];\n");
    printf("            item.submenu = submenu;\n");
    printf("        }\n");
    printf("        \n");
    printf("        [mainMenu addItem:item];\n");
    printf("    }\n");
    printf("    \n");
    printf("    [NSApp setMainMenu:mainMenu];\n");
    printf("}\n");
    printf("\n");
    printf("- (void)menuAction:(NSMenuItem*)sender {\n");
    printf("    handle_menu_event((int)sender.tag, emu);\n");
    printf("}\n");
    printf("\n");
    printf("@end\n");
    printf("*/\n");
}

int main()
{
    printf("=== Xcode メニューシステム統合テスト ===\n");
    printf("機種別メニューファイルを直接使用する新方式\n\n");
    
    // モックEMU作成
    mockEmu = new MockEMU();
    emu = (EMU*)mockEmu;
    
    // 各種テスト実行
    test_menu_creation();
    test_menu_state_update();
    test_menu_events();
    test_nsmeenu_integration();
    
    // クリーンアップ
    cleanup_menu_system();
    delete mockEmu;
    
    printf("\n✅ すべてのテストが完了しました\n");
    return 0;
}