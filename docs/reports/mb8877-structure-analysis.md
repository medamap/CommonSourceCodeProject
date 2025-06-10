# MB8877 構造解析レポート

## 調査日時
2025年6月10日

## ファイルの概要
- **mb8877.h**: 235行
- **mb8877.cpp**: 1791行
- **合計**: 2026行
- **ライセンス**: 記載なし（ヘッダーにOrigin: XM7, Author: Takeda.Toshiyaと記載）

## 1. パブリックメソッドの一覧

### コンストラクタ・デストラクタ
- `MB8877(VM_TEMPLATE* parent_vm, EMU* parent_emu)` - コンストラクタ
- `~MB8877()` - デストラクタ

### 共通関数（DEVICEクラスからのオーバーライド）
- `void initialize()` - 初期化処理
- `void release()` - リソース解放
- `void reset()` - リセット処理
- `void write_io8(uint32_t addr, uint32_t data)` - I/O書き込み（8ビット）
- `uint32_t read_io8(uint32_t addr)` - I/O読み込み（8ビット）
- `void write_dma_io8(uint32_t addr, uint32_t data)` - DMA I/O書き込み
- `uint32_t read_dma_io8(uint32_t addr)` - DMA I/O読み込み
- `void write_signal(int id, uint32_t data, uint32_t mask)` - 信号書き込み
- `uint32_t read_signal(int ch)` - 信号読み込み
- `void event_callback(int event_id, int err)` - イベントコールバック
- `void update_config()` - 設定更新
- `bool process_state(FILEIO* state_fio, bool loading)` - ステート保存/読み込み

### デバッグ関連
- `bool is_debugger_available()` - デバッガ利用可能チェック
- `bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)` - デバッグレジスタ情報取得

### コンテキスト設定
- `void set_context_irq(DEVICE* device, int id, uint32_t mask)` - IRQ出力先設定
- `void set_context_drq(DEVICE* device, int id, uint32_t mask)` - DRQ出力先設定  
- `void set_context_rdy(DEVICE* device, int id, uint32_t mask)` - RDY出力先設定
- `void set_context_noise_seek(NOISE* device)` - シーク音設定
- `NOISE* get_context_noise_seek()` - シーク音取得
- `void set_context_noise_head_down(NOISE* device)` - ヘッドダウン音設定
- `NOISE* get_context_noise_head_down()` - ヘッドダウン音取得
- `void set_context_noise_head_up(NOISE* device)` - ヘッドアップ音設定
- `NOISE* get_context_noise_head_up()` - ヘッドアップ音取得

### ディスク操作
- `DISK* get_disk_handler(int drv)` - ディスクハンドラ取得
- `void open_disk(int drv, const _TCHAR* file_path, int bank)` - ディスク挿入
- `void close_disk(int drv)` - ディスク取り出し
- `bool is_disk_inserted(int drv)` - ディスク挿入状態確認
- `bool is_disk_changed(int drv)` - ディスク変更状態確認
- `void is_disk_protected(int drv, bool value)` - ライトプロテクト設定
- `bool is_disk_protected(int drv)` - ライトプロテクト状態取得
- `bool is_drive_ready()` - 現在ドライブのレディ状態
- `bool is_drive_ready(int drv)` - 指定ドライブのレディ状態
- `uint8_t get_media_type(int drv)` - メディアタイプ取得
- `void set_drive_type(int drv, uint8_t type)` - ドライブタイプ設定
- `uint8_t get_drive_type(int drv)` - ドライブタイプ取得
- `void set_drive_rpm(int drv, int rpm)` - ドライブ回転数設定
- `void set_drive_mfm(int drv, bool mfm)` - MFMモード設定
- `void set_track_size(int drv, int size)` - トラックサイズ設定
- `uint8_t fdc_status()` - FDCステータス取得

## 2. パブリックフィールドの一覧

パブリックフィールドは存在しません。すべてのフィールドはprivateセクションに定義されています。

### 主要なプライベートフィールド

#### 出力信号
- `outputs_t outputs_irq` - IRQ出力信号
- `outputs_t outputs_drq` - DRQ出力信号  
- `outputs_t outputs_rdy` - RDY出力信号

#### ノイズ関連
- `NOISE* d_noise_seek` - シーク音
- `NOISE* d_noise_head_down` - ヘッドダウン音
- `NOISE* d_noise_head_up` - ヘッドアップ音

#### ドライブ情報構造体配列
- `struct fdc[MAX_DRIVE]` - 各ドライブのFDC情報
  - `int track` - 現在のトラック位置
  - `int index` - データインデックス
  - `bool access` - アクセス状態
  - `bool head_load` - ヘッドロード状態
  - その他タイミング・位置情報

#### ディスクハンドラ
- `DISK* disk[MAX_DRIVE]` - ディスクオブジェクト配列

#### レジスタ
- `uint8_t status` - ステータスレジスタ
- `uint8_t cmdreg` - コマンドレジスタ
- `uint8_t trkreg` - トラックレジスタ
- `uint8_t secreg` - セクタレジスタ
- `uint8_t datareg` - データレジスタ
- `uint8_t drvreg` - ドライブレジスタ
- `uint8_t sidereg` - サイドレジスタ

## 3. イベント駆動の仕組み

### イベント定義
```cpp
#define EVENT_SEEK        0  // シークイベント
#define EVENT_SEEKEND     1  // シーク終了イベント
#define EVENT_SEARCH      2  // セクタ検索イベント
#define EVENT_DRQ         3  // DRQイベント
#define EVENT_MULTI1      4  // マルチセクタ処理1
#define EVENT_MULTI2      5  // マルチセクタ処理2
#define EVENT_LOST        6  // データロストイベント
```

### イベント管理メソッド
- `cancel_my_event(int event)` - イベントキャンセル
- `register_my_event(int event, double usec)` - イベント登録
- `register_seek_event(bool first)` - シークイベント登録
- `register_drq_event(int bytes)` - DRQイベント登録
- `register_lost_event(int bytes)` - ロストイベント登録

### タイミング計算
- `get_cur_position()` - 現在位置取得
- `get_usec_to_start_trans(bool first_sector)` - 転送開始までの時間
- `get_usec_to_next_trans_pos(bool delay)` - 次の転送位置までの時間
- `get_usec_to_detect_index_hole(int count, bool delay)` - インデックスホール検出時間

### イベントコールバック処理
`event_callback()`メソッドで各イベントを処理：
- EVENT_SEEK: シーク処理の継続/完了
- EVENT_SEARCH: セクタ検索完了処理
- EVENT_DRQ: DRQ発生処理
- EVENT_LOST: データロスト処理

## 4. 外部依存関係

### インクルードファイル
- `mb8877.h`
- `disk.h` - ディスク操作クラス
- `noise.h` - ノイズ再生クラス
- `vm.h` - 仮想マシン基本定義
- `../emu.h` - エミュレータコア
- `device.h` - デバイス基底クラス

### 使用クラス
- `DISK` - ディスク管理クラス
- `NOISE` - ノイズ再生クラス
- `DEVICE` - 基底クラス（継承）

### 信号定義
```cpp
#define SIG_MB8877_ACCESS    0  // アクセス信号
#define SIG_MB8877_DRIVEREG  1  // ドライブレジスタ
#define SIG_MB8877_SIDEREG   2  // サイドレジスタ
#define SIG_MB8877_MOTOR     3  // モーター信号
```

## 5. ディスクI/O処理

### 読み込み処理フロー
1. `cmd_readdata()` - Read Dataコマンド開始
2. `search_sector()` - セクタ検索
3. EVENT_SEARCH - 検索完了イベント
4. DRQ設定・データ転送開始
5. `read_io8(3)` - データレジスタ読み込み
6. EVENT_DRQ - 次のバイト転送

### 書き込み処理フロー
1. `cmd_writedata()` - Write Dataコマンド開始
2. `search_sector()` - セクタ検索
3. EVENT_SEARCH - 検索完了イベント
4. DRQ設定・データ転送開始
5. `write_io8(3, data)` - データレジスタ書き込み
6. EVENT_DRQ - 次のバイト転送

### エラーハンドリング
- `FDC_ST_RECNFND` - セクタ未検出
- `FDC_ST_CRCERR` - CRCエラー
- `FDC_ST_LOSTDATA` - データロスト
- `FDC_ST_WRITEFAULT` - 書き込みエラー
- `FDC_ST_SEEKERR` - シークエラー

### トラック書き込み処理
- F5h: A1h（ミッシングクロック）書き込み
- F6h: C2h（ミッシングクロック）書き込み
- F7h: CRC書き込み
- データマーク（F8h/FBh）の処理

## 移植時の注意点と課題

### 1. イベント駆動の複雑性
- 7種類のイベントが相互に関連
- タイミング計算が精密（マイクロ秒単位）
- イベントIDにコマンドタイプを含める実装

### 2. ディスク位置管理
- `cur_position`と`next_trans_position`の二重管理
- バイト単位での精密な位置追跡
- インデックスホール検出タイミング

### 3. 特殊ディスク対応
- FM7用特殊ディスク（RIGLAS、XANADU2等）
- X1用特殊ディスク（Batten Tanuki）
- ディスクごとの特殊処理

### 4. MB89311拡張モード
- 拡張コマンド（FCh-FFh）
- Read/Write-after-Seekコマンド
- フォーマットコマンド（未実装）

### 5. ノイズ再生機能
- シーク音、ヘッドロード音の再生
- WAVファイルの動的ロード

## 推奨される次のステップ

1. **wd_fdc.cppとの詳細比較**
   - 共通インターフェースの抽出
   - 実装差異の明確化

2. **イベント機構の移植方針決定**
   - wd_fdcのイベント機構との統合方法
   - タイミング計算の共通化

3. **特殊ディスク処理の整理**
   - 機種依存部分の分離
   - 共通処理への統合方法

4. **テストケースの準備**
   - 基本的な読み書きテスト
   - 特殊ディスクのテスト
   - タイミング依存処理のテスト

5. **段階的移植計画の策定**
   - 基本機能の移植
   - 拡張機能の移植
   - 特殊処理の移植