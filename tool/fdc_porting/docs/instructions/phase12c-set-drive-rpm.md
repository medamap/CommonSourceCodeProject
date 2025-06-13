# Phase 12c: set_drive_rpm()実装エージェント指示書

## エージェント名
DriveSettingsAgent-Phase12c

## 作業目的
MB8877互換実装においてset_drive_rpm()メソッドを実装し、ドライブ回転速度設定機能を完全動作させる

## 前提情報
- Phase 12a,12b完了済み（set_drive_mfm()、set_track_size()実装済み）
- 現在のset_drive_rpm()は空実装（1167-1173行目）
- 標準RPM: 300RPM（5.25"）、360RPM（3.5"）
- タイミング計算に直接影響する重要な設定

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象（1167-1173行目）
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `src/vm/disk.h` - ディスクインターフェース定義
- `tool/fdc_porting/docs/reports/phase12b-track-size-report.md` - Phase 12b完了状況
- `tool/fdc_porting/test/test_mb8877_timing.cpp` - タイミング計算参考

## 実装項目

### 1. set_drive_rpm()の実装（1167-1173行目）
現在の状態:
```cpp
void MB8877::set_drive_rpm(int drv, int rpm)
{
    // Mock implementation - no actual hardware control needed
    if(drv < MAX_DRIVE && disk[drv]) {
        // In test environment, rpm setting is not needed
    }
}
```

実装すべき内容:
- DISKオブジェクトのRPM設定
- RPM値の妥当性チェック（240-400RPM程度）
- 標準RPM値の識別（300, 360RPM）
- タイミング計算への反映
- エラーハンドリング（無効RPM、範囲外ドライブ等）
- デバッグログ出力

### 2. RPMの仕様
- 標準値: 300RPM（5.25"ドライブ）, 360RPM（3.5"ドライブ）
- 許可範囲: 240-400RPM
- 1回転時間: 60000000 / RPM マイクロ秒
- タイミング精度への影響

### 3. 関連機能との統合
- get_usec_to_detect_index_hole()での回転時間計算
- get_usec_to_start_trans()でのタイミング調整
- 位置計算の更新

## テスト作成

### 1. 新規テストファイル作成
`tool/fdc_porting/test/test_mb8877_drive_rpm.cpp`:
- 基本的なRPM設定テスト
- RPM範囲チェックテスト
- 標準RPM値での動作確認
- 各ドライブでの個別設定テスト
- 無効値での処理テスト
- タイミング計算への影響確認

### 2. 統合テスト
- MFM設定・トラックサイズとの組み合わせテスト
- 全ドライブ設定機能の統合確認

## 作業手順

1. オリジナルmb8877.cppでのRPM制御を分析
2. disk.hのRPM関連インターフェースを確認
3. set_drive_rpm()メソッドを実装
4. RPM検証とエラーハンドリングを追加
5. タイミング計算への統合を確認
6. テストケースを作成・実行

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - set_drive_rpm()実装
   - `src/vm/mb8877_compat.h` - 必要に応じて追加定義

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_drive_rpm.cpp` - 新規

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase12c-drive-rpm-report.md`

## レポート記載事項
- 実装日時と作業時間
- set_drive_rpm()の実装仕様詳細
- サポートするRPM範囲
- タイミング計算への影響確認
- Phase 12シリーズ全体の統合結果
- Phase 13または最終評価への引き継ぎ事項

## 注意事項
- RPM値の妥当性チェックを確実に実装
- タイミング計算への影響を正確に反映
- 既存のタイミング関数との整合性確保
- 標準RPM値での動作を優先
- エラー時の適切な処理を実装