# Phase 15: 残りのビルドエラー修正エージェント指示書

## エージェント名
BuildFixAgent-Phase15

## 作業目的
test_mb8877_error_handlingを含む、get_intr_ack()を使用する全てのテストのビルドエラーを修正する

## 問題の詳細
- エラー: `Undefined symbols for architecture arm64: "MB8877::get_intr_ack()"`
- 影響テスト: test_mb8877_error_handling（および他の可能性あり）
- 原因: mb8877_test_wrapper.o使用によるget_intr_ack()メソッドの欠如

## 参照すべきファイル
- `tool/fdc_porting/test/Makefile` - 修正対象
- `tool/fdc_porting/test/test_mb8877_error_handling.cpp` - get_intr_ack()使用確認
- `tool/fdc_porting/test/test_mb8877_timing.cpp` - get_intr_ack()使用確認
- `tool/fdc_porting/test/test_mb8877_drive_mfm.cpp` - get_intr_ack()使用確認
- `tool/fdc_porting/test/test_mb8877_drive_rpm.cpp` - get_intr_ack()使用確認

## 修正項目

### 1. get_intr_ack()を使用する全テストの特定
- grep等でget_intr_ack()を使用する全てのテストファイルを検索
- 各テストがどのメソッドを必要としているか確認

### 2. Makefileの系統的修正
Phase 14で修正した方法を参考に：
- get_intr_ack()を使用するテストはmb8877_compat_wrapper.oを使用
- 使用しないテストは従来通りMB8877_OBJSを使用

### 3. 修正対象の可能性があるテスト
- test_mb8877_error_handling（確実）
- test_mb8877_timing（可能性あり）
- test_mb8877_write_track（可能性あり）
- test_mb8877_drive_mfm（可能性あり）
- test_mb8877_drive_rpm（可能性あり）
- test_mb8877_mb89311（可能性あり）

### 4. 一括修正戦略
1. 全テストファイルでget_intr_ack()使用を確認
2. 使用するテストのリストを作成
3. Makefileで該当テストのビルドルールを一括修正
4. ビルド確認

## テスト項目

### 1. ビルド確認
- `make clean && make all`で全テストのクリーンビルド
- リンクエラーが完全に解消されることを確認

### 2. テスト実行確認
- `./run_all_tests.sh`で全テスト実行
- ビルド問題以外のテスト失敗を記録

## 作業手順

1. get_intr_ack()を使用する全テストファイルを特定
2. Makefileで該当テストのビルドルールを修正
3. mb8877_compat_wrapper.oを使用するよう変更
4. 全テストのビルド確認
5. テスト実行で動作確認

## 成果物

1. 修正ファイル:
   - `tool/fdc_porting/test/Makefile` - ビルドルール修正

2. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase15-build-fix-remaining-report.md`

## レポート記載事項
- get_intr_ack()を使用する全テストのリスト
- 実施した修正内容（Makefileの変更箇所）
- ビルド結果（全テスト成功確認）
- テスト実行結果の概要
- 今後の保守に向けた推奨事項

## 注意事項
- 全テストを網羅的に確認
- 修正の一貫性を保つ
- 将来のテスト追加時の指針を残す
- ビルドシステムの保守性を考慮