//
// Created by Claude Code for testing menu integration
// Date: 2025/01/29
//
// [ macOS/iOS Menu System Test ]
//
// This file demonstrates how to integrate the menu system 
// into the Xcode/macOS environment

#include "xcode_menu.h"
#include <iostream>

// Test function to demonstrate menu creation and usage
void test_menu_system()
{
    printf("=== Menu System Test ===\n");
    
    // Create main menu
    Menu* mainMenu = create_main_menu();
    if (!mainMenu) {
        printf("Failed to create main menu\n");
        return;
    }
    
    printf("Menu created successfully\n");
    
    // Get root nodes
    auto rootNodes = mainMenu->getRootNodes();
    printf("Root menu items: %lu\n", rootNodes.size());
    
    for (const auto& node : rootNodes) {
        printf("  - %s (ID: %d, Type: %s)\n", 
            node.getCaption().c_str(), 
            node.getNodeId(),
            node.getItemType() == Category ? "Category" : "Property");
            
        // Get subnodes for categories
        if (node.getItemType() == Category) {
            auto subNodes = mainMenu->getNodes(node.getNodeId());
            for (const auto& subNode : subNodes) {
                printf("    - %s (Return: %d)\n", 
                    subNode.getCaption().c_str(), 
                    subNode.getReturnValue());
            }
        }
    }
    
    // Test menu state management
    printf("\n=== Testing Menu State Management ===\n");
    
    // Test radio button functionality
    mainMenu->CheckMenuRadioItem(ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER2);
    printf("Set CPU power to x4\n");
    
    // Test checkbox functionality
    mainMenu->CheckMenuItem(ID_FULL_SPEED, true);
    printf("Enabled full speed mode\n");
    
    // Test menu item enabling/disabling
    mainMenu->EnableMenuItem(ID_RESET, true);
    printf("Enabled reset menu item\n");
    
    // Update menu based on current state (simulated)
    update_popup_menu(&mainMenu);
    printf("Updated menu state\n");
    
    // Test event processing (simulated)
    printf("\n=== Testing Menu Event Processing ===\n");
    printf("Simulating menu events (without actual EMU object):\n");
    process_menu_event(ID_RESET, nullptr);
    process_menu_event(ID_CPU_POWER2, nullptr);
    process_menu_event(ID_FULL_SPEED, nullptr);
    
    // Cleanup
    delete mainMenu;
    printf("\nMenu system test completed successfully\n");
}

// Example of how to integrate menu into macOS NSMenu
void example_macos_integration()
{
    printf("\n=== macOS Integration Example ===\n");
    printf("// Example Objective-C code for macOS menu bar integration:\n");
    printf("/*\n");
    printf("// In your ViewController or AppDelegate:\n");
    printf("Menu* cppMenu = create_main_menu();\n");
    printf("NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@\"Emulator\"];\n");
    printf("\n");
    printf("// Get root nodes and create NSMenuItem for each\n");
    printf("auto rootNodes = cppMenu->getRootNodes();\n");
    printf("for (const auto& node : rootNodes) {\n");
    printf("    NSString* title = [NSString stringWithUTF8String:node.getCaption().c_str()];\n");
    printf("    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title action:@selector(menuAction:) keyEquivalent:@\"\"];\n");
    printf("    item.tag = node.getReturnValue();\n");
    printf("    \n");
    printf("    if (node.getItemType() == Category) {\n");
    printf("        // Create submenu for categories\n");
    printf("        NSMenu* submenu = [[NSMenu alloc] initWithTitle:title];\n");
    printf("        auto subNodes = cppMenu->getNodes(node.getNodeId());\n");
    printf("        for (const auto& subNode : subNodes) {\n");
    printf("            NSString* subTitle = [NSString stringWithUTF8String:subNode.getCaption().c_str()];\n");
    printf("            NSMenuItem* subItem = [[NSMenuItem alloc] initWithTitle:subTitle action:@selector(menuAction:) keyEquivalent:@\"\"];\n");
    printf("            subItem.tag = subNode.getReturnValue();\n");
    printf("            [submenu addItem:subItem];\n");
    printf("        }\n");
    printf("        item.submenu = submenu;\n");
    printf("    }\n");
    printf("    \n");
    printf("    [mainMenu addItem:item];\n");
    printf("}\n");
    printf("\n");
    printf("// Set the main menu\n");
    printf("[NSApp setMainMenu:mainMenu];\n");
    printf("\n");
    printf("// Menu action handler\n");
    printf("- (void)menuAction:(NSMenuItem*)sender {\n");
    printf("    int menuId = (int)sender.tag;\n");
    printf("    process_menu_event(menuId, emu); // emu is your EMU* instance\n");
    printf("}\n");
    printf("*/\n");
}

// Main test function
int main()
{
    test_menu_system();
    example_macos_integration();
    return 0;
}