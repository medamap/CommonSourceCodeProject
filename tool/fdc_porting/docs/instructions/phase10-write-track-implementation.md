# Phase 10: Write Track実装エージェント指示書

## エージェント名
WriteTrackAgent-Phase10

## 作業目的
MB8877互換実装において未実装のWrite Track機能（ディスクフォーマット）を完全実装する

## 前提情報
- Phase 1-9完了済み（基本的な読み書きは動作）
- Write Trackのデータ処理部分が未実装（TODO状態）
- テスト環境は`tool/fdc_porting/test/`に整備済み
- 72.7%のテスト合格率を維持しつつ新機能を追加

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象ファイル（282-285行目）
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `src/vm/disk.h` - ディスクインターフェース仕様
- `tool/fdc_porting/docs/reports/phase1-completion-report.md` - 現在の実装状態
- `tool/fdc_porting/docs/write_track_implementation_plan.md` - 実装計画

## 実装項目

### 1. Write Trackデータ処理の実装（最重要）
```cpp
// 現在の状態（282-285行目）
} else if(main_state == WRITE_TRACK) {
    // Write track implementation
    // TODO: Implement write track data handling
}
```

以下を実装：
- データレジスタからのフォーマットデータ受信
- フォーマットバイトの解析（0xF5, 0xF6, 0xF7等の特殊マーカー）
- IDフィールドの書き込み（Track, Side, Sector, Length）
- データフィールドの書き込み
- CRC計算と書き込み
- ギャップバイトの処理

### 2. Write Track状態管理の改善
- `fdc[drvreg].id_written`フラグの適切な管理
- `fdc[drvreg].sector_found`フラグの更新
- セクタ書き込み位置の追跡
- フォーマット進行状況の管理

### 3. ディスクインターフェースの活用
- `disk[drvreg]->format_track()`の適切な呼び出し（利用可能な場合）
- セクタヘッダ情報の設定
- トラックバッファへの書き込み
- `disk[drvreg]->sync_buffer()`でのディスクイメージ更新

### 4. エラー処理の実装
- ライトプロテクトチェック（既存）
- タイミングエラーの検出
- 不正なフォーマットデータの処理

## テスト作成

### 1. 新規テストファイル作成
`tool/fdc_porting/test/test_mb8877_write_track.cpp`:
- 基本的なトラックフォーマットテスト
- 各種セクタサイズ（128, 256, 512, 1024）でのテスト
- 不正なフォーマットデータのテスト
- CRC検証テスト

### 2. 既存テストへの追加
`tool/fdc_porting/test/test_mb8877_type3_commands.cpp`に追加：
- Write Trackコマンドの詳細テスト
- フォーマット後の読み取り検証

## 作業手順

1. オリジナルmb8877.cppのWrite Track実装を詳細分析
2. mb8877_compat.cppの該当箇所にWrite Track処理を実装
3. 必要に応じてヘルパー関数を追加
4. テストケースを作成・実行
5. 既存テストの回帰確認（72.7%維持）
6. フォーマット機能の実機相当動作確認

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - Write Track実装追加
   - `src/vm/mb8877_compat.h` - 必要に応じて追加定義

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_write_track.cpp` - 新規
   - `tool/fdc_porting/test/test_mb8877_type3_commands.cpp` - 更新

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase10-write-track-report.md`

## レポート記載事項
- 実装日時と作業時間
- 実装したWrite Track機能の詳細仕様
- テスト結果（新規追加分と既存回帰）
- フォーマットデータ形式の解説
- 既知の制限事項
- Phase 11への引き継ぎ事項

## 注意事項
- オリジナルmb8877.cppとの互換性を最優先
- フォーマットデータの仕様（IBM Format）を正確に実装
- CRC計算は既存のdisk.cppの機能を活用
- テスト環境での動作確認を徹底
- コードコメントでフォーマット処理を詳細説明