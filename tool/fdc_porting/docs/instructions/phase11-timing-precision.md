# Phase 11: タイミング精度向上エージェント指示書

## エージェント名
TimingAgent-Phase11

## 作業目的
MB8877互換実装のタイミング関連関数を正確に実装し、実機相当の動作精度を達成する

## 前提情報
- Phase 10完了（Write Track実装済み）
- 現在のタイミング関数は固定値を返すスタブ実装
- ディスク回転速度は300RPM（200ms/回転）が標準
- セクタ間のタイミングが実機と異なる可能性

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象（1642-1668行目）
- `src/vm/mb8877.cpp` - オリジナルのタイミング実装
- `src/vm/disk.h` - ディスクパラメータ定義
- `tool/fdc_porting/docs/reports/phase10-write-track-report.md` - Phase 10結果
- `tool/fdc_porting/test/test_mb8877_timing.cpp` - 既存タイミングテスト

## 実装項目

### 1. get_cur_position()の正確な実装（1642-1646行目）
現在:
```cpp
return fdc[drvreg].cur_position; // Mock implementation
```

実装すべき内容:
- 前回のクロックからの経過時間を計算
- ディスク回転速度に基づく位置計算
- トラックサイズを考慮した位置の正規化
- インデックスホールからの相対位置

### 2. get_usec_to_start_trans()の実装（1648-1653行目）
現在:
```cpp
return first_sector ? 1000.0 : 500.0; // Hardcoded
```

実装すべき内容:
- 現在位置から目標セクタまでの回転時間計算
- セクタギャップ時間の考慮
- ヘッドロード時間の加算（first_sector時）
- 実際のセクタ位置に基づく計算

### 3. get_usec_to_next_trans_pos()の実装（1655-1659行目）
現在:
```cpp
return delay ? 200.0 : 100.0; // Hardcoded
```

実装すべき内容:
- 次のデータ転送位置までの正確な時間
- バイト転送レートの考慮（MFM: 31.25μs/byte, FM: 62.5μs/byte）
- セクタ境界の処理
- DMAモードでの高速転送対応

### 4. get_usec_to_detect_index_hole()の改善（1661-1668行目）
現在の簡易実装を改善:
- 現在位置からインデックスホールまでの正確な時間
- 部分回転の計算（count < 1の場合）
- モーター起動時間の考慮
- ドライブ種別による速度差対応

### 5. 補助関数の追加
- `calculate_rotation_time()` - 回転時間計算
- `get_bytes_per_track()` - トラック容量取得
- `get_transfer_rate()` - 転送レート取得
- `normalize_position()` - 位置正規化

## テスト強化

### 1. タイミング精度テストの拡張
`tool/fdc_porting/test/test_mb8877_timing.cpp`を更新:
- セクタ読み取りタイミングの検証
- 連続セクタアクセスのタイミング
- インデックスホール検出精度
- DRQ間隔の測定

### 2. パフォーマンステストの追加
- 実機相当の転送速度確認
- レイテンシ測定
- 最悪ケースのタイミング検証

## 作業手順

1. オリジナルmb8877.cppのタイミング計算を詳細分析
2. ディスクパラメータと物理特性の理解
3. 各タイミング関数を順次実装
4. 単体テストで精度検証
5. 統合テストで全体動作確認
6. パフォーマンス測定と調整

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - タイミング関数実装
   - `src/vm/mb8877_compat.h` - 必要に応じて定数追加

2. テストファイル:
   - `tool/fdc_porting/test/test_mb8877_timing.cpp` - 拡張
   - `tool/fdc_porting/test/test_mb8877_performance.cpp` - 新規

3. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase11-timing-report.md`

## レポート記載事項
- 実装したタイミング計算の詳細
- 測定した転送速度とレイテンシ
- 実機との精度比較
- MFM/FM両モードでの検証結果
- 最適化のポイント
- Phase 12への引き継ぎ事項

## 注意事項
- 実機のFDCタイミング仕様を正確に再現
- 300RPMと360RPMの両方に対応
- オーバーヘッドを最小限に抑える
- 浮動小数点演算の精度に注意
- デバッグログでタイミング値を出力可能に