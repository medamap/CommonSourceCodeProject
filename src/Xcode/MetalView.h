/*
    Metal View for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [Metal API implementation for macOS/iOS/iPad]
*/

#ifndef _METALVIEW_H_
#define _METALVIEW_H_

#import <TargetConditionals.h>

#if TARGET_OS_OSX
    #import <Cocoa/Cocoa.h>
    #import <MetalKit/MetalKit.h>
    #define PlatformView NSView
#else
    #import <UIKit/UIKit.h>
    #import <MetalKit/MetalKit.h>
    #define PlatformView UIView
#endif

#import <Metal/Metal.h>
#import <simd/simd.h>
#import "IconRenderer.h"
#import "IconLayoutManager.h"

// Vertex structure for screen quad
typedef struct {
    vector_float2 position;
    vector_float2 texCoord;
} Vertex;

// Metal view class for rendering emulator screen
@interface CSCPMetalView : MTKView <MTKViewDelegate>

// Metal objects
@property (nonatomic, strong) id<MTLDevice> metalDevice;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLTexture> screenTexture;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;

// Screen properties
@property (nonatomic) NSInteger screenWidth;
@property (nonatomic) NSInteger screenHeight;
@property (nonatomic) MTLPixelFormat textureFormat;

// Icon system
@property (nonatomic, strong) CSCPIconRenderer *iconRenderer;
@property (nonatomic, strong) CSCPIconLayoutManager *layoutManager;
@property (nonatomic) BOOL iconsEnabled;

// Initialize with frame
- (instancetype)initWithFrame:(CGRect)frame device:(nullable id<MTLDevice>)device;

// Update screen texture with emulator buffer
- (void)updateScreenTexture:(const void*)buffer 
                      width:(NSInteger)width 
                     height:(NSInteger)height 
                bytesPerRow:(NSInteger)bytesPerRow;

// Setup Metal pipeline
- (void)setupMetal;

// Shader effect control methods
- (void)setShaderEffect:(NSInteger)effectType;
- (void)setColorBlindnessType:(NSInteger)type;
- (void)setBlurRadius:(float)radius;
- (void)setScanlineIntensity:(float)intensity;
- (void)setDotPattern:(float)pattern;

// Performance optimization methods
- (void)markDirtyRegion:(CGRect)region;
- (void)forceFullTextureUpdate;

// Performance monitoring methods
- (double)getCurrentFPS;
- (double)getAverageFrameTime;
- (NSUInteger)getFrameCount;

// Icon system methods
- (void)enableIcons:(BOOL)enabled;
- (void)addSystemIcon:(SystemIconType)iconType;
- (void)addFileIcon:(FileIconType)iconType forDrive:(NSInteger)drive;
- (void)removeAllIcons;
- (BOOL)handleIconTouch:(CGPoint)point;
- (void)updateIconLayout;

// Cleanup resources
- (void)cleanup;

@end

// C++ wrapper for MetalView
#ifdef __cplusplus
extern "C" {
#endif

// Create Metal view
void* metal_view_create(int x, int y, int width, int height);

// Update screen buffer
void metal_view_update_screen(void* view, const void* buffer, int width, int height, int pitch);

// Destroy Metal view
void metal_view_destroy(void* view);

// Get native view handle
void* metal_view_get_native_handle(void* view);

// Shader effect control functions
void metal_view_set_shader_effect(void* view, int effectType);
void metal_view_set_color_blindness_type(void* view, int type);
void metal_view_set_blur_radius(void* view, float radius);
void metal_view_set_scanline_intensity(void* view, float intensity);
void metal_view_set_dot_pattern(void* view, float pattern);

// Performance optimization functions
void metal_view_mark_dirty_region(void* view, int x, int y, int width, int height);
void metal_view_force_full_texture_update(void* view);

// Performance monitoring functions
double metal_view_get_current_fps(void* view);
double metal_view_get_average_frame_time(void* view);
unsigned long metal_view_get_frame_count(void* view);

#ifdef __cplusplus
}
#endif

#endif // _METALVIEW_H_