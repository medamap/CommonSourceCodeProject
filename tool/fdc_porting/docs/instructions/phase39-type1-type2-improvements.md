# Phase 39: Type I/IIコマンド改善・全体成功率向上指示書

## エージェント名
Type1Type2ImprovementAgent-Phase39

## 作業目的
Phase 38でType IV 88.2%を達成した成果を活かし、残るType I/IIコマンドの問題を修正して全体成功率を12/14テスト以上（85%+）に向上させる。メモリ安全性100%とセグフォルト0を維持しながら機能完成度を高める。

## 前提情報
- Phase 37: セグフォルト100%撲滅
- Phase 38: Type IV 88.2%達成（+29.4%改善）
- 現在成功: 6/14テスト（SafeDISK、レジスタ、Type III、RPM関連）
- 失敗中: Type I/IIコマンド、エラーハンドリング、タイミング関連

## 現在の問題分析

### run_all_tests.shの失敗パターン
```
Type Iコマンド問題:
1. RESTOREコマンド
   - "BUSY cleared after restore with verify: Expected true but got false"
   - "TRACK00 set after restore with verify: Expected true but got false"
   - "Track = 0 after restore with verify: Expected 0x00, got 0x02"

2. SEEKコマンド
   - ベリファイ処理が正常に完了しない
   - トラック位置の不一致

Type IIコマンド問題:
1. READ/WRITE SECTOR
   - DRQタイミングの問題
   - データ転送完了判定の不備
   - エラーフラグ設定の問題

2. エラーハンドリング
   - RNF (Record Not Found)フラグが設定されない
   - CRCエラー検出の不備
   - ステータス永続性の問題
```

## 実装戦略

### Phase 39.1: Type Iコマンド修正（3時間）

#### 1.1 RESTORE with Verifyの修正
```cpp
// mb8877_compat_impl.cpp - cmd_restore修正
void cmd_restore() {
    cmdtype = FDC_CMD_RD;
    status = S_BUSY;
    
    // ヘッドロード処理
    if(command & 0x08) {  // hビット
        head_load = true;
        if(!motor_on) {
            motor_on = true;
            // ヘッドロード時間待ち
            register_my_event(EVENT_SEEK, get_head_load_time());
            return;
        }
    }
    
    // RESTOREの目標は常にトラック0
    seektrk = 0;
    seekvct = (trkreg > seektrk);  // 現在位置から0への方向
    
    // ステップレート計算
    int step_time = get_step_rate_time();
    
    if(trkreg == 0) {
        // 既にトラック0の場合
        if(command & 0x04) {  // vビット（ベリファイ）
            // ベリファイ実行
            register_my_event(EVENT_RESTORE_VERIFY, step_time);
        } else {
            // ベリファイなしで完了
            status &= ~S_BUSY;
            status |= S_TRACK00;  // TRACK00フラグセット
            set_irq(true);
        }
    } else {
        // シーク開始
        register_my_event(EVENT_SEEK, step_time);
    }
}

// EVENT_RESTORE_VERIFY処理
case EVENT_RESTORE_VERIFY:
    // トラック0でのベリファイ処理
    if(trkreg == 0) {
        DISK* d = get_disk_safe(drvreg);
        if(d && d->inserted) {
            // セクタ1を検索してベリファイ
            if(d->get_sector(0, 0, 1)) {
                // ベリファイ成功
                status &= ~S_BUSY;
                status |= S_TRACK00;
                set_irq(true);
            } else {
                // ベリファイ失敗（セクタ未検出）
                status &= ~S_BUSY;
                status |= S_SEEKERR | S_TRACK00;
                set_irq(true);
            }
        } else {
            // ディスクなし
            status &= ~S_BUSY;
            status |= S_NOTREADY | S_TRACK00;
            set_irq(true);
        }
    } else {
        // トラック0でない場合（エラー）
        status &= ~S_BUSY;
        status |= S_SEEKERR;
        set_irq(true);
    }
    break;
```

#### 1.2 SEEK with Verifyの修正
```cpp
// mb8877_compat_impl.cpp - cmd_seek修正
void cmd_seek() {
    cmdtype = FDC_CMD_RD;
    status = S_BUSY;
    
    // データレジスタから目標トラック取得
    seektrk = datareg;
    
    // 範囲チェック
    DISK* d = get_disk_safe(drvreg);
    int max_tracks = 80;  // デフォルト
    if(d) {
        max_tracks = d->get_max_tracks();
    }
    
    if(seektrk >= max_tracks) {
        seektrk = max_tracks - 1;
    }
    
    seekvct = (trkreg < seektrk);
    
    if(trkreg == seektrk) {
        // 既に目標位置
        if(command & 0x04) {  // vビット
            register_my_event(EVENT_SEEK_VERIFY, 100);
        } else {
            status &= ~S_BUSY;
            update_track00_flag();
            set_irq(true);
        }
    } else {
        // シーク開始
        register_my_event(EVENT_SEEK, get_step_rate_time());
    }
}

// EVENT_SEEK_VERIFY処理
case EVENT_SEEK_VERIFY:
    {
        DISK* d = get_disk_safe(drvreg);
        if(d && d->inserted) {
            // 現在位置でセクタ検索
            bool sector_found = false;
            for(int sect = 1; sect <= 16; sect++) {
                if(d->get_sector(trkreg, 0, sect)) {
                    sector_found = true;
                    break;
                }
            }
            
            if(sector_found) {
                // ベリファイ成功
                status &= ~S_BUSY;
                update_track00_flag();
                set_irq(true);
            } else {
                // ベリファイ失敗
                status &= ~S_BUSY;
                status |= S_SEEKERR;
                update_track00_flag();
                set_irq(true);
            }
        } else {
            // ディスクなし
            status &= ~S_BUSY;
            status |= S_NOTREADY;
            update_track00_flag();
            set_irq(true);
        }
    }
    break;

// TRACK00フラグ更新
void update_track00_flag() {
    if(trkreg == 0) {
        status |= S_TRACK00;
    } else {
        status &= ~S_TRACK00;
    }
}
```

### Phase 39.2: Type IIコマンド修正（3時間）

#### 2.1 READ SECTORエラーハンドリング強化
```cpp
// mb8877_compat_impl.cpp - cmd_read_sector修正
void cmd_read_sector(bool first_sector) {
    if(first_sector) {
        cmdtype = FDC_CMD_TYPE2;
        status = S_BUSY;
        
        // ドライブ準備チェック
        if(!check_drive_ready()) {
            status = S_NOTREADY;
            status &= ~S_BUSY;
            set_irq(true);
            return;
        }
    }
    
    DISK* d = get_disk_safe(drvreg);
    if(!d || !d->inserted) {
        status = S_NOTREADY | S_RECNFND;
        status &= ~S_BUSY;
        set_irq(true);
        return;
    }
    
    // セクタ検索
    bool sector_found = d->get_sector(trkreg, sidereg, sector);
    
    if(!sector_found) {
        // セクタ未検出エラー
        status &= ~S_BUSY;
        status |= S_RECNFND;  // Record Not Found
        set_irq(true);
        return;
    }
    
    // CRCエラーチェック
    if(d->data_crc_error) {
        status |= S_CRCERR;
        // CRCエラーでもデータは読む
    }
    
    // 削除データマークチェック
    if(d->deleted) {
        status |= S_RECTYPE;
    }
    
    // データ転送準備
    fdc[drvreg].count = d->sector_size.sd;
    if(fdc[drvreg].count > sizeof(fdc[drvreg].buffer)) {
        fdc[drvreg].count = sizeof(fdc[drvreg].buffer);
    }
    
    memcpy(fdc[drvreg].buffer, d->sector, fdc[drvreg].count);
    fdc[drvreg].index = 0;
    
    // DRQ設定
    set_drq(true);
    register_my_event(EVENT_DRQ, 30);
}

// DRQタイムアウト処理強化
case EVENT_DRQ:
    if(status & S_DRQ) {
        // DRQが長時間クリアされない
        if(++fdc[drvreg].drq_timeout_count > 10) {
            // データロスト
            status |= S_LOST;
            status &= ~(S_DRQ | S_BUSY);
            set_irq(true);
            fdc[drvreg].drq_timeout_count = 0;
        } else {
            // 再度チェック
            register_my_event(EVENT_DRQ, 100);
        }
    }
    break;
```

#### 2.2 WRITE SECTORデータ転送修正
```cpp
// mb8877_compat_impl.cpp - write_io8データレジスタ修正
case 3:  // データレジスタ
    if(status & S_DRQ) {
        if(cmdtype == FDC_CMD_TYPE2 && 
           ((command & 0xF0) == 0xA0 || (command & 0xF0) == 0xB0)) {
            // WRITE SECTOR/MULTIPLE
            if(fdc[drvreg].index < fdc[drvreg].count) {
                fdc[drvreg].buffer[fdc[drvreg].index++] = data;
                
                if(fdc[drvreg].index >= fdc[drvreg].count) {
                    // バッファフル - セクタ書き込み実行
                    status &= ~S_DRQ;
                    register_my_event(EVENT_WRITE_SECTOR, 50);
                }
            }
        }
    } else {
        // DRQがない状態での書き込み - データロスト
        if(status & S_BUSY) {
            status |= S_LOST;
        }
    }
    break;

// EVENT_WRITE_SECTOR処理
case EVENT_WRITE_SECTOR:
    {
        DISK* d = get_disk_safe(drvreg);
        if(!d || !d->inserted) {
            status = S_NOTREADY;
            status &= ~S_BUSY;
            set_irq(true);
            break;
        }
        
        if(d->write_protected) {
            status |= S_WRITEP;
            status &= ~S_BUSY;
            set_irq(true);
            break;
        }
        
        // セクタ書き込み
        if(d->get_sector(trkreg, sidereg, sector)) {
            memcpy(d->sector, fdc[drvreg].buffer, fdc[drvreg].count);
            d->changed = true;
            
            // マルチセクタチェック
            if((command & 0xF0) == 0xB0) {  // WRITE MULTIPLE
                sector++;
                if(sector <= d->sector_num.sd) {
                    // 次のセクタ準備
                    fdc[drvreg].index = 0;
                    set_drq(true);
                    register_my_event(EVENT_DRQ, 30);
                } else {
                    // 全セクタ完了
                    status &= ~S_BUSY;
                    set_irq(true);
                }
            } else {
                // シングルセクタ完了
                status &= ~S_BUSY;
                set_irq(true);
            }
        } else {
            // セクタアクセスエラー
            status |= S_RECNFND;
            status &= ~S_BUSY;
            set_irq(true);
        }
    }
    break;
```

### Phase 39.3: エラーハンドリング修正（2時間）

#### 3.1 ステータスフラグ永続性
```cpp
// mb8877_compat_impl.cpp - ステータス管理改善
uint32_t read_io8(uint32_t addr) override {
    uint32_t val = 0xff;
    
    switch(addr & 3) {
        case 0:  // ステータスレジスタ
            if(cmdtype == FDC_CMD_RD) {
                // Type I ステータス
                val = get_type1_status();
            } else {
                // Type II/III ステータス
                val = status;
            }
            
            // エラーフラグは次のコマンドまで保持
            // IRQは読み取りでクリア（FORCE INTERRUPTの場合のみ）
            if(cmdtype == FDC_CMD_TYPE4) {
                clear_irq_on_status_read();
            }
            break;
            
        // 他のレジスタ...
    }
    
    return val;
}

// エラーフラグ保持管理
void clear_error_flags_on_new_command() {
    // 新しいコマンド開始時のみエラーフラグクリア
    status &= ~(S_CRCERR | S_RECNFND | S_LOST | S_RECTYPE | S_WRITEP | S_SEEKERR);
}

// process_cmd修正
void process_cmd() {
    command = cmdreg;
    
    // 新しいコマンド開始時にエラーフラグクリア
    clear_error_flags_on_new_command();
    
    // 既存のコマンド判定処理...
}
```

#### 3.2 RNFエラー検出強化
```cpp
// mb8877_compat_impl.cpp - セクタ検索タイムアウト
case EVENT_SEARCH:
    {
        DISK* d = get_disk_safe(drvreg);
        bool sector_found = false;
        
        if(d && d->inserted) {
            // セクタ検索試行
            sector_found = d->get_sector(trkreg, sidereg, sector);
            
            if(!sector_found) {
                // 無効なセクタ番号チェック
                if(sector < 1 || sector > 16) {
                    sector_found = false;
                } else {
                    // 他のセクタサイズで再試行
                    for(int retry = 0; retry < 3 && !sector_found; retry++) {
                        sector_found = d->get_sector(trkreg, sidereg, sector);
                    }
                }
            }
        }
        
        if(!sector_found) {
            // Record Not Found
            status &= ~S_BUSY;
            status |= S_RECNFND;
            set_irq(true);
        } else {
            // セクタ発見 - データ転送開始
            switch(cmdtype) {
                case FDC_CMD_TYPE2:
                    if((command & 0xF0) == 0x80 || (command & 0xF0) == 0x90) {
                        // READ処理
                        cmd_read_sector_data();
                    } else if((command & 0xF0) == 0xA0 || (command & 0xF0) == 0xB0) {
                        // WRITE処理
                        cmd_write_sector_prepare();
                    }
                    break;
            }
        }
    }
    break;
```

### Phase 39.4: タイミング調整（2時間）

#### 4.1 REALタイミング実装
```cpp
// mb8877_compat_impl.cpp - タイミング定数調整
int get_step_rate_time() {
    int rate_bits = (command >> 1) & 0x03;
    switch(rate_bits) {
        case 0: return 6000;   // 6ms (最速)
        case 1: return 12000;  // 12ms
        case 2: return 20000;  // 20ms  
        case 3: return 30000;  // 30ms (最遅)
    }
    return 6000;
}

int get_head_load_time() {
    if(command & 0x08) {  // hビット
        return 15000;  // 15ms
    }
    return 0;
}

int get_search_time() {
    return 5000;  // 5ms per sector search
}

double get_rotation_time() {
    DISK* d = get_disk_safe(drvreg);
    if(d && d->inserted) {
        return d->get_usec_per_track();
    }
    return 200000.0;  // 200ms default (300 RPM)
}
```

#### 4.2 DRQタイミング最適化
```cpp
// mb8877_compat_impl.cpp - DRQ間隔調整
void set_drq(bool val) {
    if(val) {
        status |= S_DRQ;
        fdc[drvreg].drq_timeout_count = 0;
        
        // DRQタイムアウト監視開始
        register_my_event(EVENT_DRQ, 200);  // 200μs間隔
    } else {
        status &= ~S_DRQ;
        cancel_my_event(EVENT_DRQ);
    }
    
    // DRQ信号出力
    if(d_drq) {
        write_signals(&outputs_drq, val ? 0xffffffff : 0);
    }
}
```

## テスト手順

### 1. Type I修正確認
```bash
cd tool/fdc_porting/test
make test_mb8877_type1_commands
./test_mb8877_type1_commands -v
```

### 2. Type II修正確認  
```bash
make test_mb8877_type2_commands
./test_mb8877_type2_commands -v
```

### 3. 全体テスト
```bash
make all
./run_all_tests.sh
```

### 4. エラーハンドリング確認
```bash
make test_mb8877_error_handling
./test_mb8877_error_handling -v
```

## 成果物

### 1. 修正レポート
- `tool/fdc_porting/docs/reports/phase39-type1-type2-improvements-report.md`

### 2. 更新ファイル
- `tool/fdc_porting/test/mb8877_compat_impl.cpp` - Type I/II修正
- `tool/fdc_porting/test/test_mb8877_type1_commands.cpp` - テスト改良
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - テスト改良
- `tool/fdc_porting/test/test_mb8877_error_handling.cpp` - エラーテスト強化

### 3. 標準出力（JSON形式）
```json
{
  "phase": 39,
  "task": "type1_type2_improvements",
  "test_improvements": {
    "type1_commands": {
      "before": "失敗（機能問題）",
      "after": "X% success",
      "fixes": ["restore_verify", "seek_verify", "track00_flag"]
    },
    "type2_commands": {
      "before": "失敗（機能問題）", 
      "after": "X% success",
      "fixes": ["rnf_detection", "drq_timing", "error_persistence"]
    },
    "error_handling": {
      "before": "62.5% (15/24)",
      "after": "X% (X/24)",
      "improvements": ["rnf_flag", "status_persistence", "crc_detection"]
    }
  },
  "overall_success": {
    "before": "6/14 tests (43%)",
    "after": "X/14 tests (X%)",
    "target": "12/14 tests (85%+)"
  },
  "stability_maintained": {
    "segfaults": 0,
    "memory_safety": "100%",
    "type4_success": "88.2% maintained"
  },
  "next_phase": {
    "phase": 40,
    "focus": "final_optimization",
    "target": "95%+ overall completion"
  }
}
```

## 成功基準

### 必須基準
- Type Iコマンド基本動作
- Type IIコマンド基本動作  
- 全体テスト10/14以上成功
- メモリ安全性100%維持

### 目標基準
- Type I/IIコマンド80%以上成功
- 全体テスト12/14以上成功
- エラーハンドリング80%以上

### 理想基準
- 全体テスト13/14以上成功
- Type IV 88.2%を維持
- 実用レベルの完成度

## 注意事項
- Phase 37/38の成果を破壊しない
- セグフォルト0を維持
- Type IV 88.2%成功率を維持
- 段階的修正でリスク最小化