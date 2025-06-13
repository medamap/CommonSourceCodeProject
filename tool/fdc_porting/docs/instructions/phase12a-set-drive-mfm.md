# Phase 12a: set_drive_mfm()実装エージェント指示書

## エージェント名
DriveSettingsAgent-Phase12a

## 作業目的
MB8877互換実装においてset_drive_mfm()メソッドを実装し、FM/MFM（単密度/倍密度）切り替え機能を完全動作させる

## 前提情報
- Phase 10-11完了済み（Write Track実装、タイミング精度向上済み）
- 現在のset_drive_mfm()は空実装（1071-1077行目）
- FM: 125kbps転送、MFM: 250kbps転送の切り替えが必要
- ディスクフォーマットとデータ転送レートに影響

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象（1071-1077行目）
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `src/vm/disk.h` - ディスクタイプ定義
- `tool/fdc_porting/docs/reports/phase11-timing-report.md` - Phase 11完了状況
- `tool/fdc_porting/test/test_mb8877_timing.cpp` - タイミングテスト参考

## 実装項目

### 1. set_drive_mfm()の実装（1071-1077行目）
現在の状態:
```cpp
void MB8877::set_drive_mfm(int drv, bool mfm)
{
    // Mock implementation - no actual hardware control needed
    if(drv < MAX_DRIVE && disk[drv]) {
        // In test environment, MFM setting is not needed
    }
}
```

実装すべき内容:
- DISKオブジェクトのMFM/FMモード設定
- 転送レート設定（FM: 125kbps, MFM: 250kbps）
- バイト転送タイミングの更新
- トラックフォーマット情報の更新
- エラーチェック（範囲外ドライブ番号等）

### 2. 関連する内部状態の管理
- ドライブごとのMFM/FMフラグ保持
- タイミング計算への反映
- フォーマット処理での密度考慮

### 3. 既存機能との統合
- get_usec_to_next_trans_pos()でのMFM/FM切り替え対応（既存）
- Write Track処理でのフォーマット密度反映
- 読み書き処理での転送レート適用

## テスト作成

### 1. 新規テストファイル作成
`tool/fdc_porting/test/test_mb8877_drive_mfm.cpp`:
- 基本的なMFM/FM切り替えテスト
- 転送レート変更の検証
- 各ドライブでの個別設定テスト
- 無効なドライブ番号での処理テスト

### 2. 既存テストの更新
`tool/fdc_porting/test/test_mb8877_timing.cpp`に追加:
- MFM/FMモードでのタイミング検証
- 密度切り替え後の読み書きテスト

## 作業手順

1. オリジナルmb8877.cppでのMFM制御方法を分析
2. disk.hのMFM関連インターフェースを確認
3. set_drive_mfm()メソッドを実装
4. 必要に応じてヘルパー関数を追加
5. テストケースを作成・実行
6. タイミング関数との連携確認

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - set_drive_mfm()実装
   - `src/vm/mb8877_compat.h` - 必要に応じて追加定義

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_drive_mfm.cpp` - 新規
   - `tool/fdc_porting/test/test_mb8877_timing.cpp` - 更新（必要に応じて）

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase12a-drive-mfm-report.md`

## レポート記載事項
- 実装日時と作業時間
- set_drive_mfm()の実装仕様詳細
- FM/MFMモード切り替えの動作確認
- 転送レート変更の検証結果
- 既存機能への影響確認
- Phase 12bへの引き継ぎ事項

## 注意事項
- FM: 単密度（125kbps）、MFM: 倍密度（250kbps）の正確な実装
- ドライブ番号の範囲チェックを確実に実施
- 既存のタイミング計算機能との整合性確保
- テスト環境での動作確認を徹底
- 実機での密度切り替え仕様に準拠