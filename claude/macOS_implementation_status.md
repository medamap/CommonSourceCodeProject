# macOS/iOS/iPad エミュレータ実装状況

このドキュメントは、レトロパソコンエミュレータのmacOS/iOS/iPad移植プロジェクトの実装状況をまとめています。

## フォルダ構成（実際の構成）

```
temp/emulator/                 # 機種別エミュレータファイル（.gitignore対象）
├── x1turboROM/               # X1 Turbo用ファイル
│   ├── IPLROM.X1T            # IPL ROM
│   ├── CGROM.X1              # キャラクタジェネレータROM
│   ├── FNT0808.X1            # フォント8x8
│   ├── FNT0816.X1            # フォント8x16
│   ├── FNT1616.X1            # フォント16x16
│   ├── PCG0808.X1            # プログラマブルキャラクタジェネレータ
│   ├── x1turbo.ini           # X1 Turbo専用設定ファイル（保存予定）
│   ├── DISK/                 # フロッピーディスクイメージ
│   │   ├── Alpha (X1turbo).2d       # テスト用メインディスク ✅
│   │   ├── Xanadu (Disk A).d88      # RPGゲーム例
│   │   └── ...               # その他100個以上のゲーム
│   ├── TAPE/                 # カセットテープイメージ
│   │   ├── Mario Bros. Special.tap  # テスト確認済み ✅
│   │   └── ...               # その他テープゲーム
│   ├── HDD/                  # ハードディスクイメージ（空）
│   └── CART/                 # カートリッジROM（空）
├── msx1ROM/                  # MSX1用ファイル
├── msx2ROM/                  # MSX2用ファイル
├── pc8801maROM/              # PC-8801MA用ファイル
└── ...                       # その他多数の機種
```

## 実装状況

### ✅ 完了済み機能

1. **CMakeビルドシステム**
   - 機種別ビルド対応（`./build_machine.sh -m x1turbo`）
   - Apple プラットフォーム専用設定
   - Xcodeプロジェクト自動生成
   - Metal/AppKitフレームワークリンク ⭐ **新規実装**

2. **Apple プラットフォーム対応OSDレイヤー**
   - macOS/iOS/iPad 用OSD実装
   - プリプロセッサ条件による分岐（`#ifdef __APPLE__`）
   - 60FPS メインループ（フレーム制御付き）

3. **コマンドライン引数解析システム**
   - 機種指定（`-machine x1turbo`）
   - フロッピーディスク（`-fdd0` ～ `-fdd3`）
   - テープドライブ（`-tape0`, `-tape1`）
   - ハードディスク（`-hdd0` ～ `-hdd3`）
   - カートリッジ（`-cart0` ～ `-cart3`）
   - ヘルプ表示（`-help`, `--help`）

4. **実際のメディアマウント機能**
   - EMUクラスのメディア関連メソッド呼び出し
   - パス変換処理（char* → _TCHAR*）
   - エラーハンドリングと成功/失敗ログ
   - 複数メディア同時マウント対応

5. **Metal API統合** ⭐ **新規実装**
   - MetalViewクラス（MTKView継承）
   - Metal シェーダー（頂点・フラグメント）
   - テクスチャ更新機能（RGB565/RGBA8888対応）
   - macOS/iOS両対応の描画実装

6. **GUI アプリケーション基盤** ⭐ **新規実装**
   - CSCPViewController（macOS/iOS両対応）
   - CSCPAppDelegate（macOS専用）
   - ネイティブウィンドウ管理
   - キーボード入力対応

### 🎯 動作確認済みテスト

#### ✅ 基本機能テスト
```bash
# ヘルプ表示
./cscp_exec -help

# X1 Turbo + Alpha.2d フロッピーディスクマウント
./cscp_exec -machine x1turbo -fdd0 "temp/emulator/x1turboROM/DISK/Alpha (X1turbo).2d"
# 出力: "FDD0 マウント成功: temp/emulator/x1turboROM/DISK/Alpha (X1turbo).2d"

# X1 Turbo + Mario Bros. Special.tap テープマウント
./cscp_exec -machine x1turbo -tape0 "temp/emulator/x1turboROM/TAPE/Mario Bros. Special.tap"
# 出力: "TAPE0 マウント成功: temp/emulator/x1turboROM/TAPE/Mario Bros. Special.tap"
```

#### ✅ 複数メディア同時マウントテスト
```bash
# フロッピーディスク＋テープの同時マウント
./cscp_exec -machine x1turbo \
  -fdd0 "temp/emulator/x1turboROM/DISK/Alpha (X1turbo).2d" \
  -tape0 "temp/emulator/x1turboROM/TAPE/Mario Bros. Special.tap"
# 出力: 両方のメディアが正常にマウント成功
```

### 🏗️ アーキテクチャ概要

#### 1. コマンドライン引数解析
```cpp
typedef struct {
    char machine_name[256];        // 機種名
    char fdd_paths[4][512];        // フロッピーディスクドライブ0-3
    char tape_paths[2][512];       // テープドライブ0-1
    char hdd_paths[4][512];        // ハードディスクドライブ0-3
    char cart_paths[4][512];       // カートリッジスロット0-3
    bool show_help;                // ヘルプ表示フラグ
} emulator_args_t;
```

#### 2. メディアマウント処理
```cpp
void mount_media(EMU* emu, const emulator_args_t* args)
{
    // フロッピーディスク、テープ、ハードディスク、カートリッジの
    // 各メディアタイプに対してEMUクラスの対応メソッド呼び出し
    // emu->open_floppy_disk(drive, path, bank);
    // emu->play_tape(drive, path);
    // emu->open_hard_disk(drive, path);
    // emu->open_cart(drive, path);
}
```

#### 3. Apple専用パス変換
```cpp
void convert_to_tchar(const char* src, _TCHAR* dest, size_t dest_size)
{
    // char* から _TCHAR* への変換処理
    // Unicode/マルチバイト文字列対応
}
```

### 🔧 今後の拡張予定

1. **Core Audio統合** ← 次の実装
   - 音声出力システム
   - リアルタイム音声処理
   - AudioUnit/AVAudioEngine統合

2. **設定ファイル自動読み込み**
   - 機種別ini ファイル処理
   - `temp/emulator/x1turboROM/x1turbo.ini` 自動読み込み

3. **Apple パス管理**
   - macOS: `~/Library/Application Support/CSCP/`
   - iOS/iPadOS: `Documents/CSCP/`
   - プラットフォーム標準フォルダ対応

4. **GUI機能強化**
   - メニューバー実装
   - ファイル選択ダイアログ
   - 設定画面UI
   - フルスクリーン対応

### 📋 コマンドライン使用方法

#### 基本的な使用方法
```bash
./cscp_exec [オプション]

# オプション:
-machine <機種名>        # エミュレート機種 (デフォルト: x1turbo)
-fdd0 <パス>             # フロッピーディスクドライブ0にマウント
-fdd1 <パス>             # フロッピーディスクドライブ1にマウント
-fdd2 <パス>             # フロッピーディスクドライブ2にマウント
-fdd3 <パス>             # フロッピーディスクドライブ3にマウント
-tape0 <パス>            # テープドライブ0にマウント
-tape1 <パス>            # テープドライブ1にマウント
-hdd0 <パス>             # ハードディスクドライブ0にマウント
-hdd1 <パス>             # ハードディスクドライブ1にマウント
-hdd2 <パス>             # ハードディスクドライブ2にマウント
-hdd3 <パス>             # ハードディスクドライブ3にマウント
-cart0 <パス>            # カートリッジスロット0にマウント
-cart1 <パス>            # カートリッジスロット1にマウント
-cart2 <パス>            # カートリッジスロット2にマウント
-cart3 <パス>            # カートリッジスロット3にマウント
-help, --help            # ヘルプを表示
```

#### 使用例
```bash
# コンソール版: X1 Turbo基本起動
./build_x1turbo/cscp_exec -machine x1turbo -fdd0 "temp/emulator/x1turboROM/DISK/Alpha (X1turbo).2d"

# GUI版: X1 Turbo基本起動（Metal描画）
./build_x1turbo_gui/cscp_exec -machine x1turbo -fdd0 "temp/emulator/x1turboROM/DISK/Alpha (X1turbo).2d"

# MSX1でカートリッジゲーム起動
./cscp_exec -machine msx1 -cart0 "temp/emulator/msx1ROM/CART/Gradius.rom"

# 複数メディア同時マウント
./cscp_exec -machine x1turbo \
  -fdd0 "temp/emulator/x1turboROM/DISK/Xanadu (Disk A).d88" \
  -fdd1 "temp/emulator/x1turboROM/DISK/Xanadu (Disk B).d88" \
  -tape0 "temp/emulator/x1turboROM/TAPE/Mario Bros. Special.tap"
```

#### ビルド方法
```bash
# コンソール版ビルド
./build_machine.sh -m x1turbo

# GUI版ビルド（Metal描画付き）
./build_machine_gui.sh -m x1turbo
```

### 🚀 プロジェクト進捗

**レトロパソコンエミュレータの macOS/iOS/iPad 移植プロジェクトが大きく前進しました！**

- ✅ 基盤となるOSDレイヤー実装完了
- ✅ コマンドライン引数による柔軟なメディアマウント機能実装完了
- ✅ 実際のゲームディスクとテープの動作確認完了
- ✅ **Metal API統合完了** - ネイティブGPU描画実現！
- ✅ **GUI版アプリケーション実装完了** - macOSネイティブウィンドウ対応！
- 🔄 次フェーズ：Core Audio統合（音声出力実装）

### 📝 注意事項

- `temp/` フォルダは.gitignoreに追加されているため、Gitには含まれません
- ROMファイルは著作権保護されているため、個人で所有しているものを使用してください
- 現在はコマンドライン版での動作確認段階です
- GUI版の実装は次の開発フェーズで予定されています

---

**Medamap & Claude** - 2024年5月29日更新