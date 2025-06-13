# Phase 14: ビルドエラー修正エージェント指示書

## エージェント名
BuildFixAgent-Phase14

## 作業目的
テストビルドで発生したリンクエラーを修正し、全テストが正常にビルドできるようにする

## 問題の詳細
- エラー: `Undefined symbols for architecture arm64: "MB8877::get_intr_ack()"`
- 原因: mb8877_test_wrapper.cppがget_intr_ack()メソッドを含んでいない
- 影響: test_mb8877_type4_commandsがビルドできない

## 参照すべきファイル
- `tool/fdc_porting/test/mb8877_test_wrapper.cpp` - 修正対象
- `src/vm/mb8877_compat.cpp` - get_intr_ack()実装確認（1867-1873行目）
- `src/vm/mb8877.h` - メソッド定義確認
- `tool/fdc_porting/test/test_mb8877_type4_commands.cpp` - エラー発生箇所

## 修正項目

### 1. mb8877_test_wrapper.cppの確認と修正
- 現在の#defineマクロ確認
- get_intr_ack()が除外されていないか確認
- 必要に応じてmb8877.cppのインクルード範囲を調整

### 2. 考えられる原因と対策
1. **マクロによる除外**
   - #ifdef条件でget_intr_ack()が除外されている可能性
   - 必要なマクロ定義を追加

2. **インクルード順序**
   - mb8877.hとmb8877.cppのインクルード順序確認
   - mock_environment.hとの競合確認

3. **メソッド実装の確認**
   - オリジナルmb8877.cppでのget_intr_ack()実装確認
   - mb8877_compat.cppでの実装確認

### 3. 修正手順
1. mb8877_test_wrapper.cppを調査
2. get_intr_ack()メソッドが含まれるよう修正
3. 他のwrapperファイル（mb8877_compat_wrapper.cpp等）も同様に確認
4. Makefileの依存関係確認
5. テストビルドで検証

## テスト項目

### 1. ビルド確認
- `make clean && make all`で全テストのビルド確認
- リンクエラーの解消確認

### 2. テスト実行確認
- 修正後のテスト実行確認
- 既存テストへの影響確認

## 作業手順

1. mb8877_test_wrapper.cppの内容確認
2. get_intr_ack()メソッドのインクルード確認
3. 必要な修正を実施
4. 全wrapperファイルの一貫性確認
5. ビルドテスト実行
6. 全テストの動作確認

## 成果物

1. 修正ファイル:
   - `tool/fdc_porting/test/mb8877_test_wrapper.cpp` - 修正
   - `tool/fdc_porting/test/mb8877_compat_wrapper.cpp` - 確認/修正
   - `tool/fdc_porting/test/mb8877_original_wrapper.cpp` - 確認/修正

2. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase14-build-fix-report.md`

## レポート記載事項
- 問題の原因詳細
- 実施した修正内容
- ビルド結果
- テスト実行結果
- 今後の予防策

## 注意事項
- 既存のテストに影響を与えないよう注意
- モック環境との整合性を維持
- 全てのwrapperファイルで一貫性を保つ
- ビルドシステムの健全性を確認