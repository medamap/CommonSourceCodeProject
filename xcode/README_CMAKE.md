# CommonSourceCodeProject Xcode/CMake ビルドシステム

## 概要

このディレクトリには、CommonSourceCodeProjectをmacOS/iOS向けにビルドするためのCMakeベースのビルドシステムが含まれています。

## 機能

### 実装済みシステム

1. **メニューシステム** - 機種別メニューの統合管理
2. **入力システム** - キーボード・マウス・タッチ対応
3. **音声システム** - AudioQueueによる音声出力
4. **画面表示システム** - Metal APIによるGPU描画
5. **ファイル選択システム** - NSOpenPanel/UIDocumentPicker対応
6. **設定ダイアログシステム** - NSAlert/UIAlertController対応
7. **アラートシステム** - エラー・確認ダイアログ
8. **多機種対応CMakeシステム** - 自動設定ファイル生成

### 対応機種

- **X1シリーズ**: X1 turbo, X1
- **PC-8801シリーズ**: PC-8801
- **PC-9801シリーズ**: PC-9801
- **MSXシリーズ**: MSX1, MSX2
- **FM-7シリーズ**: FM-7, FM-77
- **MZシリーズ**: MZ-2500, MZ-700
- **その他**: PC-6001, ColecoVision

## セットアップ

### 必要な環境

- macOS 11.0以上
- Xcode 12.0以上
- CMake 3.15以上
- Python 3.6以上（設定ファイル生成用）

### 基本的な使用方法

#### 1. 機種設定ファイルの生成

```bash
# 主要機種の設定ファイルを自動生成
python3 generate_machine_configs.py

# 特定機種のみ生成
python3 generate_machine_configs.py x1turbo pc8801 msx1
```

#### 2. ビルド実行

```bash
# 簡単ビルド（推奨）
./build_machine.sh x1turbo

# 手動CMakeビルド
mkdir build_x1turbo_debug
cd build_x1turbo_debug
cmake -DMACHINE=x1turbo ..
make -j4
```

#### 3. ビルドオプション

```bash
# リリースビルド
./build_machine.sh x1turbo --release

# クリーンビルド
./build_machine.sh x1turbo --clean --release

# 並列ビルド（8コア）
./build_machine.sh x1turbo -j8

# 詳細出力
./build_machine.sh x1turbo --verbose
```

## ディレクトリ構造

```
xcode/
├── CMakeLists.txt              # メインCMake設定
├── Machines/                   # 機種別設定ファイル
│   ├── _X1TURBO.txt           # X1 turbo設定
│   ├── _PC8801.txt            # PC-8801設定
│   ├── _MSX1.txt              # MSX1設定
│   └── ...
├── generate_machine_configs.py # 設定ファイル自動生成
├── build_machine.sh           # ビルドスクリプト
├── build_*/                   # ビルド出力ディレクトリ
└── README_CMAKE.md            # このファイル
```

## CMake設定

### 主要な変数

- `MACHINE`: ビルド対象機種（必須）
- `CMAKE_BUILD_TYPE`: Debug/Release
- `CMAKE_OSX_DEPLOYMENT_TARGET`: 最小macOSバージョン

### 使用例

```bash
# X1 turbo のデバッグビルド
cmake -DMACHINE=x1turbo -DCMAKE_BUILD_TYPE=Debug ..

# PC-8801 のリリースビルド
cmake -DMACHINE=pc8801 -DCMAKE_BUILD_TYPE=Release ..

# MSX1 のiOSビルド
cmake -DMACHINE=msx1 -DIOS=ON ..
```

## 機種設定ファイル

### ファイル形式

各機種の設定ファイル（`Machines/_MACHINE.txt`）には以下が含まれます：

```cmake
set(HEADER
    ${SRC_DIR}/common.h
    ${SRC_DIR}/vm/machine_specific/header.h
    # ... 機種固有のヘッダーファイル
)

set(SOURCES
    ${SRC_DIR}/common.cpp
    ${SRC_DIR}/vm/machine_specific/source.cpp
    # ... 機種固有のソースファイル
)
```

### 自動生成

設定ファイルは以下の方法で自動生成されます：

1. `src/menu/` ディレクトリから機種別メニューファイルを検出
2. `src/vm/` ディレクトリから機種別VMファイルを検出
3. 機種名に基づいてCPU・デバイス依存関係を推定
4. 共通ファイル + 機種固有ファイルのリストを生成

## ビルドスクリプト

### build_machine.sh の機能

- **機種名バリデーション**: サポート機種の自動チェック
- **設定ファイル確認**: 必要な設定ファイルの存在確認
- **ディレクトリ管理**: 機種別ビルドディレクトリの自動作成
- **CMake実行**: 適切なパラメータでのCMake設定
- **ビルド実行**: 並列ビルドとエラーハンドリング
- **結果確認**: 実行ファイルの生成確認

### コマンドライン例

```bash
# ヘルプ表示
./build_machine.sh --help

# 基本ビルド
./build_machine.sh x1turbo

# 全オプション指定
./build_machine.sh pc8801 --clean --release -j8 --verbose
```

## トラブルシューティング

### よくある問題

#### 1. 機種設定ファイルが見つからない

```bash
[ERROR] 機種設定ファイルが見つかりません: Machines/_MACHINE.txt
```

**解決方法**: 設定ファイルを生成してください
```bash
python3 generate_machine_configs.py machine_name
```

#### 2. CMake設定エラー

```bash
CMake Error: Machine configuration not found
```

**解決方法**: 正しい機種名を指定してください
```bash
./build_machine.sh --help  # 利用可能な機種を確認
```

#### 3. ビルドエラー

```bash
error: 'some_header.h' file not found
```

**解決方法**: 依存関係を確認し、設定ファイルを更新してください
```bash
# 設定ファイルを再生成
python3 generate_machine_configs.py machine_name
```

### デバッグ方法

#### 詳細出力でビルド

```bash
./build_machine.sh machine_name --verbose
```

#### CMakeキャッシュのクリア

```bash
./build_machine.sh machine_name --clean
```

#### 手動CMake実行

```bash
mkdir build_debug
cd build_debug
cmake -DMACHINE=machine_name -DCMAKE_VERBOSE_MAKEFILE=ON ..
make VERBOSE=1
```

## 開発者向け情報

### 新しい機種の追加

1. `src/menu/new_machine.cpp` にメニューファイルを追加
2. `src/vm/new_machine/` にVM実装を追加
3. 設定ファイルを生成: `python3 generate_machine_configs.py new_machine`
4. ビルドテスト: `./build_machine.sh new_machine`

### ビルドシステムの拡張

#### 新しいプラットフォーム対応

1. `CMakeLists.txt` にプラットフォーム固有の設定を追加
2. `generate_machine_configs.py` で新しい依存関係を追加
3. `build_machine.sh` で新しいオプションを追加

#### 新しいライブラリの追加

1. `CMakeLists.txt` の `find_library` セクションを更新
2. 各機種設定ファイルに必要なヘッダー・ソースを追加

## ライセンス

このビルドシステムはCommonSourceCodeProjectと同じライセンスに従います。

## 作者

- Medamap and Claude
- 2025年1月29日