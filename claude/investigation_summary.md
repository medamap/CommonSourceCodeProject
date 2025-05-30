# CommonSourceCodeProject Xcode移植調査結果

## プロジェクト概要
レトロコンピュータエミュレータを macOS/iOS/iPadOS に移植するプロジェクト

## ディレクトリ構造
- `androidstudio/` - Android Studio用プロジェクト
- `src/` - エミュレータ本体のソースコード（C++）
  - `win32/` - Windows用OS依存レイヤー
  - `Android/` - Android用OS依存レイヤー
  - `Xcode/` - macOS/iOS用OS依存レイヤー（移植中）
  - `vm/` - 各種レトロパソコンのVMレイヤー
  - `menu/` - 共通メニューシステム
- `xcode/` - Xcode用プロジェクト（X1 turbo対応中）
- `tool/` - 各種ユーティリティツール
- `vc++2008/`, `vc++2017/` - Windows用プロジェクトファイル

## 現状分析

### 1. Xcode フォルダの実装状況

#### 実装済み
- 基本的なプロジェクト構造（CMakeLists.txt）
- 最小限のOS依存レイヤーインターフェース
- Swift-C++ブリッジの基本構造

#### 未実装
- 画面描画システム（Metal）
- 音声出力システム（Core Audio）
- 入力処理システム（キーボード/タッチ）
- ファイルI/O（iOS/macOSファイルシステム）
- 設定管理

### 2. プラットフォーム間の比較

#### OSDモジュール構成
| モジュール | Win32 | Android | Xcode（計画） |
|-----------|-------|---------|--------------|
| osd.cpp | ✓ | ✓ | △（最小実装） |
| osd_console.cpp | ✓ | ✓ | ✗ |
| osd_input.cpp | ✓ | ✓ | ✗ |
| osd_screen.cpp | ✓ | ✓ | ✗ |
| osd_sound.cpp | ✓ | ✓ | ✗ |
| osd_midi.cpp | ✓ | ✓ | ✗ |
| osd_socket.cpp | ✓ | ✓ | ✗ |
| osd_video.cpp | ✓ | ✗ | ✗ |

#### レンダリング方式
- **Windows**: DirectX/GDI
- **Android**: OpenGL ES 2.0/3.0 + シェーダー
- **Xcode（計画）**: Metal + シェーダー

### 3. Android実装の特徴（参考）

#### OpenGL ES実装
- RGB565/RGBA8888テクスチャ対応
- 複数のシェーダーエフェクト
  - 通常表示
  - ブラーフィルター
  - CRTスキャンライン
  - グリーンディスプレイ
- 色覚シミュレーション（5種類）
- スクリーンスケーリング/フィルタリング

#### メニューシステム
- BaseMenuクラスによる動的メニュー構築
- タッチ操作に最適化されたUI
- アイコンベースのインターフェース

### 4. 技術的課題

#### 画面描画
- MetalでのRGB565テクスチャ処理
- シェーダーエフェクトの移植
- 画面回転/スケーリング対応

#### 音声処理
- 低レイテンシ音声出力の実現
- バックグラウンド対応

#### 入力処理
- ソフトウェアキーボード統合
- ゲームコントローラー対応
- タッチジェスチャー

#### ファイルシステム
- アプリケーションサンドボックス
- iCloud Drive統合
- ドキュメントピッカー対応

## 移植方針

### 設計原則
1. 機能ごとにファイルを分離（xcode_screen.cpp, xcode_input.cpp等）
2. プラットフォーム固有機能はSwift側で実装
3. C++側は純粋なインターフェースとして維持
4. Android実装を参考にしつつ、よりクリーンな設計を目指す

### ファイル構成計画
```
src/Xcode/
├── osd.cpp/h          - メインOSDインターフェース
├── xcode_screen.cpp   - Metal描画実装
├── xcode_input.cpp    - 入力処理
├── xcode_sound.cpp    - Core Audio実装
├── xcode_file.cpp     - ファイルI/O
├── xcode_config.cpp   - 設定管理
└── Bridge.h/mm        - Swift-C++ブリッジ
```