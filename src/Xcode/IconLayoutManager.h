/*
    Icon Layout Manager for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.30
    
    [Icon layout strategy for macOS/iOS/iPad]
*/

#ifndef _ICONLAYOUTMANAGER_H_
#define _ICONLAYOUTMANAGER_H_

#import <TargetConditionals.h>

#if TARGET_OS_OSX
    #import <Cocoa/Cocoa.h>
    #import <MetalKit/MetalKit.h>
    // macOSではUIEdgeInsetsの代わりにNSEdgeInsetsを使用
    #define UIEdgeInsets NSEdgeInsets
    #define UIEdgeInsetsZero NSEdgeInsetsZero
    #define UIEdgeInsetsEqualToEdgeInsets NSEdgeInsetsEqual
    #define UIEdgeInsetsMake NSEdgeInsetsMake
#else
    #import <UIKit/UIKit.h>
    #import <MetalKit/MetalKit.h>
#endif

#import <Metal/Metal.h>
#import <simd/simd.h>
#import "IconRenderer.h"

// Platform-specific layout strategies
typedef NS_ENUM(NSInteger, IconLayoutStrategy) {
    IconLayoutStrategyInScreen = 0,     // Icons rendered within emulator screen (iOS/iPad mandatory)
    IconLayoutStrategySeparateWindow,   // Icons in separate window (macOS optional)
    IconLayoutStrategyOverlay,          // Icons as overlay on top of emulator (macOS optional)
    IconLayoutStrategyAdaptive          // Automatically choose best strategy based on platform/size
};

// Device type detection
typedef NS_ENUM(NSInteger, DeviceType) {
    DeviceTypeMac = 0,
    DeviceTypeiPad,
    DeviceTypeiPhone,
    DeviceTypeUnknown
};

// Screen orientation
typedef NS_ENUM(NSInteger, ScreenOrientation) {
    ScreenOrientationPortrait = 0,
    ScreenOrientationLandscape,
    ScreenOrientationPortraitUpsideDown,
    ScreenOrientationLandscapeLeft,
    ScreenOrientationLandscapeRight
};

// Layout configuration for different devices and orientations
typedef struct {
    DeviceType deviceType;
    ScreenOrientation orientation;
    IconLayoutStrategy strategy;
    CGSize screenSize;
    
    // Icon areas (normalized coordinates 0.0 to 1.0)
    CGRect fileIconArea;
    CGRect systemIconArea;
    CGRect progressBarArea;
    CGRect emulatorArea;
    
    // Icon sizing
    float iconSize;
    float iconSpacing;
    NSInteger maxFileIconsPerRow;
    NSInteger maxSystemIconsPerRow;
    
    // Safe area considerations (for iOS notch, etc.)
    UIEdgeInsets safeAreaInsets;
    
    // Performance settings
    BOOL useIconCaching;
    BOOL enableIconAnimations;
    float iconFadeInDuration;
} IconLayoutConfiguration;

// Icon layout manager class
@interface CSCPIconLayoutManager : NSObject

// Current configuration
@property (nonatomic) IconLayoutConfiguration currentConfig;
@property (nonatomic, strong) CSCPIconRenderer *iconRenderer;

// Platform detection
@property (nonatomic, readonly) DeviceType currentDeviceType;
@property (nonatomic, readonly) ScreenOrientation currentOrientation;

// Layout calculation
@property (nonatomic, readonly) CGRect calculatedEmulatorRect;
@property (nonatomic, readonly) CGRect calculatedIconArea;

// Initialize with screen size
- (instancetype)initWithScreenSize:(CGSize)screenSize;

// Platform detection
- (DeviceType)detectDeviceType;
- (ScreenOrientation)detectOrientation:(CGSize)screenSize;

// Layout strategy selection
- (IconLayoutStrategy)recommendedStrategyForDevice:(DeviceType)deviceType
                                       screenSize:(CGSize)screenSize;

// Configuration methods
- (void)updateLayoutForScreenSize:(CGSize)screenSize;
- (void)updateLayoutForOrientation:(ScreenOrientation)orientation;
- (void)updateSafeAreaInsets:(UIEdgeInsets)safeAreaInsets;

// Specific platform configurations
- (IconLayoutConfiguration)configurationForMac:(CGSize)screenSize;
- (IconLayoutConfiguration)configurationForIPad:(CGSize)screenSize 
                                     orientation:(ScreenOrientation)orientation
                                  safeAreaInsets:(UIEdgeInsets)safeAreaInsets;
- (IconLayoutConfiguration)configurationForIPhone:(CGSize)screenSize 
                                       orientation:(ScreenOrientation)orientation
                                    safeAreaInsets:(UIEdgeInsets)safeAreaInsets;

// Layout calculations
- (CGRect)calculateEmulatorAreaForConfig:(IconLayoutConfiguration)config;
- (CGRect)calculateFileIconAreaForConfig:(IconLayoutConfiguration)config;
- (CGRect)calculateSystemIconAreaForConfig:(IconLayoutConfiguration)config;
- (CGRect)calculateProgressBarAreaForConfig:(IconLayoutConfiguration)config;

// Icon positioning
- (void)positionIconsWithRenderer:(CSCPIconRenderer*)renderer;
- (NSArray<NSValue*>*)calculateIconPositionsInArea:(CGRect)area 
                                          iconCount:(NSInteger)iconCount
                                       maxPerRow:(NSInteger)maxPerRow
                                        iconSize:(float)iconSize
                                         spacing:(float)spacing;

// Safe area handling
- (CGRect)adjustRectForSafeArea:(CGRect)rect safeAreaInsets:(UIEdgeInsets)insets;
- (UIEdgeInsets)getCurrentSafeAreaInsets;

// Animation support
- (void)animateLayoutTransitionWithDuration:(NSTimeInterval)duration
                                 completion:(void(^)(BOOL finished))completion;

// Debug and testing
- (void)logCurrentConfiguration;
- (NSDictionary*)getLayoutMetrics;

@end

// C++ wrapper functions
#ifdef __cplusplus
extern "C" {
#endif

// Create layout manager
void* icon_layout_manager_create(float width, float height);

// Update layout
void icon_layout_manager_update_screen_size(void* manager, float width, float height);
void icon_layout_manager_update_orientation(void* manager, int orientation);
void icon_layout_manager_update_safe_area(void* manager, float top, float left, float bottom, float right);

// Get layout information
int icon_layout_manager_get_strategy(void* manager);
void icon_layout_manager_get_emulator_area(void* manager, float* x, float* y, float* width, float* height);
void icon_layout_manager_get_icon_area(void* manager, float* x, float* y, float* width, float* height);

// Configuration
void icon_layout_manager_set_strategy(void* manager, int strategy);
float icon_layout_manager_get_icon_size(void* manager);
float icon_layout_manager_get_icon_spacing(void* manager);

// Platform detection
int icon_layout_manager_get_device_type(void* manager);
int icon_layout_manager_get_orientation(void* manager);

// Cleanup
void icon_layout_manager_destroy(void* manager);

#ifdef __cplusplus
}
#endif

#endif // _ICONLAYOUTMANAGER_H_