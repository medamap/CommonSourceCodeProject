/*
    Icon Renderer for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.30
    
    [Icon display system for macOS/iOS/iPad]
*/

#ifndef _ICONRENDERER_H_
#define _ICONRENDERER_H_

#import <TargetConditionals.h>

#if TARGET_OS_OSX
    #import <Cocoa/Cocoa.h>
    #import <MetalKit/MetalKit.h>
#else
    #import <UIKit/UIKit.h>
    #import <MetalKit/MetalKit.h>
#endif

#import <Metal/Metal.h>
#import <simd/simd.h>

// Icon types (matching Android implementation)
#ifndef ICON_TYPE_DEFINED
#define ICON_TYPE_DEFINED
typedef NS_ENUM(NSInteger, IconType) {
    NONE_ICON = -1,
    SYSTEM_ICON = 0,
    FILE_ICON = 1,
    // NS_ENUM aliases for Objective-C compatibility
    IconTypeNone = NONE_ICON,
    IconTypeSystem = SYSTEM_ICON,
    IconTypeFile = FILE_ICON
};
#endif // ICON_TYPE_DEFINED

// System icon types
typedef NS_ENUM(NSInteger, SystemIconType) {
    SystemIconExit = 0,
    SystemIconReset,
    SystemIconSound,
    SystemIconPCG,
    SystemIconConfig,
    SystemIconKeyboard,
    SystemIconMouse,
    SystemIconWallpaper,
    SystemIconJoystick,
    SystemIconScreenshot,
    SystemIconMIDI,
    SystemIconColorBlind
};

// File icon types
typedef NS_ENUM(NSInteger, FileIconType) {
    FileIconFloppyDisk = 0,
    FileIconCassetteTape,
    FileIconCartridge,
    FileIconQuickDisk,
    FileIconHardDisk,
    FileIconCompactDisc,
    FileIconBubbleCassette,
    FileIconBinary
};

// Icon data structure
typedef struct {
    simd_float2 position;
    simd_float2 size;
    simd_float2 texCoord;
    int iconType;
    int iconIndex;
    int isPressed;
    float alpha;
} IconData;

// Icon layout configuration
typedef struct {
    // Screen orientation
    BOOL isLandscape;
    
    // Icon areas (normalized coordinates -1.0 to 1.0)
    simd_float4 fileIconArea;    // left, top, right, bottom
    simd_float4 systemIconArea;  // left, top, right, bottom
    simd_float4 progressArea;    // left, top, right, bottom
    
    // Icon sizes
    float iconSize;
    float iconSpacing;
    
    // Screen offsets for emulator area
    simd_float4 emulatorArea;    // left, top, right, bottom
} IconLayout;

// Icon renderer class
@interface CSCPIconRenderer : NSObject

// Metal objects
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLRenderPipelineState> iconPipelineState;
@property (nonatomic, strong) id<MTLRenderPipelineState> progressPipelineState;
@property (nonatomic, strong) id<MTLBuffer> iconVertexBuffer;
@property (nonatomic, strong) id<MTLBuffer> iconIndexBuffer;
@property (nonatomic, strong) id<MTLSamplerState> iconSamplerState;

// Icon textures
@property (nonatomic, strong) NSMutableDictionary<NSString*, id<MTLTexture>> *systemIconTextures;
@property (nonatomic, strong) NSMutableDictionary<NSString*, id<MTLTexture>> *fileIconTextures;

// Layout and data
@property (nonatomic) IconLayout iconLayout;
@property (nonatomic, strong) NSMutableArray<NSValue*> *systemIcons;
@property (nonatomic, strong) NSMutableArray<NSValue*> *fileIcons;

// Progress bar
@property (nonatomic) float progressValue;
@property (nonatomic) simd_float3 progressColor;
@property (nonatomic) BOOL progressVisible;

// Initialize with Metal device
- (instancetype)initWithDevice:(id<MTLDevice>)device;

// Setup icon system
- (BOOL)setupIconRenderer;

// Layout calculation
- (void)calculateLayoutForScreenSize:(CGSize)screenSize;
- (void)updateIconLayout:(BOOL)isLandscape;

// Icon management
- (void)addSystemIcon:(SystemIconType)iconType atIndex:(NSInteger)index;
- (void)addFileIcon:(FileIconType)iconType forDrive:(NSInteger)drive;
- (void)removeAllIcons;

// Icon interaction
- (BOOL)handleTouchAtPoint:(CGPoint)point;
- (NSInteger)getIconAtPoint:(CGPoint)point iconType:(IconType*)outType;

// Progress bar control
- (void)setProgressValue:(float)value;
- (void)setProgressColor:(simd_float3)color;
- (void)setProgressVisible:(BOOL)visible;

// Rendering
- (void)renderIconsWithEncoder:(id<MTLRenderCommandEncoder>)encoder
                    screenSize:(CGSize)screenSize;

// Texture loading
- (BOOL)loadSystemIconTextures;
- (BOOL)loadFileIconTextures;
- (id<MTLTexture>)createTextureFromImageNamed:(NSString*)imageName;

// Cleanup
- (void)cleanup;

@end

// C++ wrapper functions
#ifdef __cplusplus
extern "C" {
#endif

// Create icon renderer
void* icon_renderer_create(void* metalDevice);

// Setup and layout
int icon_renderer_setup(void* renderer);
void icon_renderer_calculate_layout(void* renderer, float width, float height);
void icon_renderer_update_layout(void* renderer, int isLandscape);

// Icon management
void icon_renderer_add_system_icon(void* renderer, int iconType, int index);
void icon_renderer_add_file_icon(void* renderer, int iconType, int drive);
void icon_renderer_remove_all_icons(void* renderer);

// Interaction
int icon_renderer_handle_touch(void* renderer, float x, float y);
int icon_renderer_get_icon_at_point(void* renderer, float x, float y, int* outType);

// Progress bar
void icon_renderer_set_progress_value(void* renderer, float value);
void icon_renderer_set_progress_color(void* renderer, float r, float g, float b);
void icon_renderer_set_progress_visible(void* renderer, int visible);

// Rendering
void icon_renderer_render(void* renderer, void* encoder, float width, float height);

// Cleanup
void icon_renderer_destroy(void* renderer);

#ifdef __cplusplus
}
#endif

#endif // _ICONRENDERER_H_