# Phase 12b: set_track_size()実装エージェント指示書

## エージェント名
DriveSettingsAgent-Phase12b

## 作業目的
MB8877互換実装においてset_track_size()メソッドを実装し、カスタムトラックサイズ設定機能を完全動作させる

## 前提情報
- Phase 12a完了済み（set_drive_mfm()実装済み）
- 現在のset_track_size()は空実装（1203-1209行目）
- 非標準トラックフォーマット対応が目的
- 特殊ディスクやコピープロテクト対応に必要

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象（1203-1209行目）
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `src/vm/disk.h` - ディスクインターフェース定義
- `tool/fdc_porting/docs/reports/phase12a-drive-mfm-report.md` - Phase 12a完了状況
- `tool/fdc_porting/test/test_mb8877_drive_mfm.cpp` - MFM設定テスト参考

## 実装項目

### 1. set_track_size()の実装（1203-1209行目）
現在の状態:
```cpp
void MB8877::set_track_size(int drv, int size)
{
    // Mock implementation - no actual hardware control needed
    if(drv < MAX_DRIVE && disk[drv]) {
        // In test environment, track size setting is not needed
    }
}
```

実装すべき内容:
- DISKオブジェクトのトラックサイズ設定
- サイズ範囲チェック（最小/最大値）
- 既存フォーマットとの整合性確認
- エラーハンドリング（無効サイズ、範囲外ドライブ等）
- デバッグログ出力

### 2. トラックサイズの仕様
- 標準サイズ: 6250バイト（2D/2DD）, 12500バイト（2HD）
- 最小サイズ: 1024バイト
- 最大サイズ: 65536バイト
- 特殊ディスク対応: FM7/X1の特殊フォーマット

### 3. 関連機能との連携
- タイミング計算への反映
- フォーマット処理での考慮
- 位置計算の更新

## テスト作成

### 1. 新規テストファイル作成
`tool/fdc_porting/test/test_mb8877_track_size.cpp`:
- 基本的なトラックサイズ設定テスト
- サイズ範囲チェックテスト
- 各ドライブでの個別設定テスト
- 無効値での処理テスト
- 特殊サイズでの動作確認

### 2. 統合テスト
- MFM設定との組み合わせテスト
- タイミング計算への影響確認

## 作業手順

1. オリジナルmb8877.cppでのトラックサイズ制御を分析
2. disk.hのトラックサイズ関連インターフェースを確認
3. set_track_size()メソッドを実装
4. サイズ検証とエラーハンドリングを追加
5. テストケースを作成・実行
6. 既存機能との統合確認

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - set_track_size()実装
   - `src/vm/mb8877_compat.h` - 必要に応じて追加定義

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_track_size.cpp` - 新規

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase12b-track-size-report.md`

## レポート記載事項
- 実装日時と作業時間
- set_track_size()の実装仕様詳細
- サポートするトラックサイズ範囲
- エラーハンドリングの動作確認
- 既存機能への影響確認
- Phase 12cへの引き継ぎ事項

## 注意事項
- トラックサイズの妥当性チェックを確実に実装
- メモリ使用量への影響を考慮
- 既存のタイミング計算との整合性確保
- 特殊ディスク対応の拡張性を考慮
- エラー時の適切な処理を実装