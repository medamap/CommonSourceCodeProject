# WD_FDC 構造解析レポート

## 調査日時
2025年6月10日

## 取得したファイルのバージョン情報
- **取得元**: https://github.com/mamedev/mame/master/src/devices/machine/
- **wd_fdc.h**: 613行
- **wd_fdc.cpp**: 3126行
- **合計**: 3739行
- **ライセンス**: BSD-3-Clause
- **著作権者**: Olivier Galibert

## ファイルの概要

### wd_fdc.h
- Western Digitalフロッピーコントローラファミリーの統一実装
- 基底クラス: `wd_fdc_device_base`
- アナログPLL版: `wd_fdc_analog_device_base`
- デジタルPLL版: `wd_fdc_digital_device_base`
- MB8877を含む多数のFDCチップをサポート

### wd_fdc.cpp
- ステートマシン駆動の実装
- ライブステート（live state）による精密なタイミング制御
- PLL（Phase Locked Loop）による読み書きタイミング管理

## MB8877との機能対応表

### パブリックインターフェース比較

| 機能 | MB8877.cpp | wd_fdc.cpp | 対応状況 |
|------|------------|------------|----------|
| **基本I/O** |
| write_io8/read_io8 | ○ | write/read | 名前は異なるが機能は同等 |
| write_dma_io8/read_dma_io8 | ○ | data_w/data_r | DRQ制御で実現 |
| **レジスタアクセス** |
| コマンドレジスタ | addr=0 | cmd_w | ○ |
| トラックレジスタ | addr=1 | track_w/track_r | ○ |
| セクタレジスタ | addr=2 | sector_w/sector_r | ○ |
| データレジスタ | addr=3 | data_w/data_r | ○ |
| ステータスレジスタ | addr=0(読) | status_r | ○ |
| **信号制御** |
| write_signal | ○ | 個別メソッド | 部分対応 |
| set_context_irq/drq/rdy | ○ | コールバック登録 | ○ |
| **ディスク操作** |
| open_disk/close_disk | ○ | set_floppy | 方式が異なる |
| is_disk_inserted | ○ | floppyオブジェクト経由 | ○ |
| is_disk_protected | ○ | floppy->wpt_r() | ○ |
| get_drive_type | ○ | × | 未実装 |
| set_drive_rpm | ○ | × | 未実装 |
| **デバッグ** |
| get_debug_regs_info | ○ | × | 未実装 |
| **ノイズ再生** |
| set_context_noise_* | ○ | × | 未実装 |

### コマンド実装の比較

| コマンド | MB8877 | wd_fdc | 備考 |
|----------|--------|--------|------|
| Type I (Restore/Seek/Step) | ○ | ○ | 完全互換 |
| Type II (Read/Write Sector) | ○ | ○ | 完全互換 |
| Type III (Read Address/Track) | ○ | ○ | 完全互換 |
| Type IV (Force Interrupt) | ○ | ○ | 完全互換 |
| MB89311拡張 (FCh-FFh) | ○ | × | 未実装 |

## ステートマシンとイベント駆動の対応関係

### アーキテクチャの違い

#### MB8877（イベント駆動）
```
イベント登録 → タイマー待機 → イベントコールバック → 処理実行
- EVENT_SEEK, EVENT_SEARCH, EVENT_DRQ等の7種類のイベント
- register_my_event()でイベント登録
- event_callback()で処理
```

#### wd_fdc（ステートマシン駆動）
```
状態遷移 → ライブステート実行 → 同期ポイント → 次の状態へ
- メイン状態（IDLE, RESTORE, SEEK等）
- サブ状態（SPINUP, SETTLE_WAIT等）
- ライブ状態（SEARCH_ADDRESS_MARK等）
```

### 状態遷移の対応

| MB8877イベント | wd_fdcステート | 説明 |
|----------------|----------------|------|
| EVENT_SEEK | SEEK_MOVE → SEEK_WAIT_STEP_TIME | シーク処理 |
| EVENT_SEEKEND | SEEK_DONE | シーク完了 |
| EVENT_SEARCH | SCAN_ID → live state | セクタ検索 |
| EVENT_DRQ | live state内で処理 | データ転送 |
| EVENT_LOST | status |= S_LOST | データロスト |

### タイミング管理の違い

#### MB8877
- マイクロ秒単位の絶対時間管理
- get_cur_position()で現在位置取得
- usec単位でイベント登録

#### wd_fdc
- attotime（高精度時間）使用
- PLLによる同期管理
- チェックポイント/ロールバック機能

## ディスクI/O実装の差異

### セクタ読み書きフロー

#### 読み込み処理

**MB8877:**
1. cmd_readdata() → search_sector()
2. EVENT_SEARCH待機
3. データ転送開始、DRQ設定
4. read_io8()でバイト読み込み
5. EVENT_DRQで次バイト

**wd_fdc:**
1. read_sector_start() → SCAN_ID
2. live_start(SEARCH_ADDRESS_MARK_HEADER)
3. セクタ一致確認
4. live_start(SEARCH_ADDRESS_MARK_DATA)
5. READ_SECTOR_DATAライブステート

### エラーハンドリング

両実装とも同じステータスフラグを使用:
- S_RNF (Record Not Found)
- S_CRC (CRC Error)
- S_LOST (Lost Data)
- S_WF (Write Fault)

### CRCチェックとデータ整合性

**MB8877:** セクタ単位でCRC計算・検証
**wd_fdc:** ライブステート内でビット単位のCRC計算

## 特殊機能の対応状況

### MB8877特有機能のwd_fdc対応

| 機能 | MB8877 | wd_fdc | 移植可能性 |
|------|--------|--------|------------|
| **特殊ディスク対応** |
| FM7用（RIGLAS等） | ○ | × | カスタム実装必要 |
| X1用（Batten Tanuki） | ○ | × | カスタム実装必要 |
| **MB89311拡張** |
| FCh-FFhコマンド | ○ | × | 派生クラスで実装可能 |
| Read/Write-after-Seek | ○ | × | 状態遷移追加で対応可能 |
| **ノイズ再生** |
| シーク音 | ○ | × | コールバック追加で対応可能 |
| ヘッドロード音 | ○ | × | コールバック追加で対応可能 |
| **ドライブ制御** |
| 複数ドライブ管理 | 内部配列 | 外部管理 | アーキテクチャが異なる |
| RPM設定 | ○ | × | PLLパラメータで代替可能 |

## 移植時の技術的課題

### 1. アーキテクチャの根本的違い
- **MB8877**: イベント駆動、個別タイマー管理
- **wd_fdc**: ステートマシン駆動、統一的なライブステート

### 2. ディスク管理方式
- **MB8877**: 内部でDISKオブジェクト配列管理
- **wd_fdc**: 外部からfloppy_image_deviceを設定

### 3. タイミング精度
- **MB8877**: マイクロ秒単位、相対時間
- **wd_fdc**: attotime使用、絶対時間、PLL同期

### 4. 特殊機能の欠如
- MB89311拡張コマンド未対応
- 特殊ディスク処理なし
- ノイズ再生機能なし

## 推奨される移植アプローチ

### 1. 段階的移植戦略

#### Phase 1: 基本機能移植
1. mb8877_deviceをwd_fdc_analog_device_baseから派生
2. 既存のMB8877設定をコンストラクタで適用
3. レジスタI/Oインターフェースのラッパー実装

#### Phase 2: 特殊機能の追加
1. MB89311拡張コマンドをcmd_w()オーバーライドで実装
2. 特殊ディスク処理をライブステート拡張で対応
3. ノイズ再生コールバックの追加

#### Phase 3: 互換性の完全化
1. ドライブ管理方式の調整
2. タイミング互換性の検証
3. 既存ソフトウェアでのテスト

### 2. 実装上の注意点

1. **状態管理の変換**
   - MB8877のイベントIDをwd_fdcのサブステートにマッピング
   - タイミング計算をPLLベースに変換

2. **インターフェースの互換性**
   - write_io8/read_io8をwrite/readにマッピング
   - write_signalを個別メソッドに分解

3. **特殊処理の移植**
   - 特殊ディスク判定をライブステート内に組み込み
   - MB89311コマンドを派生クラスで実装

### 3. テスト計画

1. **基本動作テスト**
   - 標準的なディスク読み書き
   - 各種エラー条件の確認

2. **互換性テスト**
   - FM7/X1実機用ソフトウェア
   - 特殊フォーマットディスク

3. **パフォーマンステスト**
   - タイミング精度の検証
   - CPU負荷の比較

## 結論

wd_fdcは高度に汎用化されたFDC実装であり、MB8877の基本機能は完全にカバーしています。ただし、MB8877固有の拡張機能（MB89311互換、特殊ディスク対応、ノイズ再生）は実装されていないため、これらの機能が必要な場合は派生クラスでの追加実装が必要です。

移植作業は技術的に可能であり、段階的アプローチにより既存の互換性を保ちながら、より保守性の高い実装への移行が実現できます。