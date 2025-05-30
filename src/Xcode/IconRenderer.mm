/*
    Icon Renderer for CSCP Emulator
    
    Author : Medamap & Claude
    Date   : 2025.05.30
    
    [Icon display system for macOS/iOS/iPad]
*/

#import "IconRenderer.h"
#include "../common.h"

// Icon vertex structure
typedef struct {
    simd_float2 position;
    simd_float2 texCoord;
    float alpha;
} IconVertex;

// Icon uniforms
typedef struct {
    simd_float2 screenSize;
    simd_float4x4 projectionMatrix;
    simd_float3 tintColor;
    float globalAlpha;
} IconUniforms;

@implementation CSCPIconRenderer {
    id<MTLBuffer> _iconUniformsBuffer;
    IconUniforms _iconUniforms;
    
    // Icon data storage
    NSMutableArray<NSValue*> *_iconDataArray;
    NSUInteger _maxIcons;
    
    // Touch interaction
    CGPoint _lastTouchPoint;
    NSInteger _pressedIconIndex;
    
    // Progress bar properties
    float _progressValue;
    simd_float3 _progressColor;
    BOOL _progressVisible;
    
    // Default icon settings
    float _defaultIconSize;
    float _defaultIconSpacing;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device {
    self = [super init];
    if (self) {
        self.device = device;
        
        // Initialize arrays
        self.systemIconTextures = [[NSMutableDictionary alloc] init];
        self.fileIconTextures = [[NSMutableDictionary alloc] init];
        self.systemIcons = [[NSMutableArray alloc] init];
        self.fileIcons = [[NSMutableArray alloc] init];
        _iconDataArray = [[NSMutableArray alloc] init];
        
        // Default settings
        _maxIcons = 50; // Maximum number of icons
        _defaultIconSize = 0.08f; // 8% of screen width/height
        _defaultIconSpacing = 0.01f; // 1% spacing
        _pressedIconIndex = -1;
        
        // Progress bar defaults
        _progressValue = 0.0f;
        _progressColor = simd_make_float3(0.0f, 1.0f, 0.0f); // Green
        _progressVisible = NO;
        
        // Initialize layout
        [self initializeDefaultLayout];
    }
    return self;
}

- (void)initializeDefaultLayout {
    // Default layout for landscape orientation
    IconLayout layout;
    layout.isLandscape = YES;
    layout.iconSize = _defaultIconSize;
    layout.iconSpacing = _defaultIconSpacing;
    
    // File icons on the left
    layout.fileIconArea = simd_make_float4(-1.0f, -1.0f, -0.8f, 1.0f);
    
    // System icons on the right
    layout.systemIconArea = simd_make_float4(0.8f, -1.0f, 1.0f, 1.0f);
    
    // Progress bar at the top
    layout.progressArea = simd_make_float4(-0.8f, 0.9f, 0.8f, 1.0f);
    
    // Emulator area in the center
    layout.emulatorArea = simd_make_float4(-0.8f, -1.0f, 0.8f, 0.9f);
    
    // Set the layout property
    self.iconLayout = layout;
}

- (BOOL)setupIconRenderer {
    NSError *error = nil;
    
    // Create uniforms buffer
    _iconUniformsBuffer = [self.device newBufferWithLength:sizeof(IconUniforms)
                                                   options:MTLResourceStorageModeShared];
    
    // Load default Metal library
    id<MTLLibrary> defaultLibrary = [self.device newDefaultLibrary];
    if (!defaultLibrary) {
        NSLog(@"Failed to create default library for icons, creating from source");
        
        // Basic icon shader source as fallback
        NSString *iconShaderSource = @""
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "struct IconVertexIn {\n"
        "    float2 position [[attribute(0)]];\n"
        "    float2 texCoord [[attribute(1)]];\n"
        "    float alpha [[attribute(2)]];\n"
        "};\n"
        "struct IconVertexOut {\n"
        "    float4 position [[position]];\n"
        "    float2 texCoord;\n"
        "    float alpha;\n"
        "};\n"
        "struct IconUniforms {\n"
        "    float2 screenSize;\n"
        "    float4x4 projectionMatrix;\n"
        "    float3 tintColor;\n"
        "    float globalAlpha;\n"
        "};\n"
        "vertex IconVertexOut iconVertexShader(IconVertexIn in [[stage_in]], constant IconUniforms& uniforms [[buffer(1)]], constant float4x4& modelMatrix [[buffer(2)]]) {\n"
        "    IconVertexOut out;\n"
        "    float4 worldPosition = modelMatrix * float4(in.position, 0.0, 1.0);\n"
        "    out.position = uniforms.projectionMatrix * worldPosition;\n"
        "    out.texCoord = in.texCoord;\n"
        "    out.alpha = in.alpha * uniforms.globalAlpha;\n"
        "    return out;\n"
        "}\n"
        "fragment float4 iconFragmentShader(IconVertexOut in [[stage_in]], texture2d<float> iconTexture [[texture(0)]], sampler iconSampler [[sampler(0)]], constant IconUniforms& uniforms [[buffer(1)]]) {\n"
        "    float4 color = iconTexture.sample(iconSampler, in.texCoord);\n"
        "    color.rgb *= uniforms.tintColor;\n"
        "    color.a *= in.alpha;\n"
        "    return color;\n"
        "}\n"
        "fragment float4 progressFragmentShader(IconVertexOut in [[stage_in]], constant IconUniforms& uniforms [[buffer(1)]], constant float& progressValue [[buffer(3)]]) {\n"
        "    float progress = progressValue;\n"
        "    float4 color = (in.texCoord.x <= progress) ? float4(uniforms.tintColor, 1.0) : float4(uniforms.tintColor * 0.3, 0.5);\n"
        "    color.a *= in.alpha;\n"
        "    return color;\n"
        "};";
        
        NSError *error = nil;
        defaultLibrary = [self.device newLibraryWithSource:iconShaderSource options:nil error:&error];
        
        if (!defaultLibrary) {
            NSLog(@"Failed to create icon shader library from source: %@", error.localizedDescription);
            return NO;
        }
        
        NSLog(@"Successfully created icon shader library from source");
    }
    
    // Create icon pipeline state
    MTLRenderPipelineDescriptor *iconDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    iconDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"iconVertexShader"];
    iconDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"iconFragmentShader"];
    iconDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Enable alpha blending for icons
    iconDescriptor.colorAttachments[0].blendingEnabled = YES;
    iconDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    iconDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    iconDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    iconDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    
    self.iconPipelineState = [self.device newRenderPipelineStateWithDescriptor:iconDescriptor
                                                                        error:&error];
    if (error) {
        NSLog(@"Failed to create icon pipeline state: %@", error);
        return NO;
    }
    
    // Create progress bar pipeline state
    MTLRenderPipelineDescriptor *progressDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    progressDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"progressVertexShader"];
    progressDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"progressFragmentShader"];
    progressDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    progressDescriptor.colorAttachments[0].blendingEnabled = YES;
    progressDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    progressDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    
    self.progressPipelineState = [self.device newRenderPipelineStateWithDescriptor:progressDescriptor
                                                                            error:&error];
    if (error) {
        NSLog(@"Failed to create progress pipeline state: %@", error);
        return NO;
    }
    
    // Create vertex buffer for icon quads
    [self createIconVertexBuffer];
    
    // Create sampler state
    MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
    self.iconSamplerState = [self.device newSamplerStateWithDescriptor:samplerDescriptor];
    
    // Load icon textures
    [self loadSystemIconTextures];
    [self loadFileIconTextures];
    
    printf("Icon renderer setup completed\n");
    return YES;
}

- (void)createIconVertexBuffer {
    // Create vertices for a unit quad (-0.5 to 0.5)
    IconVertex vertices[] = {
        { { -0.5f, -0.5f }, { 0.0f, 1.0f }, 1.0f }, // Bottom left
        { {  0.5f, -0.5f }, { 1.0f, 1.0f }, 1.0f }, // Bottom right
        { { -0.5f,  0.5f }, { 0.0f, 0.0f }, 1.0f }, // Top left
        { {  0.5f,  0.5f }, { 1.0f, 0.0f }, 1.0f }, // Top right
    };
    
    self.iconVertexBuffer = [self.device newBufferWithBytes:vertices
                                                     length:sizeof(vertices)
                                                    options:MTLResourceStorageModeShared];
    
    // Create index buffer for quad
    uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
    self.iconIndexBuffer = [self.device newBufferWithBytes:indices
                                                    length:sizeof(indices)
                                                   options:MTLResourceStorageModeShared];
}

- (void)calculateLayoutForScreenSize:(CGSize)screenSize {
    BOOL isLandscape = screenSize.width > screenSize.height;
    IconLayout layout = self.iconLayout;
    
    if (isLandscape) {
        // Landscape layout: file icons left, system icons right
        float iconAreaWidth = 0.15f; // 15% of screen width for each side
        
        layout.fileIconArea = simd_make_float4(-1.0f, -1.0f, -1.0f + iconAreaWidth, 1.0f);
        layout.systemIconArea = simd_make_float4(1.0f - iconAreaWidth, -1.0f, 1.0f, 1.0f);
        layout.progressArea = simd_make_float4(-1.0f + iconAreaWidth, 0.85f, 1.0f - iconAreaWidth, 1.0f);
        layout.emulatorArea = simd_make_float4(-1.0f + iconAreaWidth, -1.0f, 1.0f - iconAreaWidth, 0.85f);
    } else {
        // Portrait layout: file icons top, system icons bottom
        float iconAreaHeight = 0.15f; // 15% of screen height for each area
        
        layout.fileIconArea = simd_make_float4(-1.0f, 1.0f - iconAreaHeight, 1.0f, 1.0f);
        layout.systemIconArea = simd_make_float4(-1.0f, -1.0f, 1.0f, -1.0f + iconAreaHeight);
        layout.progressArea = simd_make_float4(-1.0f, 1.0f - iconAreaHeight - 0.05f, 1.0f, 1.0f - iconAreaHeight);
        layout.emulatorArea = simd_make_float4(-1.0f, -1.0f + iconAreaHeight, 1.0f, 1.0f - iconAreaHeight - 0.05f);
    }
    
    layout.isLandscape = isLandscape;
    self.iconLayout = layout;
    
    // Update icon positions
    [self updateIconPositions];
    
    printf("Icon layout calculated for %s: %.0fx%.0f\n", 
           isLandscape ? "landscape" : "portrait", screenSize.width, screenSize.height);
}

- (void)updateIconPositions {
    // Update system icons positions
    [self layoutIconsInArea:self.iconLayout.systemIconArea
                  iconArray:self.systemIcons
                 maxColumns:self.iconLayout.isLandscape ? 1 : 6];
    
    // Update file icons positions
    [self layoutIconsInArea:self.iconLayout.fileIconArea
                  iconArray:self.fileIcons
                 maxColumns:self.iconLayout.isLandscape ? 1 : 6];
}

- (void)layoutIconsInArea:(simd_float4)area
                iconArray:(NSMutableArray*)iconArray
               maxColumns:(NSInteger)maxColumns {
    
    float areaWidth = area.z - area.x;
    float areaHeight = area.w - area.y;
    float iconSize = self.iconLayout.iconSize;
    float spacing = self.iconLayout.iconSpacing;
    
    NSInteger iconCount = iconArray.count;
    if (iconCount == 0) return;
    
    NSInteger columns = MIN(maxColumns, iconCount);
    NSInteger rows = (iconCount + columns - 1) / columns;
    
    float totalWidth = columns * iconSize + (columns - 1) * spacing;
    float totalHeight = rows * iconSize + (rows - 1) * spacing;
    
    float startX = area.x + (areaWidth - totalWidth) * 0.5f;
    float startY = area.y + (areaHeight - totalHeight) * 0.5f;
    
    for (NSInteger i = 0; i < iconCount; i++) {
        NSInteger row = i / columns;
        NSInteger col = i % columns;
        
        float x = startX + col * (iconSize + spacing) + iconSize * 0.5f;
        float y = startY + row * (iconSize + spacing) + iconSize * 0.5f;
        
        // Update icon position (assuming iconArray contains IconData wrapped in NSValue)
        NSValue *iconValue = iconArray[i];
        IconData iconData;
        [iconValue getValue:&iconData];
        iconData.position = simd_make_float2(x, y);
        iconData.size = simd_make_float2(iconSize, iconSize);
        iconArray[i] = [NSValue valueWithBytes:&iconData objCType:@encode(IconData)];
    }
}

- (void)addSystemIcon:(SystemIconType)iconType atIndex:(NSInteger)index {
    IconData iconData;
    iconData.position = simd_make_float2(0.0f, 0.0f); // Will be updated by layout
    iconData.size = simd_make_float2(self.iconLayout.iconSize, self.iconLayout.iconSize);
    iconData.texCoord = simd_make_float2(0.0f, 0.0f);
    iconData.iconType = IconTypeSystem;
    iconData.iconIndex = (int)iconType;
    iconData.isPressed = 0;
    iconData.alpha = 1.0f;
    
    [self.systemIcons addObject:[NSValue valueWithBytes:&iconData objCType:@encode(IconData)]];
    [self updateIconPositions];
    
    printf("Added system icon type %d at index %ld\n", (int)iconType, (long)index);
}

- (void)addFileIcon:(FileIconType)iconType forDrive:(NSInteger)drive {
    IconData iconData;
    iconData.position = simd_make_float2(0.0f, 0.0f); // Will be updated by layout
    iconData.size = simd_make_float2(self.iconLayout.iconSize, self.iconLayout.iconSize);
    iconData.texCoord = simd_make_float2(0.0f, 0.0f);
    iconData.iconType = IconTypeFile;
    iconData.iconIndex = (int)iconType;
    iconData.isPressed = 0;
    iconData.alpha = 1.0f;
    
    [self.fileIcons addObject:[NSValue valueWithBytes:&iconData objCType:@encode(IconData)]];
    [self updateIconPositions];
    
    printf("Added file icon type %d for drive %ld\n", (int)iconType, (long)drive);
}

- (void)removeAllIcons {
    [self.systemIcons removeAllObjects];
    [self.fileIcons removeAllObjects];
    _pressedIconIndex = -1;
    
    printf("All icons removed\n");
}

- (BOOL)handleTouchAtPoint:(CGPoint)point {
    // Convert touch point to normalized coordinates (-1.0 to 1.0)
    CGSize screenSize = CGSizeMake(2.0f, 2.0f); // Assuming normalized coordinates
    float normalizedX = (point.x / screenSize.width) * 2.0f - 1.0f;
    float normalizedY = (point.y / screenSize.height) * 2.0f - 1.0f;
    
    _lastTouchPoint = CGPointMake(normalizedX, normalizedY);
    
    // Check system icons
    for (NSInteger i = 0; i < self.systemIcons.count; i++) {
        NSValue *iconValue = self.systemIcons[i];
        IconData iconData;
        [iconValue getValue:&iconData];
        
        if ([self isPoint:_lastTouchPoint insideIcon:iconData]) {
            _pressedIconIndex = i;
            iconData.isPressed = 1;
            self.systemIcons[i] = [NSValue valueWithBytes:&iconData objCType:@encode(IconData)];
            
            printf("System icon %d pressed\n", iconData.iconIndex);
            return YES;
        }
    }
    
    // Check file icons
    for (NSInteger i = 0; i < self.fileIcons.count; i++) {
        NSValue *iconValue = self.fileIcons[i];
        IconData iconData;
        [iconValue getValue:&iconData];
        
        if ([self isPoint:_lastTouchPoint insideIcon:iconData]) {
            _pressedIconIndex = i + self.systemIcons.count;
            iconData.isPressed = 1;
            self.fileIcons[i] = [NSValue valueWithBytes:&iconData objCType:@encode(IconData)];
            
            printf("File icon %d pressed\n", iconData.iconIndex);
            return YES;
        }
    }
    
    return NO;
}

- (BOOL)isPoint:(CGPoint)point insideIcon:(IconData)iconData {
    float halfWidth = iconData.size.x * 0.5f;
    float halfHeight = iconData.size.y * 0.5f;
    
    return (point.x >= iconData.position.x - halfWidth &&
            point.x <= iconData.position.x + halfWidth &&
            point.y >= iconData.position.y - halfHeight &&
            point.y <= iconData.position.y + halfHeight);
}

- (NSInteger)getIconAtPoint:(CGPoint)point iconType:(IconType*)outType {
    // Convert touch point to normalized coordinates
    float normalizedX = (point.x / 2.0f) - 1.0f;
    float normalizedY = (point.y / 2.0f) - 1.0f;
    CGPoint normalizedPoint = CGPointMake(normalizedX, normalizedY);
    
    // Check system icons first
    for (NSInteger i = 0; i < self.systemIcons.count; i++) {
        NSValue *iconValue = self.systemIcons[i];
        IconData iconData;
        [iconValue getValue:&iconData];
        
        if ([self isPoint:normalizedPoint insideIcon:iconData]) {
            if (outType) *outType = IconTypeSystem;
            return iconData.iconIndex;
        }
    }
    
    // Check file icons
    for (NSInteger i = 0; i < self.fileIcons.count; i++) {
        NSValue *iconValue = self.fileIcons[i];
        IconData iconData;
        [iconValue getValue:&iconData];
        
        if ([self isPoint:normalizedPoint insideIcon:iconData]) {
            if (outType) *outType = IconTypeFile;
            return iconData.iconIndex;
        }
    }
    
    if (outType) *outType = IconTypeNone;
    return -1;
}

- (void)renderIconsWithEncoder:(id<MTLRenderCommandEncoder>)encoder
                    screenSize:(CGSize)screenSize {
    
    // Update uniforms
    _iconUniforms.screenSize = simd_make_float2(screenSize.width, screenSize.height);
    _iconUniforms.projectionMatrix = matrix_identity_float4x4;
    _iconUniforms.tintColor = simd_make_float3(1.0f, 1.0f, 1.0f);
    _iconUniforms.globalAlpha = 1.0f;
    
    memcpy([_iconUniformsBuffer contents], &_iconUniforms, sizeof(IconUniforms));
    
    // Render system icons
    [self renderIconArray:self.systemIcons 
              withEncoder:encoder 
               pipelineState:self.iconPipelineState];
    
    // Render file icons
    [self renderIconArray:self.fileIcons 
              withEncoder:encoder 
               pipelineState:self.iconPipelineState];
    
    // Render progress bar if visible
    if (self.progressVisible) {
        [self renderProgressBarWithEncoder:encoder];
    }
}

- (void)renderIconArray:(NSMutableArray*)iconArray
            withEncoder:(id<MTLRenderCommandEncoder>)encoder
         pipelineState:(id<MTLRenderPipelineState>)pipelineState {
    
    if (iconArray.count == 0) return;
    
    [encoder setRenderPipelineState:pipelineState];
    [encoder setVertexBuffer:self.iconVertexBuffer offset:0 atIndex:0];
    [encoder setVertexBuffer:_iconUniformsBuffer offset:0 atIndex:1];
    [encoder setFragmentBuffer:_iconUniformsBuffer offset:0 atIndex:1];
    [encoder setFragmentSamplerState:self.iconSamplerState atIndex:0];
    
    for (NSValue *iconValue in iconArray) {
        IconData iconData;
        [iconValue getValue:&iconData];
        
        // Create model matrix for this icon
        simd_float4x4 modelMatrix = matrix_identity_float4x4;
        modelMatrix.columns[3].x = iconData.position.x;
        modelMatrix.columns[3].y = iconData.position.y;
        modelMatrix.columns[0].x = iconData.size.x;
        modelMatrix.columns[1].y = iconData.size.y;
        
        [encoder setVertexBytes:&modelMatrix length:sizeof(simd_float4x4) atIndex:2];
        
        // Get appropriate texture
        id<MTLTexture> iconTexture = [self getTextureForIcon:iconData];
        if (iconTexture) {
            [encoder setFragmentTexture:iconTexture atIndex:0];
        }
        
        // Draw the icon
        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:6
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:self.iconIndexBuffer
                     indexBufferOffset:0];
    }
}

- (void)renderProgressBarWithEncoder:(id<MTLRenderCommandEncoder>)encoder {
    [encoder setRenderPipelineState:self.progressPipelineState];
    [encoder setVertexBuffer:self.iconVertexBuffer offset:0 atIndex:0];
    [encoder setVertexBuffer:_iconUniformsBuffer offset:0 atIndex:1];
    [encoder setFragmentBuffer:_iconUniformsBuffer offset:0 atIndex:1];
    
    // Update progress color
    _iconUniforms.tintColor = self.progressColor;
    memcpy([_iconUniformsBuffer contents], &_iconUniforms, sizeof(IconUniforms));
    
    // Create model matrix for progress bar
    simd_float4x4 modelMatrix = matrix_identity_float4x4;
    float barWidth = self.iconLayout.progressArea.z - self.iconLayout.progressArea.x;
    float barHeight = self.iconLayout.progressArea.w - self.iconLayout.progressArea.y;
    float barX = (self.iconLayout.progressArea.x + self.iconLayout.progressArea.z) * 0.5f;
    float barY = (self.iconLayout.progressArea.y + self.iconLayout.progressArea.w) * 0.5f;
    
    modelMatrix.columns[3].x = barX;
    modelMatrix.columns[3].y = barY;
    modelMatrix.columns[0].x = barWidth;
    modelMatrix.columns[1].y = barHeight;
    
    [encoder setVertexBytes:&modelMatrix length:sizeof(simd_float4x4) atIndex:2];
    float progressVal = self.progressValue;
    [encoder setFragmentBytes:&progressVal length:sizeof(float) atIndex:3];
    
    // Draw the progress bar
    [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:6
                         indexType:MTLIndexTypeUInt16
                       indexBuffer:self.iconIndexBuffer
                 indexBufferOffset:0];
}

- (id<MTLTexture>)getTextureForIcon:(IconData)iconData {
    NSString *textureKey;
    
    if (iconData.iconType == IconTypeSystem) {
        textureKey = [NSString stringWithFormat:@"system_%d", iconData.iconIndex];
        return self.systemIconTextures[textureKey];
    } else if (iconData.iconType == IconTypeFile) {
        textureKey = [NSString stringWithFormat:@"file_%d", iconData.iconIndex];
        return self.fileIconTextures[textureKey];
    }
    
    return nil;
}

- (BOOL)loadSystemIconTextures {
    // Create placeholder textures for system icons
    for (int i = 0; i < 12; i++) {
        NSString *textureKey = [NSString stringWithFormat:@"system_%d", i];
        id<MTLTexture> texture = [self createPlaceholderTextureWithColor:simd_make_float3(0.8f, 0.8f, 1.0f)];
        if (texture) {
            self.systemIconTextures[textureKey] = texture;
        }
    }
    
    printf("Loaded %lu system icon textures\n", (unsigned long)self.systemIconTextures.count);
    return YES;
}

- (BOOL)loadFileIconTextures {
    // Create placeholder textures for file icons
    simd_float3 color1 = simd_make_float3(1.0f, 0.8f, 0.8f);
    simd_float3 color2 = simd_make_float3(0.8f, 1.0f, 0.8f);
    simd_float3 color3 = simd_make_float3(0.8f, 0.8f, 1.0f);
    simd_float3 color4 = simd_make_float3(1.0f, 1.0f, 0.8f);
    simd_float3 color5 = simd_make_float3(1.0f, 0.8f, 1.0f);
    simd_float3 color6 = simd_make_float3(0.8f, 1.0f, 1.0f);
    simd_float3 color7 = simd_make_float3(1.0f, 0.9f, 0.8f);
    simd_float3 color8 = simd_make_float3(0.9f, 0.8f, 1.0f);
    
    NSArray *colors = @[
        [NSValue valueWithBytes:&color1 objCType:@encode(simd_float3)], // Red
        [NSValue valueWithBytes:&color2 objCType:@encode(simd_float3)], // Green
        [NSValue valueWithBytes:&color3 objCType:@encode(simd_float3)], // Blue
        [NSValue valueWithBytes:&color4 objCType:@encode(simd_float3)], // Yellow
        [NSValue valueWithBytes:&color5 objCType:@encode(simd_float3)], // Magenta
        [NSValue valueWithBytes:&color6 objCType:@encode(simd_float3)], // Cyan
        [NSValue valueWithBytes:&color7 objCType:@encode(simd_float3)], // Orange
        [NSValue valueWithBytes:&color8 objCType:@encode(simd_float3)]  // Purple
    ];
    
    for (int i = 0; i < 8; i++) {
        NSString *textureKey = [NSString stringWithFormat:@"file_%d", i];
        simd_float3 color;
        [colors[i] getValue:&color];
        id<MTLTexture> texture = [self createPlaceholderTextureWithColor:color];
        if (texture) {
            self.fileIconTextures[textureKey] = texture;
        }
    }
    
    printf("Loaded %lu file icon textures\n", (unsigned long)self.fileIconTextures.count);
    return YES;
}

- (id<MTLTexture>)createPlaceholderTextureWithColor:(simd_float3)color {
    const int size = 64;
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = size * bytesPerPixel;
    NSUInteger imageSize = bytesPerRow * size;
    
    uint8_t *imageData = (uint8_t*)malloc(imageSize);
    if (!imageData) return nil;
    
    // Create a simple colored square with border
    uint8_t r = (uint8_t)(color.x * 255);
    uint8_t g = (uint8_t)(color.y * 255);
    uint8_t b = (uint8_t)(color.z * 255);
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            NSUInteger offset = (y * size + x) * bytesPerPixel;
            
            // Create border effect
            if (x < 4 || x >= size-4 || y < 4 || y >= size-4) {
                imageData[offset + 0] = 255; // White border
                imageData[offset + 1] = 255;
                imageData[offset + 2] = 255;
            } else {
                imageData[offset + 0] = r;
                imageData[offset + 1] = g;
                imageData[offset + 2] = b;
            }
            imageData[offset + 3] = 255; // Alpha
        }
    }
    
    MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
    textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
    textureDescriptor.width = size;
    textureDescriptor.height = size;
    textureDescriptor.usage = MTLTextureUsageShaderRead;
    
    id<MTLTexture> texture = [self.device newTextureWithDescriptor:textureDescriptor];
    if (texture) {
        MTLRegion region = MTLRegionMake2D(0, 0, size, size);
        [texture replaceRegion:region mipmapLevel:0 withBytes:imageData bytesPerRow:bytesPerRow];
    }
    
    free(imageData);
    return texture;
}

- (void)setProgressValue:(float)value {
    _progressValue = fmaxf(0.0f, fminf(1.0f, value));
}

- (float)progressValue {
    return _progressValue;
}

- (void)setProgressColor:(simd_float3)color {
    _progressColor = color;
}

- (simd_float3)progressColor {
    return _progressColor;
}

- (void)setProgressVisible:(BOOL)visible {
    _progressVisible = visible;
}

- (BOOL)progressVisible {
    return _progressVisible;
}

- (void)cleanup {
    [self.systemIconTextures removeAllObjects];
    [self.fileIconTextures removeAllObjects];
    [self removeAllIcons];
    
    self.iconPipelineState = nil;
    self.progressPipelineState = nil;
    self.iconVertexBuffer = nil;
    self.iconIndexBuffer = nil;
    self.iconSamplerState = nil;
    _iconUniformsBuffer = nil;
    
    printf("Icon renderer cleanup completed\n");
}

@end

// C++ wrapper implementations
void* icon_renderer_create(void* metalDevice) {
    @autoreleasepool {
        id<MTLDevice> device = (__bridge id<MTLDevice>)metalDevice;
        CSCPIconRenderer *renderer = [[CSCPIconRenderer alloc] initWithDevice:device];
        return (__bridge_retained void*)renderer;
    }
}

int icon_renderer_setup(void* renderer) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        return [iconRenderer setupIconRenderer] ? 1 : 0;
    }
}

void icon_renderer_calculate_layout(void* renderer, float width, float height) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer calculateLayoutForScreenSize:CGSizeMake(width, height)];
    }
}

void icon_renderer_add_system_icon(void* renderer, int iconType, int index) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer addSystemIcon:(SystemIconType)iconType atIndex:index];
    }
}

void icon_renderer_add_file_icon(void* renderer, int iconType, int drive) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer addFileIcon:(FileIconType)iconType forDrive:drive];
    }
}

void icon_renderer_remove_all_icons(void* renderer) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer removeAllIcons];
    }
}

void icon_renderer_set_progress_value(void* renderer, float value) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer setProgressValue:value];
    }
}

void icon_renderer_set_progress_color(void* renderer, float r, float g, float b) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer setProgressColor:simd_make_float3(r, g, b)];
    }
}

void icon_renderer_set_progress_visible(void* renderer, int visible) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        [iconRenderer setProgressVisible:visible ? YES : NO];
    }
}

int icon_renderer_handle_touch(void* renderer, float x, float y) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        CGPoint point = CGPointMake(x, y);
        return [iconRenderer handleTouchAtPoint:point] ? 1 : 0;
    }
}

int icon_renderer_get_icon_at_point(void* renderer, float x, float y, int* outType) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        CGPoint point = CGPointMake(x, y);
        IconType iconType;
        NSInteger iconIndex = [iconRenderer getIconAtPoint:point iconType:&iconType];
        if (outType) *outType = (int)iconType;
        return (int)iconIndex;
    }
}

void icon_renderer_render(void* renderer, void* encoder, float width, float height) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge CSCPIconRenderer*)renderer;
        id<MTLRenderCommandEncoder> renderEncoder = (__bridge id<MTLRenderCommandEncoder>)encoder;
        CGSize screenSize = CGSizeMake(width, height);
        [iconRenderer renderIconsWithEncoder:renderEncoder screenSize:screenSize];
    }
}

void icon_renderer_destroy(void* renderer) {
    @autoreleasepool {
        CSCPIconRenderer *iconRenderer = (__bridge_transfer CSCPIconRenderer*)renderer;
        [iconRenderer cleanup];
    }
}