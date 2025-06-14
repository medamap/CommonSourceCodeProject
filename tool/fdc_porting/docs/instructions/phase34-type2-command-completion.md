# Phase 34: Type IIコマンド完全実装・80%成功率達成指示書

## エージェント名
Type2CommandCompletionAgent-Phase34

## 作業目的
Phase 33で修正したディスクインフラストラクチャ上で、Phase 31のREAD操作修正を活かし、Type IIコマンド（READ/WRITE SECTOR）の完全実装を行い、80%以上の成功率を達成する。

## 前提情報
- Phase 33でSafeDISK実装完了（クラッシュ0%）
- MB8877統合でREAD SECTOR 100%動作確認
- Phase 31のREAD操作修正が正しいことが証明済み
- Type IIコマンドテストは再ビルドが必要

## 現状分析
```
成功している部分：
1. SafeDISKによる安全なディスクアクセス
2. 基本的なREAD SECTOR動作
3. DRQシグナリング
4. ステータス更新

未実装/修正が必要な部分：
1. WRITE SECTORコマンド
2. マルチセクタ読み書き
3. エラー条件の処理
4. 削除データマーク対応
```

## 実装戦略

### Phase 34.1: 既存コードの統合（1時間）

#### 1.1 Phase 31修正の再適用
```cpp
// mb8877_compat.cpp - EVENT_SEARCHハンドラー
case EVENT_SEARCH:
    if(disk[drvreg] == nullptr) {
        // SafeDISKチェック
        status |= FDC_ST_RECNFND;
        set_irq(true);
        break;
    }
    
    if(status_tmp & FDC_ST_RECTYPE) {
        // 削除データマーク処理
        status |= FDC_ST_RECTYPE;
    }
    
    if((status & FDC_ST_RECNFND) != 0) {
        // セクタ未検出
        if(disk[drvreg]->sector_num.sd > 0) {
            // インデックスホールまで回転
            int usec = disk[drvreg]->get_usec_per_track();
            usec -= disk[drvreg]->get_passed_usec(prev_drq_clock);
            if(usec < 50) usec = 50;
            register_my_event(EVENT_LOST, usec);
            break;
        }
    }
    
    // セクタ検出成功
    switch(cmdtype & 0xf0) {
        case 0x80:  // READ SECTOR
        case 0x90:  // READ MULTIPLE
            // Phase 31の修正を適用
            DISK* disk_read = disk[drvreg];
            if(disk_read && disk_read->sector_size.sd > 0) {
                // セクタデータをバッファにコピー
                fdc[drvreg].count = disk_read->sector_size.sd;
                memcpy(fdc[drvreg].buffer, disk_read->sector, fdc[drvreg].count);
                fdc[drvreg].index = 0;
                
                // DRQ設定
                set_drq(true);
                
                // 次のイベント登録
                register_my_event(EVENT_DRQ, 30);
            }
            break;
            
        case 0xa0:  // WRITE SECTOR
        case 0xb0:  // WRITE MULTIPLE
            // WRITE実装（新規）
            if(!disk[drvreg]->write_protected) {
                fdc[drvreg].count = disk[drvreg]->sector_size.sd;
                fdc[drvreg].index = 0;
                set_drq(true);
                register_my_event(EVENT_DRQ, 30);
            } else {
                status |= FDC_ST_WRITEP;
                set_irq(true);
            }
            break;
    }
    break;
```

#### 1.2 read_io8修正の再適用
```cpp
// mb8877_compat.cpp - read_io8
case 3:  // データレジスタ
    if(status & FDC_ST_DRQ) {
        // Phase 31の修正適用
        if((cmdtype & 0xf0) == 0x80 || (cmdtype & 0xf0) == 0x90) {
            // READ操作
            if(fdc[drvreg].index < fdc[drvreg].count) {
                val = fdc[drvreg].buffer[fdc[drvreg].index++];
                
                if(fdc[drvreg].index >= fdc[drvreg].count) {
                    // セクタ読み取り完了
                    status &= ~FDC_ST_DRQ;
                    
                    if((cmdtype & 0xf0) == 0x90) {
                        // マルチセクタ読み取り
                        sector++;
                        if(sector > disk[drvreg]->sector_num.sd) {
                            // 次のトラックへ
                            cmd_next_track();
                        } else {
                            // 次のセクタ検索
                            register_my_event(EVENT_SEARCH, 100);
                        }
                    } else {
                        // シングルセクタ完了
                        status &= ~FDC_ST_BUSY;
                        set_irq(true);
                    }
                }
            }
        }
    } else {
        // DRQがない状態での読み取り
        status |= FDC_ST_LOST;
    }
    break;
```

### Phase 34.2: WRITE SECTOR実装（2時間）

#### 2.1 write_io8データレジスタ処理
```cpp
// mb8877_compat.cpp - write_io8
case 3:  // データレジスタ
    if(status & FDC_ST_DRQ) {
        if((cmdtype & 0xf0) == 0xa0 || (cmdtype & 0xf0) == 0xb0) {
            // WRITE操作
            if(fdc[drvreg].index < fdc[drvreg].count) {
                fdc[drvreg].buffer[fdc[drvreg].index++] = data;
                
                if(fdc[drvreg].index >= fdc[drvreg].count) {
                    // バッファフル - セクタ書き込み
                    status &= ~FDC_ST_DRQ;
                    register_my_event(EVENT_WRITE_SECTOR, 100);
                }
            }
        } else {
            // Type III/IV用の処理
            datareg = data;
        }
    } else {
        // DRQがない状態での書き込み
        status |= FDC_ST_LOST;
    }
    break;
```

#### 2.2 EVENT_WRITE_SECTOR実装
```cpp
// mb8877_compat.cpp - event_callback
case EVENT_WRITE_SECTOR:
    if(disk[drvreg] && !disk[drvreg]->write_protected) {
        // セクタ書き込み実行
        if(disk[drvreg]->get_track(track, sidereg)) {
            // トラック書き込み位置計算
            int sector_offset = disk[drvreg]->calc_sector_offset(sector);
            
            if(sector_offset >= 0) {
                // セクタデータ書き込み
                memcpy(disk[drvreg]->track + sector_offset, 
                       fdc[drvreg].buffer, fdc[drvreg].count);
                
                // ディスクに反映
                disk[drvreg]->set_data_crc_error(false);
                
                if((cmdtype & 0xf0) == 0xb0) {
                    // マルチセクタ書き込み
                    sector++;
                    if(sector > disk[drvreg]->sector_num.sd) {
                        cmd_next_track();
                    } else {
                        // 次のセクタ準備
                        fdc[drvreg].index = 0;
                        set_drq(true);
                        register_my_event(EVENT_DRQ, 30);
                    }
                } else {
                    // シングルセクタ完了
                    status &= ~FDC_ST_BUSY;
                    set_irq(true);
                }
            } else {
                // セクタ位置エラー
                status |= FDC_ST_WRITEFAULT;
                set_irq(true);
            }
        }
    } else {
        // 書き込み保護
        status |= FDC_ST_WRITEP;
        set_irq(true);
    }
    break;
```

### Phase 34.3: エラー処理強化（2時間）

#### 3.1 CRCエラー処理
```cpp
// mb8877_compat.cpp - cmd_read_sector修正
void cmd_read_sector(bool first_sector) {
    // 既存の処理...
    
    if(disk[drvreg]->data_crc_error) {
        // CRCエラー検出
        status |= FDC_ST_CRCERR;
        // データは読み取るが、エラーフラグを立てる
    }
    
    if(disk[drvreg]->deleted) {
        // 削除データマーク
        status |= FDC_ST_RECTYPE;
    }
    
    // セクタ検索開始
    register_my_event(EVENT_SEARCH, get_search_time());
}

// CRCエラーシミュレーション（テスト用）
void simulate_crc_error(int track, int sector) {
    if(disk[drvreg] && disk[drvreg]->get_sector(track, sidereg, sector)) {
        disk[drvreg]->set_data_crc_error(true);
    }
}
```

#### 3.2 データロスト処理
```cpp
// mb8877_compat.cpp - EVENT_LOST
case EVENT_LOST:
    if(status & FDC_ST_BUSY) {
        if(fdc[drvreg].index < fdc[drvreg].count) {
            // データ転送未完了
            status |= FDC_ST_LOST;
        }
        
        // コマンド強制終了
        status &= ~(FDC_ST_BUSY | FDC_ST_DRQ);
        set_irq(true);
    }
    break;

// DRQタイムアウト監視
case EVENT_DRQ:
    if(status & FDC_ST_DRQ) {
        // DRQが長時間クリアされない
        if(++fdc[drvreg].drq_count > 16) {
            // データロスト
            register_my_event(EVENT_LOST, 10);
        } else {
            // 再度チェック
            register_my_event(EVENT_DRQ, 200);
        }
    }
    break;
```

### Phase 34.4: マルチセクタ対応（2時間）

#### 4.1 連続セクタ読み取り
```cpp
// mb8877_compat.cpp - cmd_next_sector
void cmd_next_sector() {
    sector++;
    
    if(sector > disk[drvreg]->sector_num.sd) {
        // トラック境界
        if(cmdtype & 0x10) {  // Mビット
            cmd_next_track();
        } else {
            // 単一トラックで終了
            status &= ~FDC_ST_BUSY;
            set_irq(true);
        }
    } else {
        // 次のセクタ検索
        register_my_event(EVENT_SEARCH, 100);
    }
}

// トラック移行処理
void cmd_next_track() {
    if(sidereg == 0 && disk[drvreg]->media_type != MEDIA_TYPE_2D) {
        // サイド0→サイド1
        sidereg = 1;
        sector = 1;
        register_my_event(EVENT_SEARCH, 200);
    } else {
        // 次のトラックへ
        sidereg = 0;
        track++;
        trkreg++;
        sector = 1;
        
        if(track < disk[drvreg]->get_max_tracks()) {
            // ヘッドシーク
            seektrk = track;
            seekvct = true;
            register_my_event(EVENT_SEEK, get_step_rate_time());
        } else {
            // ディスク終端
            status &= ~FDC_ST_BUSY;
            set_irq(true);
        }
    }
}
```

### Phase 34.5: Type IIコマンドテスト強化（3時間）

#### 5.1 包括的テストケース
```cpp
// test_mb8877_type2_complete.cpp
#include "test_framework.h"
#include "../safe_disk.h"
#include "../../../src/vm/mb8877_compat.h"

void test_read_write_sector_complete(TestFramework& test) {
    TEST_SECTION("Complete Type II Command Tests");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // SafeDISK使用
    fdc.open_disk(0, nullptr, 0);  // ダミーディスク
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // 1. シングルセクタ読み取り
    test_single_sector_read(&fdc, &event, test);
    
    // 2. シングルセクタ書き込み
    test_single_sector_write(&fdc, &event, test);
    
    // 3. マルチセクタ読み取り
    test_multi_sector_read(&fdc, &event, test);
    
    // 4. マルチセクタ書き込み
    test_multi_sector_write(&fdc, &event, test);
    
    // 5. エラー条件テスト
    test_error_conditions(&fdc, &event, test);
}

void test_single_sector_read(MB8877* fdc, MockEVENT* event, TestFramework& test) {
    // トラック0、セクタ1読み取り
    fdc->write_io8(1, 0);  // Track
    fdc->write_io8(2, 1);  // Sector
    fdc->write_io8(0, 0x80);  // READ SECTOR
    
    // データ読み取り
    std::vector<uint8_t> data;
    bool success = read_sector_data(fdc, event, data);
    
    test.assert_true(success, "Single sector read success");
    test.assert_equal(data.size(), 256, "Read 256 bytes");
    test.assert_true((fdc->read_io8(0) & 0x01) == 0, "BUSY cleared");
}

void test_single_sector_write(MB8877* fdc, MockEVENT* event, TestFramework& test) {
    // テストデータ準備
    std::vector<uint8_t> test_data(256);
    for(int i = 0; i < 256; i++) {
        test_data[i] = i & 0xFF;
    }
    
    // セクタ2に書き込み
    fdc->write_io8(1, 0);  // Track
    fdc->write_io8(2, 2);  // Sector
    fdc->write_io8(0, 0xA0);  // WRITE SECTOR
    
    // データ書き込み
    bool success = write_sector_data(fdc, event, test_data);
    
    test.assert_true(success, "Single sector write success");
    test.assert_true((fdc->read_io8(0) & 0x40) == 0, "No write protect");
    
    // 読み戻して検証
    fdc->write_io8(2, 2);  // Sector
    fdc->write_io8(0, 0x80);  // READ SECTOR
    
    std::vector<uint8_t> read_back;
    read_sector_data(fdc, event, read_back);
    
    test.assert_equal(read_back, test_data, "Written data verified");
}

void test_multi_sector_read(MB8877* fdc, MockEVENT* event, TestFramework& test) {
    // 3セクタ連続読み取り
    fdc->write_io8(1, 0);  // Track
    fdc->write_io8(2, 1);  // Start sector
    fdc->write_io8(0, 0x90);  // READ MULTIPLE
    
    std::vector<uint8_t> all_data;
    int sectors_read = 0;
    
    while(fdc->read_io8(0) & 0x01) {  // BUSY
        if(fdc->read_io8(0) & 0x02) {  // DRQ
            uint8_t byte = fdc->read_io8(3);
            all_data.push_back(byte);
            
            if(all_data.size() % 256 == 0) {
                sectors_read++;
            }
        }
        event->advance_clock(10);
        
        if(sectors_read >= 3) {
            // 3セクタ読んだら中断
            fdc->write_io8(0, 0xD0);  // FORCE INTERRUPT
            break;
        }
    }
    
    test.assert_equal(sectors_read, 3, "Read 3 sectors");
    test.assert_equal(all_data.size(), 768, "Read 768 bytes total");
}

// エラー条件テスト
void test_error_conditions(MB8877* fdc, MockEVENT* event, TestFramework& test) {
    // 1. セクタ未検出
    fdc->write_io8(1, 0);
    fdc->write_io8(2, 99);  // 無効なセクタ
    fdc->write_io8(0, 0x80);
    
    wait_command_complete(fdc, event);
    test.assert_true(fdc->read_io8(0) & 0x10, "Record not found");
    
    // 2. CRCエラー（シミュレート）
    // SafeDISKでCRCエラーを設定する機能が必要
    
    // 3. データロスト
    fdc->write_io8(2, 1);
    fdc->write_io8(0, 0x80);
    
    // DRQを無視して待つ
    for(int i = 0; i < 1000; i++) {
        event->advance_clock(100);
        if(!(fdc->read_io8(0) & 0x01)) break;
    }
    
    test.assert_true(fdc->read_io8(0) & 0x04, "Data lost detected");
}
```

#### 5.2 パフォーマンステスト
```cpp
void test_type2_performance(TestFramework& test) {
    TEST_SECTION("Type II Performance Tests");
    
    // セットアップ（省略）
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 1トラック（16セクタ）読み取り
    for(int sector = 1; sector <= 16; sector++) {
        fdc.write_io8(2, sector);
        fdc.write_io8(0, 0x80);
        
        std::vector<uint8_t> data;
        read_sector_data(&fdc, &event, data);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    test.assert_true(duration.count() < 100, "Track read < 100ms");
}
```

## テスト手順

### 1. 既存テストのリビルド
```bash
cd tool/fdc_porting/test
make clean
make all
```

### 2. Type IIコマンド専用テスト
```bash
make test_mb8877_type2_complete
./test_mb8877_type2_complete -v
```

### 3. 統合テスト
```bash
./run_all_tests.sh
```

## 成果物

### 1. 実装レポート
- `tool/fdc_porting/docs/reports/phase34-type2-command-completion-report.md`

### 2. 更新ファイル
- `src/vm/mb8877_compat.cpp` - Type IIコマンド完全実装
- `src/vm/mb8877_compat.h` - EVENT_WRITE_SECTOR追加
- `tool/fdc_porting/test/test_mb8877_type2_complete.cpp` - 包括的テスト

### 3. 標準出力（JSON形式）
```json
{
  "phase": 34,
  "task": "type2_command_completion",
  "implementation": {
    "read_sector": "complete",
    "write_sector": "complete",
    "multi_sector": "complete",
    "error_handling": "complete"
  },
  "test_results": {
    "type2_tests": {
      "total": 28,
      "passed": "X",
      "failed": "X",
      "success_rate": "X%"
    },
    "command_breakdown": {
      "read_single": "X/X",
      "read_multiple": "X/X",
      "write_single": "X/X",
      "write_multiple": "X/X",
      "error_cases": "X/X"
    },
    "regression_tests": {
      "type1_commands": "100%",
      "overall": "X%"
    }
  },
  "performance": {
    "sector_read_time": "X µs",
    "sector_write_time": "X µs",
    "track_read_time": "X ms"
  },
  "next_phase": {
    "phase": 35,
    "focus": "type3_type4_commands",
    "expected_completion": "95%+"
  }
}
```

## 成功基準

### 最低基準
- Type IIコマンド成功率 70%以上
- READ/WRITE基本動作確認
- クラッシュ0維持

### 目標基準
- Type IIコマンド成功率 80%以上
- マルチセクタ対応
- エラー処理実装

### 理想基準
- Type IIコマンド成功率 90%以上
- 全エラーケース対応
- 実用レベルの性能

## 注意事項
- SafeDISKとの整合性を保つ
- DRQタイミングの正確性
- セクタ境界処理の注意
- メモリ安全性の維持