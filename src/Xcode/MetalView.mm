/*
    Metal View for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.29
    
    [Metal API implementation for macOS/iOS/iPad]
*/

#import "MetalView.h"
#include "../common.h"

// Shader effect types
typedef NS_ENUM(NSInteger, ShaderEffectType) {
    ShaderEffectNone = 0,
    ShaderEffectBlur = 1,
    ShaderEffectScanline = 2,
    ShaderEffectGreen = 3,
    ShaderEffectRGB565 = 4
};

// Uniforms structure matching Metal shader
typedef struct {
    simd_float2 screenSize;
    float time;
    int effectType;
    int colorBlindnessType;
    float blurRadius;
    float scanlineIntensity;
    float dotPattern;
} Uniforms;

// Constants
static const NSUInteger kMaxFramesInFlight = 3;

@implementation CSCPMetalView {
    dispatch_semaphore_t _inFlightSemaphore;
    id<MTLLibrary> _defaultLibrary;
    id<MTLSamplerState> _samplerState;
    
    // Multiple pipeline states for different effects
    id<MTLRenderPipelineState> _basicPipelineState;
    id<MTLRenderPipelineState> _blurPipelineState;
    id<MTLRenderPipelineState> _scanlinePipelineState;
    id<MTLRenderPipelineState> _greenPipelineState;
    id<MTLRenderPipelineState> _rgb565PipelineState;
    
    // Triple buffering for performance optimization
    NSUInteger _currentBufferIndex;
    id<MTLBuffer> _uniformsBuffers[kMaxFramesInFlight];
    
    // Performance optimization: texture pool for different sizes
    NSMutableDictionary<NSString*, id<MTLTexture>> *_texturePool;
    NSUInteger _maxTexturePoolSize;
    
    // Dirty region tracking for partial updates
    BOOL _fullTextureUpdateRequired;
    CGRect _dirtyRegion;
    
    // Performance metrics
    CFTimeInterval _lastFrameTime;
    NSUInteger _frameCount;
    double _averageFrameTime;
    
    // Current effect settings
    ShaderEffectType _currentEffect;
    int _colorBlindnessType;
    CFTimeInterval _startTime;
}

- (instancetype)initWithFrame:(CGRect)frame device:(nullable id<MTLDevice>)device {
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.device = device ?: MTLCreateSystemDefaultDevice();
        self.metalDevice = self.device;
        self.delegate = self;
        self.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.framebufferOnly = NO;
        self.paused = NO;  // エミュレーション開始時に自動的にレンダリング開始
        self.enableSetNeedsDisplay = NO;  // MTKViewは自動更新
        
        // Initialize texture format (16-bit RGB565 or 32-bit RGBA8888)
#if defined(_RGB565)
        self.textureFormat = MTLPixelFormatBGR5A1Unorm; // 最も近い16bitフォーマット
#else
        self.textureFormat = MTLPixelFormatBGRA8Unorm;
#endif
        
        _inFlightSemaphore = dispatch_semaphore_create(1);
        
        // Initialize effect settings
        _currentEffect = ShaderEffectNone;
        _colorBlindnessType = 0; // Normal vision
        _startTime = CACurrentMediaTime();
        
        // Initialize performance optimization settings
        _currentBufferIndex = 0;
        _texturePool = [[NSMutableDictionary alloc] init];
        _maxTexturePoolSize = 10; // Maximum number of cached textures
        _fullTextureUpdateRequired = YES;
        _dirtyRegion = CGRectZero;
        _lastFrameTime = CACurrentMediaTime();
        _frameCount = 0;
        _averageFrameTime = 0.0;
        
        // Initialize icon system
        self.iconsEnabled = YES;
        self.layoutManager = [[CSCPIconLayoutManager alloc] initWithScreenSize:frame.size];
        self.iconRenderer = [[CSCPIconRenderer alloc] initWithDevice:self.metalDevice];
        
        [self setupMetal];
    }
    return self;
}

- (void)setupMetal {
    NSError *error = nil;
    
    // Create command queue
    self.commandQueue = [self.metalDevice newCommandQueue];
    
    // Load Metal shaders from default library (compiled into app bundle)
    _defaultLibrary = [self.metalDevice newDefaultLibrary];
    
    if (!_defaultLibrary) {
        NSLog(@"Failed to create default shader library, creating from source");
        
        // Basic shader source as fallback
        NSString *shaderSource = @""
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "struct VertexIn {\n"
        "    float2 position [[attribute(0)]];\n"
        "    float2 texCoord [[attribute(1)]];\n"
        "};\n"
        "struct VertexOut {\n"
        "    float4 position [[position]];\n"
        "    float2 texCoord;\n"
        "};\n"
        "vertex VertexOut vertexShader(VertexIn in [[stage_in]]) {\n"
        "    VertexOut out;\n"
        "    out.position = float4(in.position, 0.0, 1.0);\n"
        "    out.texCoord = in.texCoord;\n"
        "    return out;\n"
        "}\n"
        "fragment float4 basicFragmentShader(VertexOut in [[stage_in]], texture2d<float> texture [[texture(0)]], sampler textureSampler [[sampler(0)]]) {\n"
        "    return texture.sample(textureSampler, in.texCoord);\n"
        "};";
        
        NSError *error = nil;
        _defaultLibrary = [self.metalDevice newLibraryWithSource:shaderSource options:nil error:&error];
        
        if (!_defaultLibrary) {
            NSLog(@"Failed to create shader library from source: %@", error.localizedDescription);
            return;
        }
        
        NSLog(@"Successfully created shader library from source");
    }
    
    // Create vertex descriptor
    MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = offsetof(Vertex, position);
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].offset = offsetof(Vertex, texCoord);
    vertexDescriptor.attributes[1].bufferIndex = 0;
    vertexDescriptor.layouts[0].stride = sizeof(Vertex);
    
    // Create multiple pipeline states for different effects
    [self createPipelineStates:vertexDescriptor];
    
    // Create multiple uniforms buffers for triple buffering
    for (NSUInteger i = 0; i < kMaxFramesInFlight; i++) {
        _uniformsBuffers[i] = [self.metalDevice newBufferWithLength:sizeof(Uniforms)
                                                            options:MTLResourceStorageModeShared];
    }
    
    // Setup icon renderer
    if (self.iconRenderer) {
        [self.iconRenderer setupIconRenderer];
    }
    
    // Create vertex buffer for full screen quad (fix upside-down display)
    Vertex vertices[] = {
        { { -1.0, -1.0 }, { 0.0, 0.0 } },  // Bottom left -> Top left texture
        { {  1.0, -1.0 }, { 1.0, 0.0 } },  // Bottom right -> Top right texture
        { { -1.0,  1.0 }, { 0.0, 1.0 } },  // Top left -> Bottom left texture
        { {  1.0,  1.0 }, { 1.0, 1.0 } },  // Top right -> Bottom right texture
    };
    
    self.vertexBuffer = [self.metalDevice newBufferWithBytes:vertices
                                                       length:sizeof(vertices)
                                                      options:MTLResourceStorageModeShared];
    
    // Create sampler state
    MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
    _samplerState = [self.metalDevice newSamplerStateWithDescriptor:samplerDescriptor];
}

- (void)createPipelineStates:(MTLVertexDescriptor*)vertexDescriptor {
    NSError *error = nil;
    
    // Basic shader pipeline
    MTLRenderPipelineDescriptor *basicDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    basicDescriptor.vertexFunction = [_defaultLibrary newFunctionWithName:@"vertexShader"];
    basicDescriptor.fragmentFunction = [_defaultLibrary newFunctionWithName:@"basicFragmentShader"];
    basicDescriptor.vertexDescriptor = vertexDescriptor;
    basicDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _basicPipelineState = [self.metalDevice newRenderPipelineStateWithDescriptor:basicDescriptor error:&error];
    if (error) {
        NSLog(@"Failed to create basic pipeline state: %@", error);
    }
    
    // Blur shader pipeline
    MTLRenderPipelineDescriptor *blurDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    blurDescriptor.vertexFunction = [_defaultLibrary newFunctionWithName:@"vertexShader"];
    blurDescriptor.fragmentFunction = [_defaultLibrary newFunctionWithName:@"blurFragmentShader"];
    blurDescriptor.vertexDescriptor = vertexDescriptor;
    blurDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _blurPipelineState = [self.metalDevice newRenderPipelineStateWithDescriptor:blurDescriptor error:&error];
    if (error) {
        NSLog(@"Failed to create blur pipeline state: %@", error);
    }
    
    // Scanline shader pipeline
    MTLRenderPipelineDescriptor *scanlineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    scanlineDescriptor.vertexFunction = [_defaultLibrary newFunctionWithName:@"vertexShader"];
    scanlineDescriptor.fragmentFunction = [_defaultLibrary newFunctionWithName:@"scanlineFragmentShader"];
    scanlineDescriptor.vertexDescriptor = vertexDescriptor;
    scanlineDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _scanlinePipelineState = [self.metalDevice newRenderPipelineStateWithDescriptor:scanlineDescriptor error:&error];
    if (error) {
        NSLog(@"Failed to create scanline pipeline state: %@", error);
    }
    
    // Green display shader pipeline
    MTLRenderPipelineDescriptor *greenDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    greenDescriptor.vertexFunction = [_defaultLibrary newFunctionWithName:@"vertexShader"];
    greenDescriptor.fragmentFunction = [_defaultLibrary newFunctionWithName:@"greenDisplayFragmentShader"];
    greenDescriptor.vertexDescriptor = vertexDescriptor;
    greenDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _greenPipelineState = [self.metalDevice newRenderPipelineStateWithDescriptor:greenDescriptor error:&error];
    if (error) {
        NSLog(@"Failed to create green display pipeline state: %@", error);
    }
    
    // RGB565 shader pipeline
    MTLRenderPipelineDescriptor *rgb565Descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    rgb565Descriptor.vertexFunction = [_defaultLibrary newFunctionWithName:@"vertexShader"];
    rgb565Descriptor.fragmentFunction = [_defaultLibrary newFunctionWithName:@"rgb565FragmentShader"];
    rgb565Descriptor.vertexDescriptor = vertexDescriptor;
    rgb565Descriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _rgb565PipelineState = [self.metalDevice newRenderPipelineStateWithDescriptor:rgb565Descriptor error:&error];
    if (error) {
        NSLog(@"Failed to create RGB565 pipeline state: %@", error);
    }
    
    // Set default pipeline state (fallback to basic if others fail)
    self.pipelineState = _basicPipelineState ?: _blurPipelineState ?: _scanlinePipelineState ?: _greenPipelineState;
}

- (void)updateScreenTexture:(const void*)buffer 
                      width:(NSInteger)width 
                     height:(NSInteger)height 
                bytesPerRow:(NSInteger)bytesPerRow {
    
    static int texture_update_count = 0;
    texture_update_count++;
    
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    // Get or create texture from pool for better performance
    self.screenTexture = [self getOrCreateTextureWithWidth:width height:height];
    
    if (!self.screenTexture) {
        printf("エラー: テクスチャ取得/作成失敗\n");
        dispatch_semaphore_signal(_inFlightSemaphore);
        return;
    }
    
    // Update texture data with optimized region updates
    if (buffer) {
        [self updateTextureRegion:self.screenTexture 
                       withBuffer:buffer 
                            width:width 
                           height:height 
                      bytesPerRow:bytesPerRow];
        
        if (texture_update_count % 100 == 0) {
            printf("テクスチャ更新: フレーム %d (最適化済み)\n", texture_update_count);
        }
    }
    
    dispatch_semaphore_signal(_inFlightSemaphore);
    
    // Reset dirty region tracking after update
    _fullTextureUpdateRequired = NO;
    _dirtyRegion = CGRectZero;
}

#pragma mark - MTKViewDelegate

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    // Handle view resize - update icon layout
    if (self.layoutManager) {
        [self.layoutManager updateLayoutForScreenSize:size];
        [self updateIconLayout];
    }
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    // Performance metrics tracking
    CFTimeInterval currentTime = CACurrentMediaTime();
    CFTimeInterval frameTime = currentTime - _lastFrameTime;
    _lastFrameTime = currentTime;
    _frameCount++;
    
    // Update running average frame time
    if (_frameCount == 1) {
        _averageFrameTime = frameTime;
    } else {
        _averageFrameTime = (_averageFrameTime * 0.9) + (frameTime * 0.1);
    }
    
    // Debug output every 5 seconds
    if (_frameCount % 300 == 0) {
        double fps = 1.0 / _averageFrameTime;
        printf("Performance: %.1f FPS (%.2fms), Frame %lu\n", 
               fps, _averageFrameTime * 1000.0, (unsigned long)_frameCount);
    }
    
    if (!self.screenTexture) {
        if (_frameCount % 100 == 0) {
            printf("drawInMTKView: テクスチャなし (フレーム %lu)\n", (unsigned long)_frameCount);
        }
        return;
    }
    
    // Use triple buffering for better performance
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    _currentBufferIndex = (_currentBufferIndex + 1) % kMaxFramesInFlight;
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    commandBuffer.label = @"CSCPRenderCommand";
    
    __block dispatch_semaphore_t blockSemaphore = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSemaphore);
    }];
    
    MTLRenderPassDescriptor *renderPassDescriptor = self.currentRenderPassDescriptor;
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"CSCPRenderEncoder";
        
        // Update uniforms using current buffer from triple buffer pool
        [self updateUniformsForCurrentFrame];
        
        // Select appropriate pipeline state based on current effect
        id<MTLRenderPipelineState> currentPipeline = [self currentPipelineState];
        
        [renderEncoder setRenderPipelineState:currentPipeline];
        [renderEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [renderEncoder setFragmentTexture:self.screenTexture atIndex:0];
        [renderEncoder setFragmentSamplerState:_samplerState atIndex:0];
        [renderEncoder setFragmentBuffer:_uniformsBuffers[_currentBufferIndex] offset:0 atIndex:0];
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                          vertexStart:0
                          vertexCount:4];
        
        // Render icons if enabled
        if (self.iconsEnabled && self.iconRenderer) {
            [self.iconRenderer renderIconsWithEncoder:renderEncoder 
                                           screenSize:CGSizeMake(self.drawableSize.width, self.drawableSize.height)];
        }
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:self.currentDrawable];
    } else {
        if (_frameCount % 100 == 0) {
            printf("drawInMTKView: renderPassDescriptor が nil\n");
        }
    }
    
    [commandBuffer commit];
}

- (void)updateUniformsForCurrentFrame {
    CFTimeInterval currentTime = CACurrentMediaTime();
    Uniforms uniforms;
    uniforms.time = (float)(currentTime - _startTime);
    uniforms.screenSize = simd_make_float2(self.drawableSize.width, self.drawableSize.height);
    uniforms.effectType = (int)_currentEffect;
    uniforms.colorBlindnessType = _colorBlindnessType;
    uniforms.blurRadius = 1.0;
    uniforms.scanlineIntensity = 0.8;
    uniforms.dotPattern = 0.0;
    
    // Copy uniforms to current frame's buffer (triple buffering)
    memcpy([_uniformsBuffers[_currentBufferIndex] contents], &uniforms, sizeof(Uniforms));
}

// Texture pool management for performance optimization
- (id<MTLTexture>)getOrCreateTextureWithWidth:(NSInteger)width height:(NSInteger)height {
    NSString *textureKey = [NSString stringWithFormat:@"%ldx%ld", (long)width, (long)height];
    
    // Check if we have a cached texture of this size
    id<MTLTexture> cachedTexture = _texturePool[textureKey];
    if (cachedTexture && 
        cachedTexture.width == width && 
        cachedTexture.height == height &&
        cachedTexture.pixelFormat == self.textureFormat) {
        return cachedTexture;
    }
    
    // Create new texture if not found or size changed
    MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
    textureDescriptor.pixelFormat = self.textureFormat;
    textureDescriptor.width = width;
    textureDescriptor.height = height;
    textureDescriptor.usage = MTLTextureUsageShaderRead;
    textureDescriptor.storageMode = MTLStorageModeShared; // Optimize for frequent updates
    
    id<MTLTexture> newTexture = [self.metalDevice newTextureWithDescriptor:textureDescriptor];
    
    if (newTexture) {
        // Cache the texture, but limit pool size
        if (_texturePool.count >= _maxTexturePoolSize) {
            // Remove oldest texture (simple FIFO)
            NSString *oldestKey = _texturePool.allKeys.firstObject;
            if (oldestKey) {
                [_texturePool removeObjectForKey:oldestKey];
            }
        }
        _texturePool[textureKey] = newTexture;
        
        // Update screen dimensions tracking
        self.screenWidth = width;
        self.screenHeight = height;
        _fullTextureUpdateRequired = YES;
        
        printf("テクスチャプール: 新規作成 %ldx%ld (プールサイズ: %lu)\n", 
               (long)width, (long)height, (unsigned long)_texturePool.count);
    }
    
    return newTexture;
}

// Optimized texture region update
- (void)updateTextureRegion:(id<MTLTexture>)texture 
                 withBuffer:(const void*)buffer 
                      width:(NSInteger)width 
                     height:(NSInteger)height 
                bytesPerRow:(NSInteger)bytesPerRow {
    
    if (_fullTextureUpdateRequired || CGRectEqualToRect(_dirtyRegion, CGRectZero)) {
        // Full texture update
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        [texture replaceRegion:region
                   mipmapLevel:0
                     withBytes:buffer
                   bytesPerRow:bytesPerRow];
    } else {
        // Partial update using dirty region (if implemented by caller)
        NSInteger dirtyX = (NSInteger)_dirtyRegion.origin.x;
        NSInteger dirtyY = (NSInteger)_dirtyRegion.origin.y;
        NSInteger dirtyWidth = (NSInteger)_dirtyRegion.size.width;
        NSInteger dirtyHeight = (NSInteger)_dirtyRegion.size.height;
        
        // Clamp dirty region to texture bounds
        dirtyX = MAX(0, MIN(dirtyX, width - 1));
        dirtyY = MAX(0, MIN(dirtyY, height - 1));
        dirtyWidth = MAX(1, MIN(dirtyWidth, width - dirtyX));
        dirtyHeight = MAX(1, MIN(dirtyHeight, height - dirtyY));
        
        MTLRegion region = MTLRegionMake2D(dirtyX, dirtyY, dirtyWidth, dirtyHeight);
        
        // Calculate offset into source buffer
        const uint8_t *sourceBytes = (const uint8_t*)buffer + (dirtyY * bytesPerRow) + (dirtyX * 4); // Assuming 4 bytes per pixel
        
        [texture replaceRegion:region
                   mipmapLevel:0
                     withBytes:sourceBytes
                   bytesPerRow:bytesPerRow];
    }
}

- (id<MTLRenderPipelineState>)currentPipelineState {
    switch (_currentEffect) {
        case ShaderEffectBlur:
            return _blurPipelineState ?: _basicPipelineState;
        case ShaderEffectScanline:
            return _scanlinePipelineState ?: _basicPipelineState;
        case ShaderEffectGreen:
            return _greenPipelineState ?: _basicPipelineState;
        case ShaderEffectRGB565:
            return _rgb565PipelineState ?: _basicPipelineState;
        case ShaderEffectNone:
        default:
            return _basicPipelineState ?: self.pipelineState;
    }
}

// Public methods for controlling shader effects
- (void)setShaderEffect:(NSInteger)effectType {
    _currentEffect = (ShaderEffectType)effectType;
}

- (void)setColorBlindnessType:(NSInteger)type {
    _colorBlindnessType = (int)type;
}

- (void)setBlurRadius:(float)radius {
    // Update all uniform buffers for consistency
    for (NSUInteger i = 0; i < kMaxFramesInFlight; i++) {
        Uniforms *uniformsPtr = (Uniforms*)[_uniformsBuffers[i] contents];
        uniformsPtr->blurRadius = radius;
    }
}

- (void)setScanlineIntensity:(float)intensity {
    // Update all uniform buffers for consistency
    for (NSUInteger i = 0; i < kMaxFramesInFlight; i++) {
        Uniforms *uniformsPtr = (Uniforms*)[_uniformsBuffers[i] contents];
        uniformsPtr->scanlineIntensity = intensity;
    }
}

- (void)setDotPattern:(float)pattern {
    // Update all uniform buffers for consistency
    for (NSUInteger i = 0; i < kMaxFramesInFlight; i++) {
        Uniforms *uniformsPtr = (Uniforms*)[_uniformsBuffers[i] contents];
        uniformsPtr->dotPattern = pattern;
    }
}

// Performance monitoring methods
- (double)getCurrentFPS {
    return (_averageFrameTime > 0) ? (1.0 / _averageFrameTime) : 0.0;
}

- (double)getAverageFrameTime {
    return _averageFrameTime * 1000.0; // Return in milliseconds
}

- (NSUInteger)getFrameCount {
    return _frameCount;
}

// Dirty region tracking for partial updates
- (void)markDirtyRegion:(CGRect)region {
    if (_fullTextureUpdateRequired) {
        return; // Already doing full update
    }
    
    if (CGRectEqualToRect(_dirtyRegion, CGRectZero)) {
        _dirtyRegion = region;
    } else {
        _dirtyRegion = CGRectUnion(_dirtyRegion, region);
    }
}

- (void)forceFullTextureUpdate {
    _fullTextureUpdateRequired = YES;
    _dirtyRegion = CGRectZero;
}

// Icon system methods
- (void)enableIcons:(BOOL)enabled {
    self.iconsEnabled = enabled;
    if (enabled && self.iconRenderer && self.layoutManager) {
        [self updateIconLayout];
    }
}

- (void)addSystemIcon:(SystemIconType)iconType {
    if (self.iconRenderer) {
        [self.iconRenderer addSystemIcon:iconType atIndex:self.iconRenderer.systemIcons.count];
        [self updateIconLayout];
    }
}

- (void)addFileIcon:(FileIconType)iconType forDrive:(NSInteger)drive {
    if (self.iconRenderer) {
        [self.iconRenderer addFileIcon:iconType forDrive:drive];
        [self updateIconLayout];
    }
}

- (void)removeAllIcons {
    if (self.iconRenderer) {
        [self.iconRenderer removeAllIcons];
    }
}

- (BOOL)handleIconTouch:(CGPoint)point {
    if (self.iconsEnabled && self.iconRenderer) {
        return [self.iconRenderer handleTouchAtPoint:point];
    }
    return NO;
}

- (void)updateIconLayout {
    if (self.layoutManager && self.iconRenderer) {
        [self.layoutManager updateLayoutForScreenSize:CGSizeMake(self.drawableSize.width, self.drawableSize.height)];
        [self.layoutManager positionIconsWithRenderer:self.iconRenderer];
    }
}


- (void)cleanup {
    self.screenTexture = nil;
    self.vertexBuffer = nil;
    self.pipelineState = nil;
    _basicPipelineState = nil;
    _blurPipelineState = nil;
    _scanlinePipelineState = nil;
    _greenPipelineState = nil;
    _rgb565PipelineState = nil;
    
    // Clean up triple buffering resources
    for (NSUInteger i = 0; i < kMaxFramesInFlight; i++) {
        _uniformsBuffers[i] = nil;
    }
    
    // Clear texture pool
    [_texturePool removeAllObjects];
    _texturePool = nil;
    
    // Cleanup icon system
    if (self.iconRenderer) {
        [self.iconRenderer cleanup];
        self.iconRenderer = nil;
    }
    self.layoutManager = nil;
    
    self.commandQueue = nil;
    self.metalDevice = nil;
    
    printf("MetalView cleanup完了: テクスチャプールクリア、アイコンシステムクリア\n");
}

@end

// C++ wrapper functions
void* metal_view_create(int x, int y, int width, int height) {
    @autoreleasepool {
        CGRect frame = CGRectMake(x, y, width, height);
        CSCPMetalView *view = [[CSCPMetalView alloc] initWithFrame:frame device:nil];
        return (__bridge_retained void*)view;
    }
}

void metal_view_update_screen(void* view, const void* buffer, int width, int height, int pitch) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        
        static int update_count = 0;
        update_count++;
        if (update_count % 100 == 0) {
            printf("metal_view_update_screen: フレーム %d, サイズ %dx%d, pitch %d\n", 
                   update_count, width, height, pitch);
        }
        
        [metalView updateScreenTexture:buffer width:width height:height bytesPerRow:pitch];
    }
}

void metal_view_destroy(void* view) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge_transfer CSCPMetalView*)view;
        [metalView cleanup];
    }
}

void* metal_view_get_native_handle(void* view) {
    CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
    return (__bridge void*)metalView;
}

void metal_view_set_shader_effect(void* view, int effectType) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView setShaderEffect:effectType];
    }
}

void metal_view_set_color_blindness_type(void* view, int type) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView setColorBlindnessType:type];
    }
}

void metal_view_set_blur_radius(void* view, float radius) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView setBlurRadius:radius];
    }
}

void metal_view_set_scanline_intensity(void* view, float intensity) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView setScanlineIntensity:intensity];
    }
}

void metal_view_set_dot_pattern(void* view, float pattern) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView setDotPattern:pattern];
    }
}

void metal_view_mark_dirty_region(void* view, int x, int y, int width, int height) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        CGRect region = CGRectMake(x, y, width, height);
        [metalView markDirtyRegion:region];
    }
}

void metal_view_force_full_texture_update(void* view) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        [metalView forceFullTextureUpdate];
    }
}

double metal_view_get_current_fps(void* view) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        return [metalView getCurrentFPS];
    }
}

double metal_view_get_average_frame_time(void* view) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        return [metalView getAverageFrameTime];
    }
}

unsigned long metal_view_get_frame_count(void* view) {
    @autoreleasepool {
        CSCPMetalView *metalView = (__bridge CSCPMetalView*)view;
        return (unsigned long)[metalView getFrameCount];
    }
}