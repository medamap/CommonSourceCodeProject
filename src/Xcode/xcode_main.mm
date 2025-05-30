// xcode_main.cpp
#include <cstdio>

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#import "Xcode/CSCPAppDelegate.h"
#endif
#endif

// GUI版でない場合のみインクルード
#ifndef USE_GUI
#include "xcode_mainloop.h"
#endif

int main(int argc, char* argv[])
{
    printf("CSCP エミュレータ - macOS/iOS 版\n");
    
#if defined(__APPLE__) && TARGET_OS_OSX && defined(USE_GUI)
    // macOS GUI版
    printf("GUI版として起動します\n");
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        CSCPAppDelegate* delegate = [[CSCPAppDelegate alloc] initWithArgc:argc argv:argv];
        [app setDelegate:delegate];
        
        [app run];
    }
#else
    // コンソール版
    printf("コンソール版として起動します\n");
    run_emulator_mainloop_with_args(argc, argv);
#endif
    
    return 0;
}
