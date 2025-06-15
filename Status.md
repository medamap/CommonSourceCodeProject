# MB8877 FDC Compatibility Project - Status

最終更新: 2025-01-16

## 現在の状態

### 作業中のタスク
- [x] mb8877.cpp（オリジナル）と mb8877_compat.cpp（互換実装）の動作ログ比較
  - 状態: 両方のログ採取完了
  - 結果: compat版はセクタ1を繰り返し読み、次に進まない

### 発見した問題
- **~~セクタ読み込み完了時のIRQが設定されない~~** → 解決済み
  - IRQは正しく設定されている
  - しかし、同じセクタ1を繰り返し読んでいる
  
### 新たな問題
- **~~IPLプロセスが進まない~~** → 原因特定・修正済み
  - セクタ1は正常に読めてIRQも発生
  - しかし、次のセクタ/トラックに進まず、同じセクタを繰り返す
  - **原因: STEP コマンドの実装バグ**
    - compat版: `seekvct = false` の時、ヘッドが移動しない（`seektrk = fdc[drvreg].track`）
    - オリジナル: `seekvct` に基づいて必ず IN または OUT に移動
  - **修正内容**:
    - `cmd_step()`: seekvct に基づいて正しく IN/OUT 方向に移動するよう修正
    - `cmd_stepin()`: `seekvct = false` に修正（IN方向を示す）
    - `cmd_stepout()`: `seekvct = true` のまま（OUT方向を示す）

### 現在の問題
- **ブートプロセスが進まない**
  - セクタ1のデータは正しく読めている（"turbo ALPHA"）
  - 同じセクタを繰り返し読んでいる
  - マルチセクタ読み込みに進まない
  - **~~根本原因: RESTORE コマンドでのトラックレジスタ初期化の違い~~** → 修正済み
  - **新たな根本原因: STEP コマンドの実装バグ** → 修正済み

## ToDoリスト

### 高優先度
- [ ] STEP コマンド修正後のテスト
  - mb8877_compat.cpp でビルドして動作確認
  - ブートプロセスが次のトラックに進むか確認
- [ ] オリジナル mb8877.cpp でのログ採取（-D__MB8877_COMPAT でビルド）
- [ ] 互換実装 mb8877_compat.cpp でのログ採取（-D_MB8877_COMPAT でビルド）
- [ ] 両ログの差異分析
- [ ] マルチセクタ読み込み処理の修正

### 中優先度
- [ ] cmd_readdata_end() と cmd_writedata_end() の削除（デッドコード）
- [ ] sector_changed フラグの使用方法見直し
- [ ] テストコードの拡充（マルチセクタ読み込みテスト）

### 低優先度
- [ ] コードのクリーンアップ
- [ ] コメントの整理
- [ ] ドキュメントの更新

## 完了したタスク

### 2025-01-16
- [x] STEP コマンドの実装バグ修正
  - `cmd_step()`: seekvct が false の時、ヘッドが移動しない問題を修正
  - `cmd_stepin()`: seekvct を false に設定（IN方向）
  - `cmd_stepout()`: seekvct を true に設定（OUT方向）
  - オリジナルと同じ動作になるよう修正

### 2025-06-16
- [x] RESTORE コマンドの修正
  - `trkreg = 0xff` に初期化するよう修正
  - EVENT_SEEK でトラックレジスタを物理トラック位置で更新するよう修正
  - オリジナルの動作と同じになるよう調整

### 2025-06-15
- [x] mb8877.cpp と mb8877_compat.cpp に同一ログポイント設置
  - コマンド実行、セクタレジスタ書き込み、READ SECTOR開始
  - データ読み込み、マルチセクタ処理、セクタ読み込み完了
- [x] ログタグの差別化（MB8877_ORIG / MB8877_COMPAT）
- [x] CMakeLists.txt での切り替え設定（-D_MB8877_COMPAT / -D__MB8877_COMPAT）
- [x] DRQ処理の修正（無条件設定）
- [x] セクタデータ読み込みの修正（get_sector再呼び出し）
- [x] インデックス判定の修正（(index + 1) >= count）

## 次に行うべきこと

1. **ログ採取と比較**
   ```bash
   # オリジナル版ビルド（現在の設定）
   cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject/androidstudio/buildbatch
   ./allexecute.sh -m ReleaseBuild -d x1turbo
   # ログ採取 → temp/mb8877_orig_log.txt
   ```

2. **互換版への切り替え**
   - CMakeLists.txt: `-D__MB8877_COMPAT` → `-D_MB8877_COMPAT`
   - ビルド、実行、ログ採取 → temp/mb8877_compat_log.txt

3. **ログ比較分析**
   - 特に EVENT_MULTI1/MULTI2 の発生有無
   - secreg の増加タイミング
   - cmd_readdata() の再呼び出し

## メモ

### X1 特有の動作
- セクタレジスタに85（0x55）を書き込むが、実際はセクタ1を読む
- モータースピンアップ：560ms
- ディスクフォーマット：2D

### 重要なファイル
- FDC実装: `src/vm/mb8877.cpp`, `src/vm/mb8877_compat.cpp`
- X1 FDC制御: `src/vm/x1/floppy.cpp`
- ディスク処理: `src/vm/disk.cpp`
- CMake設定: `androidstudio/app/src/main/cpp/CMakeLists.txt`