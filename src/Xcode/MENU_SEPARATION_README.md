# Menu System Separation for Xcode Platform

## Overview

This document describes the separation of menu processing logic from `android_main.cpp` into dedicated Xcode-specific menu files. This separation allows for better code organization and platform-specific customization while maintaining the core menu functionality.

## Files Created

### 1. `xcode_menu.h`
- **Purpose**: Header file containing menu interface declarations
- **Key Components**:
  - `MenuNode` class: Represents individual menu items with properties
  - `BaseMenu` class: Base class for hierarchical menu management
  - `Menu` class: Machine-specific menu implementation
  - Menu constants for different media types (FDD, TAPE, HDD, etc.)
  - Function declarations for menu management and event processing

### 2. `xcode_menu.cpp`
- **Purpose**: Implementation of the menu system extracted from Android
- **Key Features**:
  - Complete `BaseMenu` implementation with node management
  - X1 Turbo-specific `Menu` constructor with full menu structure
  - Menu update functions extracted from `android_main.cpp`:
    - `update_control_menu()`
    - `update_floppy_disk_menu()`
    - `update_tape_menu()`
    - `update_hard_disk_menu()`
    - `update_cart_menu()`
  - Event processing function `process_menu_event()`
  - Menu creation and management functions

### 3. `xcode_menu_test.cpp`
- **Purpose**: Test file demonstrating menu system functionality
- **Features**:
  - Basic menu creation and structure testing
  - Menu state management testing
  - Event processing simulation
  - macOS integration example code

### 4. `xcode_menu_integration_example.cpp`
- **Purpose**: Demonstrates how to integrate menu system into existing Xcode project
- **Features**:
  - C-compatible interface functions
  - Enhanced main loop with menu integration
  - Menu initialization and cleanup
  - Event handling example

## Architecture

### Menu Hierarchy
The menu system follows the same structure as the Android version:

```
Root
├── Control
│   ├── Reset
│   ├── NMI
│   ├── CPU Power (x1, x2, x4, x8, x16)
│   ├── Full Speed
│   ├── Save State (0-9)
│   ├── Load State (0-9)
│   └── Debug Options
├── FD0-FD3 (Floppy Disk Drives)
│   ├── Insert
│   ├── Eject
│   ├── Write Protect
│   └── Settings
├── CMT (Cassette Tape)
│   ├── Insert/Eject
│   └── Transport Controls
├── HD0-HD3 (Hard Disk Drives)
│   ├── Insert
│   └── Eject
├── Device Settings
└── Host Settings
```

### Key Classes

#### `MenuNode`
Represents an individual menu item with:
- Hierarchical structure (parent/child relationships)
- Item type (Category or Property)
- Display properties (caption, thumbnail)
- State properties (checked, enabled)
- Return value for event handling

#### `BaseMenu`
Provides core menu management functionality:
- Node creation and management
- Hierarchical navigation
- State management (checked/enabled states)
- Radio button group handling

#### `Menu`
Machine-specific implementation:
- X1 Turbo menu structure
- Conditional compilation for optional features
- Platform-specific menu items

## Integration with macOS

### NSMenu Integration
The system can be integrated with macOS NSMenu:

```objc
// Get C++ menu instance
Menu* cppMenu = get_main_menu();

// Create NSMenu
NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"Emulator"];

// Convert menu structure
auto rootNodes = cppMenu->getRootNodes();
for (const auto& node : rootNodes) {
    NSString* title = [NSString stringWithUTF8String:node.getCaption().c_str()];
    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title 
                                               action:@selector(menuAction:) 
                                        keyEquivalent:@""];
    item.tag = node.getReturnValue();
    [mainMenu addItem:item];
}

[NSApp setMainMenu:mainMenu];
```

### Event Handling
Menu events are processed through the `process_menu_event()` function:

```objc
- (void)menuAction:(NSMenuItem*)sender {
    int menuId = (int)sender.tag;
    handle_menu_event(menuId, emu);
}
```

## Benefits of Separation

1. **Platform Independence**: Menu logic is separated from Android-specific code
2. **Code Reusability**: Core menu functionality can be shared across platforms
3. **Maintainability**: Easier to maintain and update menu functionality
4. **Customization**: Platform-specific menu features can be added easily
5. **Testing**: Menu functionality can be tested independently

## Usage in Xcode Project

### Basic Integration
```cpp
// Initialize menu system
init_menu_system();

// Get menu instance
Menu* menu = get_main_menu();

// Update menu state
update_menu_system(emu);

// Handle menu events
handle_menu_event(menuId, emu);

// Cleanup
cleanup_menu_system();
```

### Enhanced Main Loop
The integration example shows how to incorporate menu updates into the main emulator loop:

```cpp
// Initialize
init_menu_system();
update_menu_system(emu);

// Main loop
while (running) {
    emu->run();
    
    // Update menu periodically
    if (frame_count % 60 == 0) {
        update_menu_system(emu);
    }
}

// Cleanup
cleanup_menu_system();
```

## Configuration Support

The menu system supports all the same configuration options as the Android version:
- CPU power settings
- Full speed mode
- Drive settings
- Sound settings
- Monitor settings
- Debug options

## Future Enhancements

Possible future improvements:
1. **File Dialog Integration**: Add NSOpenPanel integration for file selection
2. **Menu Bar Icons**: Add support for menu item icons
3. **Keyboard Shortcuts**: Add keyboard shortcut support
4. **Context Menus**: Add right-click context menu support
5. **Menu State Persistence**: Save/restore menu states in configuration

## Testing

Use the test files to verify functionality:

```bash
# Compile and run test
g++ -std=c++11 xcode_menu_test.cpp xcode_menu.cpp -o menu_test
./menu_test
```

This will demonstrate:
- Menu creation and structure
- State management
- Event processing
- Integration examples

## Conclusion

The menu separation provides a clean, maintainable architecture for menu handling in the Xcode platform while preserving all functionality from the original Android implementation. The modular design allows for easy customization and integration with macOS-specific UI components.