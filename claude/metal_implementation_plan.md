# Metal描画システム実装計画

## 概要
Android版のOpenGL ES実装を参考に、macOS/iOS向けのMetal実装を設計する。

## Android OpenGL ES実装の分析

### テクスチャ管理
- **フォーマット**: RGB565（エミュレータ内部）→ RGBA8888（表示用）
- **更新方法**: 毎フレームglTexImage2Dでアップロード
- **フィルタリング**: GL_NEAREST（ピクセルパーフェクト）/ GL_LINEAR（スムーズ）

### シェーダーシステム
1. **通常カラーシェーダー**: 基本的なテクスチャ描画
2. **ブラーシェーダー**: ぼかしエフェクト
3. **TVシェーダー**: CRTスキャンライン
4. **グリーンシェーダー**: モノクロディスプレイ

### 色覚シミュレーション
- P型（1型2色覚）
- PA型（1型3色覚）
- D型（2型2色覚）
- DA型（2型3色覚）
- T型（3型2色覚）

## Metal実装設計

### アーキテクチャ
```
Swift側（MetalRenderer）
    ↓
C++ Bridge（xcode_screen.cpp）
    ↓
OSD Interface（osd.h）
    ↓
Emulator Core（emu.cpp）
```

### クラス設計

#### Swift側
```swift
class MetalRenderer {
    // Metal基本オブジェクト
    var device: MTLDevice
    var commandQueue: MTLCommandQueue
    var pipelineStates: [ShaderType: MTLRenderPipelineState]
    
    // テクスチャ管理
    var screenTexture: MTLTexture
    var intermediateTexture: MTLTexture // エフェクト用
    
    // シェーダーパラメータ
    var uniforms: Uniforms
}
```

#### C++側
```cpp
class XcodeScreen {
    // スクリーンバッファ管理
    void* screen_buffer;
    int width, height;
    
    // Swiftレンダラーへのポインタ
    void* metal_renderer;
    
    // インターフェース関数
    void initialize_screen(int width, int height);
    void update_screen();
    void set_shader_effect(int effect_type);
};
```

### テクスチャ処理フロー

1. **エミュレータ → Metal**
   ```
   RGB565バッファ（C++）
   → バイトスワップ/変換
   → MTLTextureにコピー
   → シェーダーで描画
   ```

2. **フォーマット変換オプション**
   - 方法A: CPUでRGB565→RGBA8888変換後、テクスチャ更新
   - 方法B: RGB565テクスチャを直接作成、シェーダーで変換
   - 推奨: 方法B（GPUで処理、効率的）

### シェーダー実装

#### Vertex Shader（共通）
```metal
struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

vertex VertexOut vertexShader(uint vertexID [[vertex_id]]) {
    // 画面全体を覆う四角形を生成
    // ...
}
```

#### Fragment Shaders

1. **基本シェーダー**
```metal
fragment float4 basicFragment(VertexOut in [[stage_in]],
                             texture2d<float> texture [[texture(0)]]) {
    // RGB565 → RGBA変換
    // 色覚シミュレーション適用
}
```

2. **エフェクトシェーダー**
- ブラー: 周辺ピクセルの重み付き平均
- スキャンライン: Y座標に基づく明度調整
- グリーンディスプレイ: モノクロ化 + 緑色フィルター

### パフォーマンス最適化

1. **トリプルバッファリング**
   - 3つのテクスチャをローテーション
   - CPU/GPU同期の削減

2. **テクスチャ更新の最適化**
   - blitエンコーダーを使用した高速コピー
   - 変更された領域のみ更新

3. **シェーダー最適化**
   - 事前計算可能な値はuniformバッファに格納
   - 分岐を最小限に抑える

## 実装手順

### Step 1: 基本的なMetal環境
1. MetalView（MTKView）の作成
2. 基本的なパイプラインステートの構築
3. テクスチャの作成と更新機能

### Step 2: C++ブリッジの実装
1. xcode_screen.cppの作成
2. initialize_screen(), update_screen()の実装
3. OSDインターフェースとの統合

### Step 3: シェーダーの実装
1. 基本的な描画シェーダー
2. 色覚シミュレーション機能
3. 各種エフェクトシェーダー

### Step 4: 最適化とデバッグ
1. パフォーマンス計測とボトルネック特定
2. メモリ使用量の最適化
3. 各種デバイスでのテスト

## 参考実装

### テクスチャ更新（C++側）
```cpp
void XcodeScreen::update_screen() {
    // エミュレータのスクリーンバッファを取得
    uint16_t* src = (uint16_t*)emu->get_screen_buffer();
    
    // Metalテクスチャに転送
    bridge_update_metal_texture(metal_renderer, src, width, height);
}
```

### ブリッジ関数（Objective-C++）
```objc
void bridge_update_metal_texture(void* renderer, void* buffer, int width, int height) {
    MetalRenderer* metalRenderer = (__bridge MetalRenderer*)renderer;
    [metalRenderer updateTextureWithBuffer:buffer width:width height:height];
}
```