# Phase 17: 致命的メソッド実装エージェント指示書

## エージェント名
CriticalFixAgent-Phase17

## 作業目的
Phase 16で発見された3つの致命的な未実装メソッドを実装し、リンクエラーを解決する

## 前提情報
- get_head_load_delay()、cmd_readdata_end()、cmd_writedata_end()が未実装
- これらはヘッダーで宣言されているが実装がない
- リンクエラーの直接的な原因となっている

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象
- `src/vm/mb8877_compat.h` - ヘッダー定義確認（173-174行目）
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `tool/fdc_porting/docs/reports/phase16-implementation-audit-report.md` - 監査結果

## 実装項目

### 1. get_head_load_delay()の実装
呼び出し箇所：1936行目（cmd_format内）
```cpp
double get_head_load_delay();
```

実装内容：
- ヘッドロード遅延時間を返す
- ドライブタイプに応じた遅延時間
  - 2HD: 15ms (15000.0μs)
  - 2DD: 30ms (30000.0μs)
- オリジナルmb8877.cppの実装を参考

### 2. cmd_readdata_end()の実装
ヘッダー宣言：173行目
```cpp
void cmd_readdata_end();
```

実装内容：
- READ SECTORコマンドの完了処理
- ステータス更新
- 割り込み生成
- 次セクタへの移行処理（マルチセクタ時）

### 3. cmd_writedata_end()の実装
ヘッダー宣言：174行目
```cpp
void cmd_writedata_end();
```

実装内容：
- WRITE SECTORコマンドの完了処理
- セクタデータの書き込み完了処理
- ステータス更新
- 割り込み生成
- 次セクタへの移行処理（マルチセクタ時）

## 実装手順

1. オリジナルmb8877.cppで各メソッドの実装を確認
2. mb8877_compat.cppの適切な位置にメソッドを追加
3. 必要な状態管理とエラーハンドリングを実装
4. タイミング計算の正確性を確保
5. テストビルドで検証

## テスト項目

### 1. ビルド確認
- リンクエラーの解消確認
- 全テストの正常ビルド

### 2. 機能確認
- ヘッドロード遅延の正確性
- READ/WRITEコマンドの完了処理
- マルチセクタ処理の動作

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - 3メソッド実装追加

2. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase17-critical-methods-report.md`

## レポート記載事項
- 実装した3メソッドの詳細
- 各メソッドの機能説明
- ビルド結果（リンクエラー解消確認）
- 実装の正確性確認
- Phase 18への引き継ぎ事項

## 注意事項
- オリジナル実装との互換性を最優先
- タイミング計算の正確性を確保
- エラーハンドリングを適切に実装
- 既存の動作に影響を与えない
- コードコメントで実装意図を明確化