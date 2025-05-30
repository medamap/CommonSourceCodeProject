/*
    Icon Layout Manager for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.30
    
    [Icon layout strategy for macOS/iOS/iPad]
*/

#import "IconLayoutManager.h"
#include "../common.h"

@implementation CSCPIconLayoutManager

- (instancetype)initWithScreenSize:(CGSize)screenSize {
    self = [super init];
    if (self) {
        // Initialize with detected device and orientation
        _currentDeviceType = [self detectDeviceType];
        _currentOrientation = [self detectOrientation:screenSize];
        
        // Update layout for initial screen size
        [self updateLayoutForScreenSize:screenSize];
        
        printf("Icon Layout Manager initialized: Device=%d, Orientation=%d, Size=%.0fx%.0f\n",
               (int)_currentDeviceType, (int)_currentOrientation, screenSize.width, screenSize.height);
    }
    return self;
}

- (DeviceType)detectDeviceType {
#if TARGET_OS_OSX
    return DeviceTypeMac;
#else
    UIDevice *device = [UIDevice currentDevice];
    
    if ([device.model containsString:@"iPad"]) {
        return DeviceTypeiPad;
    } else if ([device.model containsString:@"iPhone"]) {
        return DeviceTypeiPhone;
    } else {
        return DeviceTypeUnknown;
    }
#endif
}

- (ScreenOrientation)detectOrientation:(CGSize)screenSize {
    if (screenSize.width > screenSize.height) {
        return ScreenOrientationLandscape;
    } else {
        return ScreenOrientationPortrait;
    }
}

- (IconLayoutStrategy)recommendedStrategyForDevice:(DeviceType)deviceType
                                       screenSize:(CGSize)screenSize {
    
    switch (deviceType) {
        case DeviceTypeMac:
            // macOS: Prefer separate window for large screens, overlay for smaller
            if (screenSize.width >= 1920 && screenSize.height >= 1080) {
                return IconLayoutStrategySeparateWindow;
            } else {
                return IconLayoutStrategyOverlay;
            }
            
        case DeviceTypeiPad:
            // iPad: Always in-screen, but with different layouts for size classes
            return IconLayoutStrategyInScreen;
            
        case DeviceTypeiPhone:
            // iPhone: Always in-screen due to space constraints
            return IconLayoutStrategyInScreen;
            
        case DeviceTypeUnknown:
        default:
            return IconLayoutStrategyAdaptive;
    }
}

- (void)updateLayoutForScreenSize:(CGSize)screenSize {
    // Update current orientation
    _currentOrientation = [self detectOrientation:screenSize];
    
    // Get platform-specific configuration
    switch (_currentDeviceType) {
        case DeviceTypeMac:
            self.currentConfig = [self configurationForMac:screenSize];
            break;
            
        case DeviceTypeiPad:
            self.currentConfig = [self configurationForIPad:screenSize 
                                                orientation:_currentOrientation
                                             safeAreaInsets:[self getCurrentSafeAreaInsets]];
            break;
            
        case DeviceTypeiPhone:
            self.currentConfig = [self configurationForIPhone:screenSize 
                                                   orientation:_currentOrientation
                                                safeAreaInsets:[self getCurrentSafeAreaInsets]];
            break;
            
        default:
            // Fallback to iPad configuration
            self.currentConfig = [self configurationForIPad:screenSize 
                                                orientation:_currentOrientation
                                             safeAreaInsets:UIEdgeInsetsZero];
            break;
    }
    
    // Calculate layout areas
    _calculatedEmulatorRect = [self calculateEmulatorAreaForConfig:self.currentConfig];
    _calculatedIconArea = CGRectUnion([self calculateFileIconAreaForConfig:self.currentConfig],
                                      [self calculateSystemIconAreaForConfig:self.currentConfig]);
    
    printf("Layout updated: Strategy=%d, Emulator=%.0f,%.0f,%.0fx%.0f\n",
           (int)self.currentConfig.strategy,
           _calculatedEmulatorRect.origin.x, _calculatedEmulatorRect.origin.y,
           _calculatedEmulatorRect.size.width, _calculatedEmulatorRect.size.height);
}

- (IconLayoutConfiguration)configurationForMac:(CGSize)screenSize {
    IconLayoutConfiguration config = {};
    
    config.deviceType = DeviceTypeMac;
    config.orientation = [self detectOrientation:screenSize];
    config.strategy = [self recommendedStrategyForDevice:DeviceTypeMac screenSize:screenSize];
    config.screenSize = screenSize;
    config.safeAreaInsets = UIEdgeInsetsZero; // macOS doesn't have safe area
    
    if (config.strategy == IconLayoutStrategySeparateWindow) {
        // Separate window: emulator uses full screen
        config.emulatorArea = CGRectMake(0.0f, 0.0f, 1.0f, 1.0f);
        config.fileIconArea = CGRectZero;
        config.systemIconArea = CGRectZero;
        config.progressBarArea = CGRectZero;
    } else {
        // Overlay or in-screen: reserve space for icons
        if (config.orientation == ScreenOrientationLandscape) {
            // Landscape: icons on sides
            config.fileIconArea = CGRectMake(0.0f, 0.0f, 0.12f, 1.0f);
            config.systemIconArea = CGRectMake(0.88f, 0.0f, 0.12f, 1.0f);
            config.progressBarArea = CGRectMake(0.12f, 0.92f, 0.76f, 0.08f);
            config.emulatorArea = CGRectMake(0.12f, 0.0f, 0.76f, 0.92f);
        } else {
            // Portrait: icons on top/bottom
            config.fileIconArea = CGRectMake(0.0f, 0.0f, 1.0f, 0.12f);
            config.systemIconArea = CGRectMake(0.0f, 0.88f, 1.0f, 0.12f);
            config.progressBarArea = CGRectMake(0.0f, 0.12f, 1.0f, 0.06f);
            config.emulatorArea = CGRectMake(0.0f, 0.18f, 1.0f, 0.70f);
        }
    }
    
    // macOS-specific settings
    config.iconSize = 0.06f; // Smaller icons for precise mouse control
    config.iconSpacing = 0.01f;
    config.maxFileIconsPerRow = (config.orientation == ScreenOrientationLandscape) ? 1 : 8;
    config.maxSystemIconsPerRow = (config.orientation == ScreenOrientationLandscape) ? 1 : 8;
    config.useIconCaching = YES;
    config.enableIconAnimations = YES;
    config.iconFadeInDuration = 0.2f;
    
    return config;
}

- (IconLayoutConfiguration)configurationForIPad:(CGSize)screenSize 
                                     orientation:(ScreenOrientation)orientation
                                  safeAreaInsets:(UIEdgeInsets)safeAreaInsets {
    IconLayoutConfiguration config = {};
    
    config.deviceType = DeviceTypeiPad;
    config.orientation = orientation;
    config.strategy = IconLayoutStrategyInScreen; // iPad always uses in-screen
    config.screenSize = screenSize;
    config.safeAreaInsets = safeAreaInsets;
    
    // iPad layout with safe area considerations
    float safeTop = safeAreaInsets.top / screenSize.height;
    float safeBottom = safeAreaInsets.bottom / screenSize.height;
    float safeLeft = safeAreaInsets.left / screenSize.width;
    float safeRight = safeAreaInsets.right / screenSize.width;
    
    if (orientation == ScreenOrientationLandscape) {
        // Landscape iPad: icons on sides with safe area
        float iconWidth = 0.15f;
        config.fileIconArea = CGRectMake(safeLeft, safeTop, iconWidth, 1.0f - safeTop - safeBottom);
        config.systemIconArea = CGRectMake(1.0f - iconWidth - safeRight, safeTop, iconWidth, 1.0f - safeTop - safeBottom);
        config.progressBarArea = CGRectMake(safeLeft + iconWidth, 0.88f, 1.0f - 2*iconWidth - safeLeft - safeRight, 0.08f);
        config.emulatorArea = CGRectMake(safeLeft + iconWidth, safeTop, 1.0f - 2*iconWidth - safeLeft - safeRight, 0.88f - safeTop);
        
        config.maxFileIconsPerRow = 1;
        config.maxSystemIconsPerRow = 1;
    } else {
        // Portrait iPad: icons on top/bottom with safe area
        float iconHeight = 0.12f;
        config.fileIconArea = CGRectMake(safeLeft, safeTop, 1.0f - safeLeft - safeRight, iconHeight);
        config.systemIconArea = CGRectMake(safeLeft, 1.0f - iconHeight - safeBottom, 1.0f - safeLeft - safeRight, iconHeight);
        config.progressBarArea = CGRectMake(safeLeft, safeTop + iconHeight, 1.0f - safeLeft - safeRight, 0.06f);
        config.emulatorArea = CGRectMake(safeLeft, safeTop + iconHeight + 0.06f, 1.0f - safeLeft - safeRight, 1.0f - 2*iconHeight - 0.06f - safeTop - safeBottom);
        
        config.maxFileIconsPerRow = 6;
        config.maxSystemIconsPerRow = 6;
    }
    
    // iPad-specific settings
    config.iconSize = 0.08f; // Medium size for touch
    config.iconSpacing = 0.02f;
    config.useIconCaching = YES;
    config.enableIconAnimations = YES;
    config.iconFadeInDuration = 0.3f;
    
    return config;
}

- (IconLayoutConfiguration)configurationForIPhone:(CGSize)screenSize 
                                       orientation:(ScreenOrientation)orientation
                                    safeAreaInsets:(UIEdgeInsets)safeAreaInsets {
    IconLayoutConfiguration config = {};
    
    config.deviceType = DeviceTypeiPhone;
    config.orientation = orientation;
    config.strategy = IconLayoutStrategyInScreen; // iPhone always uses in-screen
    config.screenSize = screenSize;
    config.safeAreaInsets = safeAreaInsets;
    
    // iPhone layout with safe area considerations (notch, home indicator)
    float safeTop = safeAreaInsets.top / screenSize.height;
    float safeBottom = safeAreaInsets.bottom / screenSize.height;
    float safeLeft = safeAreaInsets.left / screenSize.width;
    float safeRight = safeAreaInsets.right / screenSize.width;
    
    if (orientation == ScreenOrientationLandscape) {
        // Landscape iPhone: compact layout
        float iconWidth = 0.18f;
        config.fileIconArea = CGRectMake(safeLeft, safeTop, iconWidth, 1.0f - safeTop - safeBottom);
        config.systemIconArea = CGRectMake(1.0f - iconWidth - safeRight, safeTop, iconWidth, 1.0f - safeTop - safeBottom);
        config.progressBarArea = CGRectMake(safeLeft + iconWidth, 0.85f, 1.0f - 2*iconWidth - safeLeft - safeRight, 0.10f);
        config.emulatorArea = CGRectMake(safeLeft + iconWidth, safeTop, 1.0f - 2*iconWidth - safeLeft - safeRight, 0.85f - safeTop);
        
        config.maxFileIconsPerRow = 1;
        config.maxSystemIconsPerRow = 1;
    } else {
        // Portrait iPhone: maximize emulator space
        float iconHeight = 0.15f;
        config.fileIconArea = CGRectMake(safeLeft, safeTop, 1.0f - safeLeft - safeRight, iconHeight);
        config.systemIconArea = CGRectMake(safeLeft, 1.0f - iconHeight - safeBottom, 1.0f - safeLeft - safeRight, iconHeight);
        config.progressBarArea = CGRectMake(safeLeft, safeTop + iconHeight, 1.0f - safeLeft - safeRight, 0.08f);
        config.emulatorArea = CGRectMake(safeLeft, safeTop + iconHeight + 0.08f, 1.0f - safeLeft - safeRight, 1.0f - 2*iconHeight - 0.08f - safeTop - safeBottom);
        
        config.maxFileIconsPerRow = 4;
        config.maxSystemIconsPerRow = 4;
    }
    
    // iPhone-specific settings (larger icons for easier touch)
    config.iconSize = 0.12f;
    config.iconSpacing = 0.02f;
    config.useIconCaching = YES;
    config.enableIconAnimations = NO; // Disable for performance on smaller devices
    config.iconFadeInDuration = 0.0f;
    
    return config;
}

- (CGRect)calculateEmulatorAreaForConfig:(IconLayoutConfiguration)config {
    CGRect area = config.emulatorArea;
    
    // Apply safe area adjustments
    area = [self adjustRectForSafeArea:area safeAreaInsets:config.safeAreaInsets];
    
    // Convert normalized coordinates to actual screen coordinates
    area.origin.x *= config.screenSize.width;
    area.origin.y *= config.screenSize.height;
    area.size.width *= config.screenSize.width;
    area.size.height *= config.screenSize.height;
    
    return area;
}

- (CGRect)calculateFileIconAreaForConfig:(IconLayoutConfiguration)config {
    CGRect area = config.fileIconArea;
    area = [self adjustRectForSafeArea:area safeAreaInsets:config.safeAreaInsets];
    
    area.origin.x *= config.screenSize.width;
    area.origin.y *= config.screenSize.height;
    area.size.width *= config.screenSize.width;
    area.size.height *= config.screenSize.height;
    
    return area;
}

- (CGRect)calculateSystemIconAreaForConfig:(IconLayoutConfiguration)config {
    CGRect area = config.systemIconArea;
    area = [self adjustRectForSafeArea:area safeAreaInsets:config.safeAreaInsets];
    
    area.origin.x *= config.screenSize.width;
    area.origin.y *= config.screenSize.height;
    area.size.width *= config.screenSize.width;
    area.size.height *= config.screenSize.height;
    
    return area;
}

- (CGRect)calculateProgressBarAreaForConfig:(IconLayoutConfiguration)config {
    CGRect area = config.progressBarArea;
    area = [self adjustRectForSafeArea:area safeAreaInsets:config.safeAreaInsets];
    
    area.origin.x *= config.screenSize.width;
    area.origin.y *= config.screenSize.height;
    area.size.width *= config.screenSize.width;
    area.size.height *= config.screenSize.height;
    
    return area;
}

- (CGRect)adjustRectForSafeArea:(CGRect)rect safeAreaInsets:(UIEdgeInsets)insets {
    // For normalized coordinates, safe area insets need to be normalized too
    // This is already handled in the configuration methods
    return rect;
}

- (UIEdgeInsets)getCurrentSafeAreaInsets {
#if TARGET_OS_OSX
    return UIEdgeInsetsZero;
#else
    if (@available(iOS 11.0, *)) {
        UIWindow *window = [UIApplication sharedApplication].keyWindow;
        if (window) {
            return window.safeAreaInsets;
        }
    }
    return UIEdgeInsetsZero;
#endif
}

- (void)updateLayoutForOrientation:(ScreenOrientation)orientation {
    if (_currentOrientation != orientation) {
        _currentOrientation = orientation;
        [self updateLayoutForScreenSize:self.currentConfig.screenSize];
    }
}

- (void)updateSafeAreaInsets:(UIEdgeInsets)safeAreaInsets {
    if (!UIEdgeInsetsEqualToEdgeInsets(self.currentConfig.safeAreaInsets, safeAreaInsets)) {
        self.currentConfig.safeAreaInsets = safeAreaInsets;
        [self updateLayoutForScreenSize:self.currentConfig.screenSize];
    }
}

- (void)positionIconsWithRenderer:(CSCPIconRenderer*)renderer {
    if (!renderer) return;
    
    // Update renderer layout based on current configuration
    [renderer calculateLayoutForScreenSize:self.currentConfig.screenSize];
    
    // Apply our calculated layout to the renderer
    IconLayout iconLayout;
    iconLayout.isLandscape = (self.currentConfig.orientation == ScreenOrientationLandscape);
    
    // Convert areas to normalized coordinates for renderer
    iconLayout.fileIconArea = simd_make_float4(
        self.currentConfig.fileIconArea.origin.x / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        self.currentConfig.fileIconArea.origin.y / self.currentConfig.screenSize.height * 2.0f - 1.0f,
        (self.currentConfig.fileIconArea.origin.x + self.currentConfig.fileIconArea.size.width) / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        (self.currentConfig.fileIconArea.origin.y + self.currentConfig.fileIconArea.size.height) / self.currentConfig.screenSize.height * 2.0f - 1.0f
    );
    
    iconLayout.systemIconArea = simd_make_float4(
        self.currentConfig.systemIconArea.origin.x / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        self.currentConfig.systemIconArea.origin.y / self.currentConfig.screenSize.height * 2.0f - 1.0f,
        (self.currentConfig.systemIconArea.origin.x + self.currentConfig.systemIconArea.size.width) / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        (self.currentConfig.systemIconArea.origin.y + self.currentConfig.systemIconArea.size.height) / self.currentConfig.screenSize.height * 2.0f - 1.0f
    );
    
    iconLayout.emulatorArea = simd_make_float4(
        self.currentConfig.emulatorArea.origin.x / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        self.currentConfig.emulatorArea.origin.y / self.currentConfig.screenSize.height * 2.0f - 1.0f,
        (self.currentConfig.emulatorArea.origin.x + self.currentConfig.emulatorArea.size.width) / self.currentConfig.screenSize.width * 2.0f - 1.0f,
        (self.currentConfig.emulatorArea.origin.y + self.currentConfig.emulatorArea.size.height) / self.currentConfig.screenSize.height * 2.0f - 1.0f
    );
    
    iconLayout.iconSize = self.currentConfig.iconSize;
    iconLayout.iconSpacing = self.currentConfig.iconSpacing;
    
    renderer.iconLayout = iconLayout;
}

- (void)logCurrentConfiguration {
    printf("=== Icon Layout Configuration ===\n");
    printf("Device: %d, Orientation: %d, Strategy: %d\n", 
           (int)self.currentConfig.deviceType, 
           (int)self.currentConfig.orientation, 
           (int)self.currentConfig.strategy);
    printf("Screen: %.0fx%.0f\n", 
           self.currentConfig.screenSize.width, 
           self.currentConfig.screenSize.height);
    printf("Emulator Area: %.0f,%.0f %.0fx%.0f\n",
           _calculatedEmulatorRect.origin.x, _calculatedEmulatorRect.origin.y,
           _calculatedEmulatorRect.size.width, _calculatedEmulatorRect.size.height);
    printf("Icon Size: %.3f, Spacing: %.3f\n",
           self.currentConfig.iconSize, self.currentConfig.iconSpacing);
    printf("=================================\n");
}

@end

// C++ wrapper implementations
void* icon_layout_manager_create(float width, float height) {
    @autoreleasepool {
        CGSize screenSize = CGSizeMake(width, height);
        CSCPIconLayoutManager *manager = [[CSCPIconLayoutManager alloc] initWithScreenSize:screenSize];
        return (__bridge_retained void*)manager;
    }
}

void icon_layout_manager_update_screen_size(void* manager, float width, float height) {
    @autoreleasepool {
        CSCPIconLayoutManager *layoutManager = (__bridge CSCPIconLayoutManager*)manager;
        [layoutManager updateLayoutForScreenSize:CGSizeMake(width, height)];
    }
}

int icon_layout_manager_get_strategy(void* manager) {
    @autoreleasepool {
        CSCPIconLayoutManager *layoutManager = (__bridge CSCPIconLayoutManager*)manager;
        return (int)layoutManager.currentConfig.strategy;
    }
}

void icon_layout_manager_get_emulator_area(void* manager, float* x, float* y, float* width, float* height) {
    @autoreleasepool {
        CSCPIconLayoutManager *layoutManager = (__bridge CSCPIconLayoutManager*)manager;
        CGRect rect = layoutManager.calculatedEmulatorRect;
        if (x) *x = rect.origin.x;
        if (y) *y = rect.origin.y;
        if (width) *width = rect.size.width;
        if (height) *height = rect.size.height;
    }
}

int icon_layout_manager_get_device_type(void* manager) {
    @autoreleasepool {
        CSCPIconLayoutManager *layoutManager = (__bridge CSCPIconLayoutManager*)manager;
        return (int)layoutManager.currentDeviceType;
    }
}

void icon_layout_manager_destroy(void* manager) {
    @autoreleasepool {
        CSCPIconLayoutManager *layoutManager = (__bridge_transfer CSCPIconLayoutManager*)manager;
        // ARC will handle cleanup
    }
}