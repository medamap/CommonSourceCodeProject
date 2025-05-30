# CommonSourceCodeProject

<div align="center">

**包括的なマルチプラットフォーム・レトロコンピューターエミュレーターコレクション**

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Android%20%7C%20macOS%20%7C%20iOS%20%7C%20iPadOS-green)](https://github.com/takeda-toshiya/common_source_code_project)

*130以上のヴィンテージコンピューターとゲーム機の正確なエミュレーションによるコンピューター遺産の保存*

**[[English README](README_EN.md)]**

</div>

## 🎯 概要

**CommonSourceCodeProject** は、**TAKEDA, toshiya** によって作成されたオープンソースのレトロコンピューターエミュレーターコレクションで、1970年代から1990年代の **130種類以上のヴィンテージコンピューターとゲーム機** の正確なエミュレーションを提供します。Windows、Android、Appleプラットフォーム（macOS、iOS、iPadOS）でのクロスプラットフォーム対応を可能にする共通コードベースを特徴としています。

### 主な特徴

- 🖥️ **130種類以上のエミュレート対象** - 日本のホームコンピューター、業務用システム、ゲーム機、教育用マシン
- 🎮 **マルチメディアサポート** - フロッピーディスク、カセットテープ、ハードドライブ、カートリッジ
- 🎨 **高度なグラフィック機能** - スキャンライン効果、ブラーフィルター、アクセシビリティ機能付きの正確な映像出力
- 🔊 **オーセンティックなオーディオ** - オリジナルの音響体験のための精密なサウンドチップエミュレーション
- 💾 **セーブステート機能** - いつでもエミュレーションの保存と再開が可能
- 🌐 **クロスプラットフォーム** - Windows、Android、macOS、iOS、iPadOSで一貫した体験

## 🖥️ サポート対象システム

<details>
<summary><strong>🇯🇵 日本のホームコンピューター（クリックして展開）</strong></summary>

### Sharp X1シリーズ
- **X1** - オリジナルX1コンピューター
- **X1 turbo** - 性能向上モデル
- **X1 turboZ** - 高度なグラフィック機能
- **X1 twin** - デュアルシステム対応

### NEC PCシリーズ
- **PC-6001シリーズ**: PC-6001、PC-6001mkII、PC-6001mkIISR
- **PC-8001シリーズ**: PC-8001、PC-8001mkII、PC-8001mkIISR
- **PC-8801シリーズ**: PC-8801、PC-8801mkII、PC-8801MA
- **PC-9801シリーズ**: PC-9801、PC-9801E/F/M、PC-9801VF/VM/VX、PC-9801RA、PC-98XA/XL/RL/DO/LT/HA

### 富士通 FMシリーズ
- **FM-7/77シリーズ**: FM-7、FM-77、FM77AV、FM77AV40、FM77AV40EX、FM77L4
- **FM-8** - 初期の8ビットコンピューター
- **FM16** - 16ビットシステム（FM16β、FM16π）

### Sharp MZシリーズ
- **初期モデル**: MZ-80A/B/K/C、MZ-700、MZ-800、MZ-1200
- **高性能モデル**: MZ-1500、MZ-2200、MZ-2500、MZ-2800、MZ-3500、MZ-5500、MZ-6500、MZ-6550

### MSXシステム
- **MSX1** - オリジナルMSX標準
- **MSX2** - 強化されたグラフィックとメモリ
- **MSX2+** - 高度なMSXシステム
</details>

<details>
<summary><strong>🏢 業務用・プロフェッショナルシステム</strong></summary>

### 富士通 FMRシリーズ
- FMR-30、FMR-50、FMR-60、FMR-70、FMR-80

### TOSHIBAシステム
- J-3100GT、J-3100SL、PASOPIA、PASOPIA7

### EPSONシステム
- HC-20/HX-20、HC-40/PX-4、HC-80/PX-8、QC-10/QX-10
</details>

<details>
<summary><strong>🎮 ゲーム機・エンターテイメント</strong></summary>

### SEGAシステム
- Master System、Game Gear、SC-3000、SCV（Super Cassette Vision）

### NECゲーミング
- PC Engine/TurboGrafx-16（CD-ROM²サポート付き）

### Nintendo
- Family BASIC

### その他のゲームシステム
- ColecoVision、PV-1000、PV-2000、TV BOY
</details>

<details>
<summary><strong>📱 ポータブル・教育用システム</strong></summary>

### ハンドヘルドコンピューター
- CASIO: FP-200、FP-1100、FX-9000P
- CANON: X-07、BX-1
- EPSON: PX-7

### 教育・トレーニング
- TK-80BS、TK-85、MP-85（マイクロプロセッサー学習システム）
- Babbage-2nd（教育用コンピューター）
- Z80 TV GAME SYSTEM（自作ゲーム機）
</details>

## 🚀 プラットフォームサポート

### ✅ Windows（メインプラットフォーム）
- **ビルドシステム**: Microsoft Visual C++ 2008/2017
- **グラフィック**: DirectX 9.0 with DirectInput
- **ステータス**: 完全なGUI機能付きで完全動作
- **機能**: 全エミュレーション機能が利用可能

### ✅ Android
- **ビルドシステム**: Android Studio with Gradle 8.3.1
- **グラフィック**: OpenGL ES
- **オーディオ**: Oboe高性能オーディオライブラリ
- **ステータス**: タッチ最適化インターフェース付きで動作
- **機能**: モバイル特化UI適応

### 🚧 macOS/iOS/iPadOS（開発中）
- **ビルドシステム**: CMake + Xcode
- **グラフィック**: 最新GPU加速のためのMetal API
- **オーディオ**: Core Audio/AudioUnit統合（予定）
- **ステータス**: 動作プロトタイプによる高度な開発段階

#### 現在のAppleプラットフォーム進捗：
- ✅ Metalレンダリング実装
- ✅ メディアマウント機能付きコマンドラインインターフェース
- ✅ CMakeビルドシステムによる130種類以上のマシンサポート
- ✅ 自動コンパイル用バッチビルドシステム
- 🔄 GUIアプリケーション開発進行中
- 📅 Core Audio統合予定

## 📂 プロジェクト構成

```
CommonSourceCodeProject/
├── src/                           # コアソースコード
│   ├── vm/                        # 仮想マシンコア（CPU、サウンドチップなど）
│   ├── menu/                      # マシン固有の設定メニュー
│   ├── win32/                     # Windows固有の実装
│   ├── Android/                   # Android固有の実装
│   └── Xcode/                     # Appleプラットフォーム実装
├── androidstudio/                 # Androidビルドシステム
│   └── buildbatch/               # Androidバッチビルドスクリプト
├── xcode/                         # Appleプラットフォームビルドシステム
│   ├── buildbatch/               # Appleバッチビルドスクリプト
│   ├── Machines/                 # マシン設定ファイル
│   └── build_*.sh               # ビルドスクリプト
├── vc++2008/                      # Visual C++ 2008プロジェクトファイル
├── vc++2017/                      # Visual C++ 2017プロジェクトファイル
└── res/                           # リソース（アイコンなど）
```

## 🔨 ビルド手順

### Windows
```bash
# 要件: Visual C++ 2008 SP1 または Visual C++ 2017
# DirectX SDK必須（9.0推奨）

# Visual Studioでプロジェクトファイルを開く:
# - vc++2008/*.vcproj (VS 2008用)
# - vc++2017/*.vcxproj (VS 2017用)
```

### Android
```bash
cd androidstudio

# 単一モデルビルド
./buildbatch/allexecute.sh -d x1turbo -m ReleaseBuild

# CSVから複数モデルビルド
./buildbatch/allexecute.sh -i models/x1.csv -m ReleaseBuild

# 全モデルビルド
./gradlew assembleRelease
```

### macOS/iOS/iPadOS
```bash
cd xcode

# 単一マシンビルド
./build_machine.sh x1turbo

# バッチシステム経由ビルド（未署名）
./buildbatch/allexecute.sh -d x1turbo -m MacFree

# CSVから複数マシンビルド
./buildbatch/allexecute.sh -i models/x1.csv -m MacFree

# 手動CMakeビルド
mkdir build_x1turbo && cd build_x1turbo
cmake -DMACHINE=x1turbo ..
make -j4
```

#### Appleプラットフォームビルドモード
- **MacFree**: 未署名macOS実行ファイル（開発・テスト用）
- **MacDeveloper**: Apple Developer Program証明書で署名
- **iOSFree**: 個人チーム証明書でのiOSビルド
- **iOSDeveloper**: Apple Developer Program証明書でのiOSビルド

## 🎮 使用例

### コマンドラインインターフェース（macOS/Linux）
```bash
# X1 Turbo基本起動
./x1turbo

# フロッピーディスク読み込み
./x1turbo -fdd0 "cz8fb01.2d"

# カセットテープ読み込み
./x1turbo -tape0 "cz8fb01.tap"

# 複数メディア同時マウント
./x1turbo \
  -fdd0 "cz8fb01.d88" \
  -fdd1 "datadisk.d88" \
  -tape0 "usertape.tap"

# MSXでカートリッジ
./msx1 -cart0 "gamecart.rom"
```

### サポートファイル形式
- **フロッピーディスク**: `.d88`、`.2d`、`.fdi`
- **カセットテープ**: `.tap`、`.t77`
- **ハードディスク**: 各種`.hd?`形式
- **カートリッジ**: `.rom`、`.bin`

## 🏗️ 技術アーキテクチャ

### コア設計思想
- **共通コードベース**: 全プラットフォーム間で共通のエミュレーションコア
- **プラットフォーム抽象化**: UI、グラフィック、入力のOS固有レイヤー
- **モジュラー設計**: 独立したモジュールでのマシン固有実装
- **明確な分離**: プラットフォームレンダリングから独立したコアエミュレーションロジック

### 主要コンポーネント

#### 仮想マシンコア（`src/vm/`）
- **CPUエミュレーション**: Z80、8080、6502、68000、x86シリーズ
- **サウンドチップ**: YM2203、YM2151、SN76489、AY-3-8910
- **グラフィックコントローラー**: TMS9918、HD46505、uPD7220
- **I/Oコントローラー**: 各種PIO、SIO、FDC実装

#### プラットフォーム固有レイヤー
- **OSD（Operating System Dependent）**: 画面、音声、入力、ファイルI/O
- **メニューシステム**: マシン固有の設定とコントロール
- **ビルドシステム**: プラットフォーム適応のコンパイルとパッケージング

## 🌟 最新の開発ハイライト

### Androidポートの成果
- WindowsバッチビルドシステムからShellスクリプトへの完全移行
- 130種類以上のマシンをサポートするGradleベースビルドシステム
- 仮想コントロール付きタッチ最適化インターフェース
- Oboeライブラリによる高性能オーディオ

### Appleプラットフォーム実装（2024-2025）
- **Metal API統合**: 最新のGPU加速レンダリング
- **CMakeビルドシステム**: 130種類以上のヴィンテージコンピューターモデルサポート
- **コマンドラインインターフェース**: 完全なメディアマウント機能（フロッピー、テープ、カートリッジ、HDD）
- **バッチビルドシステム**: 複数マシンタイプの自動コンパイル
- **クロスプラットフォームOSD**: コア互換性を維持したApple固有最適化

### 最近のコア改善
- 色覚アクセシビリティサポート（5種類の異なる視覚タイプ）
- SMC-777/70ボーダーカラーサポート
- MZシリーズMIDI音源CMU-800サポート
- パレット使用最適化とバグ修正
- アナログパレットボーダーカラー実装

## 🤝 貢献・コミュニティ

### 貢献者
プロジェクトは多数の開発者の貢献により成り立っています：
- **TAKEDA, toshiya** - 原作者・メインメンテナー
- **Mr. Artane** - FM-7/77シリーズ実装
- **Mr. tanam** - MSX、PC-6001、Game Gear実装
- **Mr. umaiboux** - MSX2+、各種マシン貢献
- **Mr. GORRY** - MICOM MAHJONGエミュレーター
- **Mr. Meister** - X1拡張・改良
- **多数の方々** - CPUコア、サウンドチップ、マシン固有実装

### ライセンス
このプロジェクトは**GNU General Public License Version 2**の下でリリースされています。
完全なライセンス条項については[COPYING.txt](license/COPYING.txt)を参照してください。

### 謝辞
ソースコードには以下に対する広範囲な謝辞が含まれています：
- サードパーティCPUコア（MAME、MESSプロジェクト）
- 音声合成ライブラリ（fmgen、各種チップ実装）
- プラットフォーム固有最適化・改良
- ハードウェアドキュメントとリバースエンジニアリング努力

## 📚 ドキュメント・サポート

### プロジェクトドキュメント
- **英語**: この英語版READMEとインラインコードドキュメント（[README_EN.md](README_EN.md)）
- **日本語**: `readme.txt`とソースコメントのオリジナルドキュメント
- **Wiki**: プロジェクトwiki（オンラインリポジトリ参照）の追加情報

### 開発ドキュメント
- **Appleプラットフォーム**: `claude/macOS_implementation_status.md`の詳細実装進捗
- **ビルドシステム**: 各ディレクトリのプラットフォーム固有ビルドドキュメント
- **アーキテクチャ**: ソースヘッダーで文書化されたコアエミュレーションアーキテクチャ

## 🔮 将来のロードマップ

### Appleプラットフォーム（最優先事項）
1. **Core Audio統合** - オーセンティックな音声再生
2. **GUIアプリケーション** - ネイティブmacOS/iOSインターフェース
3. **App Store対応** - 公式チャンネル経由での配布
4. **iCloud統合** - シームレスなファイル同期
5. **タッチインターフェース最適化** - iPad固有の改良

### クロスプラットフォーム強化
1. **セーブステート管理** - 強化されたセーブ/ロード機能
2. **ネットワーク接続** - マルチプレイヤーとファイル共有機能
3. **最新ディスプレイサポート** - High-DPIとHDR対応
4. **パフォーマンス最適化** - GPU加速改善

## 📧 オリジナル

- **ウェブサイト**: http://takeda-toshiya.my.coocan.jp/
- **ライセンス**: GNU GPL Version 2
- **最終更新**: 2023年12月31日

---

<div align="center">

**次世代のためのコンピューター歴史の保存**

*CommonSourceCodeProjectは、ヴィンテージコンピューティングシステムの保存とアクセス提供における最も包括的な取り組みの一つであり、パーソナルコンピューティングの豊かな歴史が最新ハードウェア上でアクセス可能であり続けることを保証しています。*

</div>