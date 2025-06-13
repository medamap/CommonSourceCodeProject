# Phase 12c: set_drive_rpm() 実装レポート

## 実装日時
- 日付: 2025/01/13
- 作業時間: 約45分

## 実装内容

### 1. set_drive_rpm() メソッドの実装

`src/vm/mb8877_compat.cpp` の1167-1203行目に実装:

```cpp
void MB8877::set_drive_rpm(int drv, int rpm)
{
    // ドライブ番号検証
    if(drv < 0 || drv >= MAX_DRIVE) {
        this->out_debug_log(_T("FDC: set_drive_rpm: invalid drive number %d"), drv);
        return;
    }
    
    // RPM範囲検証 (240-400 RPM)
    if(rpm < 240 || rpm > 400) {
        this->out_debug_log(_T("FDC: set_drive_rpm: invalid RPM %d (valid range: 240-400)"), rpm);
        return;
    }
    
    // ディスク挿入確認
    if(!disk[drv]) {
        this->out_debug_log(_T("FDC: set_drive_rpm: no disk in drive %d"), drv);
        return;
    }
    
    // RPM設定
    disk[drv]->drive_rpm = rpm;
    
    // 標準RPM識別とログ出力
    const char* rpm_type = "";
    if(rpm == 300) {
        rpm_type = " (5.25\" standard)";
    } else if(rpm == 360) {
        rpm_type = " (3.5\" standard)";
    }
    
    this->out_debug_log(_T("FDC: set_drive_rpm: drive %d set to %d RPM%s"), drv, rpm, rpm_type);
    
    // 回転時間計算とログ出力
    double rotation_time_us = 60000000.0 / rpm;
    this->out_debug_log(_T("FDC: drive %d rotation time: %.0f microseconds"), drv, rotation_time_us);
}
```

### 2. 実装仕様詳細

#### サポートするRPM範囲
- 最小値: 240 RPM
- 最大値: 400 RPM
- 標準値:
  - 300 RPM: 5.25インチドライブ標準
  - 360 RPM: 3.5インチドライブ標準

#### 妥当性チェック
1. ドライブ番号範囲チェック (0 ～ MAX_DRIVE-1)
2. RPM値範囲チェック (240 ～ 400)
3. ディスク挿入状態チェック

#### エラーハンドリング
- 無効なドライブ番号: ログ出力して終了
- 範囲外のRPM値: ログ出力して終了
- ディスク未挿入: ログ出力して終了

### 3. タイミング計算への影響

RPM設定は以下のタイミング計算に直接影響:

1. **1回転時間**
   - 計算式: 60,000,000 / RPM マイクロ秒
   - 300 RPM: 200,000 マイクロ秒
   - 360 RPM: 166,666.67 マイクロ秒

2. **バイト転送時間**
   - disk->get_usec_per_bytes() で使用
   - トラックサイズとRPMから計算

3. **インデックスホール検出時間**
   - get_usec_to_detect_index_hole() で使用
   - 正確な回転タイミング制御

## Phase 12シリーズ全体の統合結果

### 実装完了機能
1. **Phase 12a: set_drive_mfm()**
   - FM/MFMモード切り替え
   - 転送レート自動調整

2. **Phase 12b: set_track_size()** 
   - カスタムトラックサイズ設定
   - 1,024～65,536バイト範囲

3. **Phase 12c: set_drive_rpm()**
   - ドライブ回転速度設定
   - 240～400 RPM範囲

### 統合動作確認
- 各機能が独立して動作
- 組み合わせ使用時の相互影響なし
- タイミング計算の整合性維持

## テスト作成

### テストファイル
`tool/fdc_porting/test/test_mb8877_drive_rpm.cpp` を作成:

1. **基本的なRPM設定テスト**
   - 標準RPM値の設定
   - カスタムRPM値の設定

2. **RPM範囲検証テスト**
   - 最小/最大境界値
   - 範囲外値のハンドリング

3. **標準RPM値確認テスト**
   - 300 RPM (5.25")
   - 360 RPM (3.5")

4. **個別ドライブ設定テスト**
   - 各ドライブへの独立設定
   - 設定値の保持確認

5. **無効値ハンドリングテスト**
   - 無効ドライブ番号
   - ディスク未挿入状態

6. **タイミング計算影響テスト**
   - 回転時間計算
   - バイト転送時間計算

7. **Phase 12統合テスト**
   - MFM/FM設定との組み合わせ
   - トラックサイズとの組み合わせ

8. **境界値テスト**
   - 全境界値の詳細検証

## 最終的な実装完成度

### 完成した機能
- ✅ 基本的なRPM設定機能
- ✅ RPM範囲検証
- ✅ 標準RPM値の識別
- ✅ エラーハンドリング
- ✅ デバッグログ出力
- ✅ タイミング計算への統合

### 制限事項
- 実機のドライブ速度変動は考慮外
- モーター起動/停止時間は考慮外

## 引き継ぎ事項

### Phase 13以降への推奨事項
1. **ドライブ設定の永続化**
   - 設定値の保存/復元機能
   - コンフィグファイル対応

2. **高度なタイミング制御**
   - RPM変動のシミュレーション
   - モーター特性の再現

3. **ドライブタイプ自動判定**
   - メディアタイプからのRPM推定
   - 最適な設定の自動適用

## 結論

Phase 12cの実装により、MB8877互換実装のドライブ設定機能が完成しました。set_drive_rpm()は仕様通りに実装され、既存のタイミング計算システムと適切に統合されています。

Phase 12シリーズ全体（12a: MFM設定、12b: トラックサイズ、12c: RPM）が完了し、フレキシブルなドライブ制御が可能になりました。これらの機能により、様々なディスクフォーマットとドライブタイプに対応できる柔軟なFDCエミュレーションが実現されています。