//
//  Metal Shaders for CSCP Emulator
//  
//  Author : Medamap & Claude
//  Date   : 2025.05.30
//  
//  [Advanced Metal shaders for visual effects]
//

#include <metal_stdlib>
using namespace metal;

// Vertex shader input/output structures
struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

// Uniform buffer for shader parameters
struct Uniforms {
    float2 screenSize;
    float time;
    int effectType;
    int colorBlindnessType;
    float blurRadius;
    float scanlineIntensity;
    float dotPattern;
};

// Color blindness transformation matrices
constant float3x3 colorBlindnessMatrices[5] = {
    // Normal vision (C型)
    float3x3(1.0, 0.0, 0.0,
             0.0, 1.0, 0.0, 
             0.0, 0.0, 1.0),
    
    // Protanope (P型) - strong red-green color blindness
    float3x3(0.567, 0.433, 0.0,
             0.558, 0.442, 0.0,
             0.0, 0.242, 0.758),
    
    // Protanomaly (PA型) - weak red-green color blindness  
    float3x3(0.817, 0.183, 0.0,
             0.333, 0.667, 0.0,
             0.0, 0.125, 0.875),
    
    // Deuteranope (D型) - strong green-red color blindness
    float3x3(0.625, 0.375, 0.0,
             0.7, 0.3, 0.0,
             0.0, 0.3, 0.7),
    
    // Deuteranomaly (DA型) - weak green-red color blindness
    float3x3(0.8, 0.2, 0.0,
             0.258, 0.742, 0.0,
             0.0, 0.142, 0.858)
};

// Apply color blindness simulation
float3 applyColorBlindness(float3 color, int type) {
    if (type < 0 || type >= 5) {
        return color;
    }
    return colorBlindnessMatrices[type] * color;
}

// Common vertex shader
vertex VertexOut vertexShader(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    return out;
}

// Basic fragment shader (no effects)
fragment float4 basicFragmentShader(VertexOut in [[stage_in]],
                                   texture2d<float> texture [[texture(0)]],
                                   sampler textureSampler [[sampler(0)]],
                                   constant Uniforms& uniforms [[buffer(0)]]) {
    float4 color = texture.sample(textureSampler, in.texCoord);
    
    // Apply color blindness simulation
    color.rgb = applyColorBlindness(color.rgb, uniforms.colorBlindnessType);
    
    return color;
}

// Blur effect fragment shader
fragment float4 blurFragmentShader(VertexOut in [[stage_in]],
                                  texture2d<float> texture [[texture(0)]],
                                  sampler textureSampler [[sampler(0)]],
                                  constant Uniforms& uniforms [[buffer(0)]]) {
    float2 texelSize = 1.0 / uniforms.screenSize;
    float radius = uniforms.blurRadius;
    
    float3 color = float3(0.0);
    float totalWeight = 0.0;
    
    // Gaussian blur kernel
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            float2 offset = float2(x, y) * texelSize * radius;
            float weight = exp(-0.5 * (x*x + y*y) / (radius*radius));
            
            float3 sampleColor = texture.sample(textureSampler, in.texCoord + offset).rgb;
            color += sampleColor * weight;
            totalWeight += weight;
        }
    }
    
    color /= totalWeight;
    
    // Apply color blindness simulation
    color = applyColorBlindness(color, uniforms.colorBlindnessType);
    
    return float4(color, 1.0);
}

// Scanline/TV effect fragment shader
fragment float4 scanlineFragmentShader(VertexOut in [[stage_in]],
                                      texture2d<float> texture [[texture(0)]],
                                      sampler textureSampler [[sampler(0)]],
                                      constant Uniforms& uniforms [[buffer(0)]]) {
    float4 color = texture.sample(textureSampler, in.texCoord);
    
    // Scanline effect
    float scanline = sin(in.texCoord.y * uniforms.screenSize.y * 3.14159) * 0.5 + 0.5;
    scanline = mix(0.7, 1.0, scanline);
    
    // RGB phosphor simulation
    float2 pixelPos = in.texCoord * uniforms.screenSize;
    float3 phosphor = float3(1.0);
    
    if (uniforms.dotPattern > 0.0) {
        int pixelX = int(pixelPos.x) % 3;
        if (pixelX == 0) phosphor = float3(1.0, 0.7, 0.7);      // Red phosphor
        else if (pixelX == 1) phosphor = float3(0.7, 1.0, 0.7); // Green phosphor
        else phosphor = float3(0.7, 0.7, 1.0);                  // Blue phosphor
    }
    
    color.rgb *= scanline * uniforms.scanlineIntensity;
    color.rgb *= phosphor;
    
    // Apply color blindness simulation
    color.rgb = applyColorBlindness(color.rgb, uniforms.colorBlindnessType);
    
    return color;
}

// Green display fragment shader (monochrome terminal effect)
fragment float4 greenDisplayFragmentShader(VertexOut in [[stage_in]],
                                          texture2d<float> texture [[texture(0)]],
                                          sampler textureSampler [[sampler(0)]],
                                          constant Uniforms& uniforms [[buffer(0)]]) {
    float4 color = texture.sample(textureSampler, in.texCoord);
    
    // Convert to grayscale using luminance weights
    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    // Create green phosphor effect
    float2 pixelPos = in.texCoord * uniforms.screenSize;
    float flicker = sin(uniforms.time * 60.0 + pixelPos.y * 0.1) * 0.02 + 0.98;
    
    // Green color variations based on intensity
    float3 greenColor = float3(0.0, gray * 1.2, gray * 0.8) * flicker;
    
    // Apply color blindness simulation
    greenColor = applyColorBlindness(greenColor, uniforms.colorBlindnessType);
    
    return float4(greenColor, color.a);
}

// RGB565 to RGBA conversion fragment shader
fragment float4 rgb565FragmentShader(VertexOut in [[stage_in]],
                                     texture2d<ushort> texture [[texture(0)]],
                                     sampler textureSampler [[sampler(0)]],
                                     constant Uniforms& uniforms [[buffer(0)]]) {
    ushort rgb565 = texture.sample(textureSampler, in.texCoord).r;
    
    // Extract RGB components from RGB565
    float r = float((rgb565 >> 11) & 0x1F) / 31.0;
    float g = float((rgb565 >> 5) & 0x3F) / 63.0;
    float b = float(rgb565 & 0x1F) / 31.0;
    
    float3 color = float3(r, g, b);
    
    // Apply color blindness simulation
    color = applyColorBlindness(color, uniforms.colorBlindnessType);
    
    return float4(color, 1.0);
}

// =============================================================================
// ICON RENDERING SHADERS
// =============================================================================

// Icon vertex input/output structures
struct IconVertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float alpha [[attribute(2)]];
};

struct IconVertexOut {
    float4 position [[position]];
    float2 texCoord;
    float alpha;
};

// Icon uniforms
struct IconUniforms {
    float2 screenSize;
    float4x4 projectionMatrix;
    float3 tintColor;
    float globalAlpha;
};

// Icon vertex shader
vertex IconVertexOut iconVertexShader(IconVertexIn in [[stage_in]],
                                     constant IconUniforms& uniforms [[buffer(1)]],
                                     constant float4x4& modelMatrix [[buffer(2)]]) {
    IconVertexOut out;
    
    // Transform position with model matrix
    float4 worldPosition = modelMatrix * float4(in.position, 0.0, 1.0);
    
    // Apply projection
    out.position = uniforms.projectionMatrix * worldPosition;
    out.texCoord = in.texCoord;
    out.alpha = in.alpha * uniforms.globalAlpha;
    
    return out;
}

// Icon fragment shader
fragment float4 iconFragmentShader(IconVertexOut in [[stage_in]],
                                  texture2d<float> iconTexture [[texture(0)]],
                                  sampler iconSampler [[sampler(0)]],
                                  constant IconUniforms& uniforms [[buffer(1)]]) {
    
    float4 color = iconTexture.sample(iconSampler, in.texCoord);
    
    // Apply tint color
    color.rgb *= uniforms.tintColor;
    
    // Apply alpha
    color.a *= in.alpha;
    
    return color;
}

// =============================================================================
// PROGRESS BAR SHADERS
// =============================================================================

// Progress bar vertex shader
vertex IconVertexOut progressVertexShader(IconVertexIn in [[stage_in]],
                                         constant IconUniforms& uniforms [[buffer(1)]],
                                         constant float4x4& modelMatrix [[buffer(2)]]) {
    IconVertexOut out;
    
    // Transform position with model matrix
    float4 worldPosition = modelMatrix * float4(in.position, 0.0, 1.0);
    
    // Apply projection
    out.position = uniforms.projectionMatrix * worldPosition;
    out.texCoord = in.texCoord;
    out.alpha = in.alpha * uniforms.globalAlpha;
    
    return out;
}

// Progress bar fragment shader
fragment float4 progressFragmentShader(IconVertexOut in [[stage_in]],
                                      constant IconUniforms& uniforms [[buffer(1)]],
                                      constant float& progressValue [[buffer(3)]]) {
    
    // Create progress bar effect
    float progress = progressValue; // 0.0 to 1.0
    float barPosition = in.texCoord.x;
    
    float4 color;
    if (barPosition <= progress) {
        // Filled portion
        color = float4(uniforms.tintColor, 1.0);
    } else {
        // Empty portion
        color = float4(uniforms.tintColor * 0.3, 0.5);
    }
    
    // Add border
    float borderWidth = 0.05;
    if (in.texCoord.x < borderWidth || in.texCoord.x > (1.0 - borderWidth) ||
        in.texCoord.y < borderWidth || in.texCoord.y > (1.0 - borderWidth)) {
        color = float4(1.0, 1.0, 1.0, 1.0); // White border
    }
    
    color.a *= in.alpha;
    return color;
}