/*
    View Controller for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [macOS/iOS/iPad view controller]
*/

#import "CSCPViewController.h"
#import "MetalView.h"
#include "../emu.h"
#include "xcode_mainloop.h"

@interface CSCPViewController ()
{
    NSTimer* emulationTimer;
    BOOL isEmulating;
}
@end

@implementation CSCPViewController

- (instancetype)initWithEmuInstance:(void*)emu {
    self = [super init];
    if (self) {
        self.emuInstance = emu;
        isEmulating = NO;
    }
    return self;
}

#if TARGET_OS_OSX
- (void)loadView {
    printf("CSCPViewController loadView 開始\n");
    // Create main view
    NSRect frame = NSMakeRect(0, 0, 640, 400);
    NSView* view = [[NSView alloc] initWithFrame:frame];
    self.view = view;
    
    // Create Metal view
    self.metalView = [[CSCPMetalView alloc] initWithFrame:frame device:nil];
    [self.view addSubview:self.metalView];
    
    // Setup constraints
    self.metalView.translatesAutoresizingMaskIntoConstraints = NO;
    [NSLayoutConstraint activateConstraints:@[
        [self.metalView.topAnchor constraintEqualToAnchor:self.view.topAnchor],
        [self.metalView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.metalView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [self.metalView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
    ]];
    
    // キーイベントを受信するために first responder に設定
    [self.view.window makeFirstResponder:self];
    
    // macOSではviewDidAppearが呼ばれないことがあるので、ここでエミュレーション開始
    printf("エミュレーション開始をスケジュール\n");
    dispatch_async(dispatch_get_main_queue(), ^{
        [self startEmulation];
        // エミュレーション開始後にも一度設定
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            [self.view.window makeFirstResponder:self];
            printf("キーイベント受信のためにfirst responderに設定\n");
        });
    });
}
#else
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Create Metal view for iOS/iPad
    self.metalView = [[CSCPMetalView alloc] initWithFrame:self.view.bounds device:nil];
    [self.view addSubview:self.metalView];
    
    // Setup constraints
    self.metalView.translatesAutoresizingMaskIntoConstraints = NO;
    [NSLayoutConstraint activateConstraints:@[
        [self.metalView.topAnchor constraintEqualToAnchor:self.view.topAnchor],
        [self.metalView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.metalView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [self.metalView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
    ]];
}
#endif

- (void)viewDidAppear {
    [super viewDidAppear];
    [self startEmulation];
}

- (void)viewWillDisappear {
    [super viewWillDisappear];
    [self stopEmulation];
}

- (void)startEmulation {
    printf("startEmulation 呼び出し - isEmulating: %s\n", isEmulating ? "YES" : "NO");
    if (isEmulating) return;
    
    isEmulating = YES;
    printf("エミュレーションタイマー設定中 (60FPS)\n");
    
    // MetalViewのレンダリングを開始
    if (self.metalView) {
        self.metalView.paused = NO;
        printf("MetalView レンダリング開始\n");
    }
    
    // 60 FPSでタイマーを設定
    emulationTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                                      target:self
                                                    selector:@selector(emulationTick:)
                                                    userInfo:nil
                                                     repeats:YES];
    printf("エミュレーションタイマー設定完了\n");
    
#if TARGET_OS_OSX
    // キーボード入力を受け取るためにfirst responderに設定
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.view.window makeFirstResponder:self];
        printf("First responder設定完了（ViewController）\n");
    });
#endif
}

- (void)stopEmulation {
    if (!isEmulating) return;
    
    isEmulating = NO;
    
    if (emulationTimer) {
        [emulationTimer invalidate];
        emulationTimer = nil;
    }
}

- (void)emulationTick:(NSTimer*)timer {
    if (!self.emuInstance) {
        printf("ERROR: emuInstance is NULL\n");
        return;
    }
    
    static int tickCount = 0;
    if (tickCount % 60 == 0) {
        printf("エミュレーションティック: %d秒経過\n", tickCount / 60);
    }
    tickCount++;
    
    EMU* emu = (EMU*)self.emuInstance;
    
    // エミュレータを1フレーム実行
    emu->run();
    
    // 画面更新
    OSD* osd = emu->get_osd();
    if (osd) {
        // OSDのMetalViewポインタを更新（初回のみ）
        if (!osd->metal_view && self.metalView) {
            printf("OSDのMetalViewポインタを設定\n");
            osd->metal_view = (__bridge void*)self.metalView;
            osd->metal_initialized = true;
        }
        
        // 画面描画
        osd->draw_screen();
    } else {
        printf("WARNING: OSD is NULL\n");
    }
}

#if TARGET_OS_OSX
// macOS キーボード処理 - first responder対応
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (void)keyDown:(NSEvent*)event {
    if (!self.emuInstance) return;
    
    EMU* emu = (EMU*)self.emuInstance;
    unsigned short nsKeyCode = [event keyCode];
    BOOL isRepeat = [event isARepeat];
    
    printf("キーダウン: NSKeyCode=%d\n", nsKeyCode);
    
    // OSDを経由してキー入力を処理（NSKeyCode → VK変換）
    OSD* osd = emu->get_osd();
    if (osd) {
        osd->key_down_native(nsKeyCode, isRepeat);
    }
}

- (void)keyUp:(NSEvent*)event {
    if (!self.emuInstance) return;
    
    EMU* emu = (EMU*)self.emuInstance;
    unsigned short nsKeyCode = [event keyCode];
    
    printf("キーアップ: NSKeyCode=%d\n", nsKeyCode);
    
    // OSDを経由してキー入力を処理（NSKeyCode → VK変換）
    OSD* osd = emu->get_osd();
    if (osd) {
        osd->key_up_native(nsKeyCode);
    }
}
#endif

- (void)dealloc {
    [self stopEmulation];
}

@end