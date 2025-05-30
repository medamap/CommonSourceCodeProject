/*
    View Controller for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [macOS/iOS/iPad view controller]
*/

#ifndef _CSCPVIEWCONTROLLER_H_
#define _CSCPVIEWCONTROLLER_H_

#import <TargetConditionals.h>

@class CSCPMetalView;

#if TARGET_OS_OSX
    #import <Cocoa/Cocoa.h>
    @interface CSCPViewController : NSViewController <NSWindowDelegate>
#else
    #import <UIKit/UIKit.h>
    @interface CSCPViewController : UIViewController
#endif

// Metal view property
@property (nonatomic, strong) CSCPMetalView* metalView;

// Emulator core pointer
@property (nonatomic) void* emuInstance;

// Initialize with emulator instance
- (instancetype)initWithEmuInstance:(void*)emu;

// Start/stop emulation
- (void)startEmulation;
- (void)stopEmulation;

// Handle keyboard input
#if TARGET_OS_OSX
- (void)keyDown:(NSEvent*)event;
- (void)keyUp:(NSEvent*)event;
#endif

@end

#endif // _CSCPVIEWCONTROLLER_H_