/*
    App Delegate for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [macOS application delegate]
*/

#ifndef _CSCPAPPDELEGATE_H_
#define _CSCPAPPDELEGATE_H_

#import <Cocoa/Cocoa.h>

@class CSCPViewController;

// Forward declarations for C++ types
#ifdef __cplusplus
class EMU;
class Menu;
#endif

@interface CSCPAppDelegate : NSObject <NSApplicationDelegate>

@property (strong) NSWindow *window;
@property (strong) CSCPViewController* viewController;
@property (nonatomic) void* emuInstance;
@property (nonatomic) void* menuSystem;

// Command line arguments
@property (nonatomic) int argc;
@property (nonatomic) char** argv;

// Delayed media mounting
@property (nonatomic) void* delayedArgs;

// Initialize with command line arguments
- (instancetype)initWithArgc:(int)argc argv:(char**)argv;

// Menu creation and actions
#ifdef __cplusplus
- (void)createMenuBar:(EMU*)emu;
- (void)addMenuItems:(NSMenu*)menu fromMenu:(Menu*)menuSystem forParentId:(int)parentId;
#else
- (void)createMenuBar:(void*)emu;
- (void)addMenuItems:(NSMenu*)menu fromMenu:(void*)menuSystem forParentId:(int)parentId;
#endif
- (void)handleMenuAction:(id)sender;
- (void)showAbout:(id)sender;
- (void)openFloppyDisk:(id)sender;
- (void)openHardDisk:(id)sender;
- (void)resetEmulator:(id)sender;
- (void)togglePause:(id)sender;
- (void)showConfig:(id)sender;
- (void)showHelp:(id)sender;

// Delayed media mounting
- (void)performDelayedMediaMount;

@end

#endif // _CSCPAPPDELEGATE_H_