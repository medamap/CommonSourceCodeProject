# Phase 21: セグメンテーションフォルト修正エージェント指示書

## エージェント名
SegfaultDebugAgent-Phase21

## 作業目的
Phase 20で発見された3つのセグメンテーションフォルト（register/Type1/Type2テスト）を詳細に分析し、根本原因を特定して修正する

## 前提情報
- Phase 20で14テスト中3テストがセグフォルト
- 影響箇所: test_mb8877_registers, test_mb8877_type1_commands, test_mb8877_type2_commands
- 実装完成度75%だが、基本機能でクラッシュが発生
- コンパイルエラーは解消済み

## 参照すべきファイル
- `tool/fdc_porting/test/test_mb8877_registers.cpp` - レジスタテスト
- `tool/fdc_porting/test/test_mb8877_type1_commands.cpp` - Type1テスト
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - Type2テスト
- `src/vm/mb8877_compat.cpp` - 実装本体
- `tool/fdc_porting/test/mock_environment.h` - モック環境
- `tool/fdc_porting/test/mb8877_compat_wrapper.cpp` - ラッパー

## デバッグ項目

### 1. セグフォルト詳細分析
各失敗テストについて：

#### test_mb8877_registers
- クラッシュ箇所: レジスタアクセス時
- 疑問点: read_io8/write_io8の実装
- 確認事項: アドレス範囲チェック、NULL参照

#### test_mb8877_type1_commands  
- クラッシュ箇所: シークコマンド実行時
- 疑問点: cmd_restore/cmd_seek実装
- 確認事項: disk配列アクセス、イベント処理

#### test_mb8877_type2_commands
- クラッシュ箇所: データ転送コマンド実行時  
- 疑問点: cmd_readdata/cmd_writedata実装
- 確認事項: ディスクアクセス、バッファ操作

### 2. 使用するデバッグ手法

#### GDBスタックトレース
```bash
gdb ./test_mb8877_registers
(gdb) run
(gdb) bt
(gdb) info registers
(gdb) list
```

#### Valgrindメモリ解析
```bash
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./test_mb8877_registers
```

#### AddressSanitizerビルド
```bash
# Makefileに-fsanitize=address追加
make clean && make CFLAGS="-fsanitize=address -g" test_mb8877_registers
```

### 3. 修正対象の候補

#### 初期化不備
- disk配列の初期化
- fdc構造体の初期化
- イベントハンドラの初期化

#### NULL参照
- disk[drvreg]アクセス前のチェック
- コールバック関数の存在確認
- メモリ割り当て失敗のチェック

#### 配列範囲外アクセス
- drvreg値の範囲チェック（MAX_DRIVE）
- アドレス引数の検証（addr & 3）
- インデックス変数の境界チェック

#### Mock環境の問題
- モック実装の不完全性
- テストフレームワークとの不整合
- スタブ関数の戻り値

## 修正手順

### Phase 1: 詳細診断
1. 各セグフォルトのスタックトレースを取得
2. クラッシュ直前の変数状態を確認
3. 呼び出しパスを追跡

### Phase 2: 根本原因特定
1. 共通する問題パターンを特定
2. 初期化シーケンスを確認
3. メモリ管理の問題を特定

### Phase 3: 修正実装
1. 最も影響の大きい問題から修正
2. 修正後の個別テスト
3. 回帰テストの実行

### Phase 4: 検証
1. 修正した3テストの再実行
2. 他のテストへの影響確認
3. 安定性の確認

## デバッグコマンド例

```bash
# 基本的なセグフォルト情報取得
cd tool/fdc_porting/test

# GDBでの詳細解析
gdb ./test_mb8877_registers
(gdb) set environment MALLOC_CHECK_=3
(gdb) run
(gdb) bt full
(gdb) print *this
(gdb) x/10x $rsp

# Valgrindでのメモリ問題検出
valgrind --tool=memcheck --vgdb=yes --vgdb-error=0 ./test_mb8877_registers

# コアダンプ解析（システム設定による）
ulimit -c unlimited
./test_mb8877_registers
gdb ./test_mb8877_registers core
```

## 成果物

### 1. デバッグレポート（Markdownファイル）
- `tool/fdc_porting/docs/reports/phase21-segfault-debug-report.md`

レポート構成：
```markdown
# Phase 21: セグフォルト修正レポート

## 実行サマリー
- 解析対象: 3テスト
- 修正完了: X/3
- 根本原因: [詳細]

## セグフォルト詳細分析
### test_mb8877_registers
- スタックトレース: [詳細]
- 原因: [特定された問題]
- 修正内容: [実施した修正]

### test_mb8877_type1_commands
- スタックトレース: [詳細]  
- 原因: [特定された問題]
- 修正内容: [実施した修正]

### test_mb8877_type2_commands
- スタックトレース: [詳細]
- 原因: [特定された問題]
- 修正内容: [実施した修正]

## 修正後テスト結果
- 成功テスト数の変化
- セグフォルト解消状況
- 新たに発見された問題

## 今後の課題
- 残る安定性問題
- パフォーマンス最適化の必要性
- Phase 22への引き継ぎ事項
```

### 2. 標準出力（JSON形式）
実行完了時に以下のJSON形式で出力：

```json
{
  "phase": 21,
  "task": "segfault_debugging",
  "execution_status": "completed",
  "segfaults_analyzed": 3,
  "fixes_applied": {
    "test_mb8877_registers": "success/partial/failed",
    "test_mb8877_type1_commands": "success/partial/failed", 
    "test_mb8877_type2_commands": "success/partial/failed"
  },
  "root_causes_identified": [
    "primary_cause_description",
    "secondary_cause_description"
  ],
  "code_changes": [
    {
      "file": "src/vm/mb8877_compat.cpp",
      "lines": "XXX-XXX", 
      "change_type": "null_check_added/initialization_fixed/bounds_check_added",
      "description": "specific_fix_description"
    }
  ],
  "test_results_after_fix": {
    "previously_segfaulting_tests": "X/3 now passing",
    "regression_check": "no_new_failures/X_new_issues",
    "overall_improvement": "X% -> Y%"
  },
  "phase22_readiness": "ready/needs_more_work",
  "next_priority_issues": [
    "remaining_critical_issue_1",
    "remaining_critical_issue_2"
  ]
}
```

## 成功基準

### 最低基準
- 3テスト中2テスト以上でセグフォルト解消
- 根本原因の特定
- 他テストでの回帰なし

### 理想基準  
- 3テスト全てでセグフォルト解消
- 共通する問題パターンの修正
- 全体的な安定性向上

## 注意事項
- デバッグ中は安全な環境で実行
- バックアップを取ってから修正作業
- 小さな修正を段階的に適用
- 各修正後にテストを実行
- パフォーマンスより安定性を重視
- コメントで修正理由を明記