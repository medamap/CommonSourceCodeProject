# Phase 13: MB89311拡張機能実装エージェント指示書

## エージェント名
ExtensionAgent-Phase13

## 作業目的
MB8877互換実装においてMB89311拡張機能を完全実装し、拡張モードでの高度なFDC機能を提供する

## 前提情報
- Phase 10-12完了済み（基本機能、タイミング、ドライブ設定全て実装済み）
- MB89311は富士通のMB8877拡張版チップ
- 拡張コマンド（0xFC-0xFF）の実装が必要
- 既存のcmd_format()は単純なcmd_writetrack()呼び出し

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象（1752-1758行目、715-749行目）
- `src/vm/mb8877.cpp` - オリジナルMB89311実装の参考
- `src/vm/mb8877_compat.h` - HAS_MB89311定義確認
- `tool/fdc_porting/docs/reports/phase12c-drive-rpm-report.md` - Phase 12完了状況
- `tool/fdc_porting/test/test_mb8877_type3_commands.cpp` - Type III コマンドテスト参考

## 実装項目

### 1. MB89311拡張コマンドの実装（715-749行目）
現在の実装状況確認:
- 0xFC: DELAY コマンド
- 0xFD: ASSIGN PARAMETER コマンド  
- 0xFE: ASSIGN MODE コマンド
- 0xFF: RESET コマンド（推測）

実装強化内容:
- extended_mode フラグの適切な管理
- パラメータ設定の永続化
- 拡張モード時の動作変更

### 2. cmd_format()の強化（1752-1758行目）
現在の状態:
```cpp
#ifdef HAS_MB89311
void MB8877::cmd_format()
{
    // MB89311 format command
    // Similar to write track but with specific format
    cmd_writetrack();
}
#endif
```

実装すべき内容:
- MB89311固有のフォーマット仕様実装
- 拡張パラメータの使用
- 特殊フォーマットオプション対応
- エラーハンドリングの強化

### 3. 拡張モードでの動作変更
- Read-after-seek（0x44）機能の実装確認
- Write-after-seek（0x64）機能の実装確認
- パラメータ設定による動作変更
- タイミング調整の最適化

### 4. パラメータ管理システム
- 拡張パラメータの保存・復元
- モード切り替え時の設定継承
- デフォルト値の管理

## テスト作成

### 1. 新規テストファイル作成
`tool/fdc_porting/test/test_mb8877_mb89311.cpp`:
- MB89311拡張コマンドテスト（0xFC-0xFF）
- 拡張モードでのフォーマットテスト
- Read-after-seek/Write-after-seekテスト
- パラメータ設定・復元テスト
- 拡張モードと標準モードの切り替えテスト

### 2. 既存テストの更新
- Type III コマンドテストに拡張フォーマット追加
- 統合テストでの拡張機能確認

## 作業手順

1. オリジナルmb8877.cppのMB89311実装を詳細分析
2. HAS_MB89311定義の確認と活用
3. 拡張コマンド処理の強化
4. cmd_format()の独自実装
5. パラメータ管理システムの実装
6. テストケースの作成・実行

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - MB89311拡張機能実装
   - `src/vm/mb8877_compat.h` - 必要に応じて拡張定義追加

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_mb89311.cpp` - 新規
   - `tool/fdc_porting/test/test_mb8877_type3_commands.cpp` - 更新

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase13-mb89311-report.md`

## レポート記載事項
- 実装日時と作業時間
- MB89311拡張機能の実装仕様詳細
- 拡張コマンド（0xFC-0xFF）の動作確認
- cmd_format()の独自実装内容
- 拡張モードでの機能検証結果
- 最終的な実装完成度評価
- プロジェクト全体の総括

## 注意事項
- HAS_MB89311定義での条件コンパイルを適切に使用
- 既存の標準MB8877機能への影響を回避
- 拡張機能はオプションとして実装
- オリジナル実装との互換性を最優先
- パフォーマンスへの影響を最小限に抑制

## 最終目標
- MB8877/MB89311完全互換実装の達成
- 全未実装機能の解決
- 実装完成度100%の実現
- 実用レベルでの完全動作確認