# Phase 35: Type III/IVコマンド実装・95%完成率達成指示書

## エージェント名
Type3Type4CommandAgent-Phase35

## 作業目的
MB8877 FDCエミュレーションの最終段階として、Type III（READ ADDRESS/TRACK、WRITE TRACK）およびType IV（FORCE INTERRUPT）コマンドを実装し、全体の完成率95%以上を達成する。

## 前提情報
- Phase 34でType IIコマンド96.4%成功率達成
- Type Iコマンド100%、Type IIコマンドほぼ完璧
- SafeDISKによる安定したディスクアクセス
- 現在の全体成功率: 約85%（Type III/IV未実装）

## 実装対象コマンド
```
Type III Commands:
- 0xC0-0xDF: READ ADDRESS（IDフィールド読み取り）
- 0xE0-0xFF: READ TRACK（トラック全体読み取り）
- 0xF0-0xFF: WRITE TRACK（フォーマット）

Type IV Commands:
- 0xD0-0xDF: FORCE INTERRUPT（強制割り込み）
```

## 実装戦略

### Phase 35.1: READ ADDRESSコマンド実装（2時間）

#### 1.1 コマンドハンドラ
```cpp
// mb8877_compat.cpp - cmd_read_address実装
void cmd_read_address() {
    // Type IIIコマンド開始
    cmdtype = FDC_CMD_TYPE3;
    status = FDC_ST_BUSY;
    
    // モーターON確認
    if(!check_drive_ready()) {
        status = FDC_ST_NOTREADY;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    // 現在のヘッド位置でIDフィールド検索
    fdc[drvreg].id_read_count = 0;
    fdc[drvreg].id_read_state = ID_READ_SEARCHING;
    
    // インデックスホール待ち
    if(disk[drvreg]) {
        int wait_time = disk[drvreg]->get_usec_per_track() / 16;
        register_my_event(EVENT_SEARCH_ID, wait_time);
    }
}

// EVENT_SEARCH_ID処理
case EVENT_SEARCH_ID:
    if(disk[drvreg] && disk[drvreg]->get_sector(trkreg, sidereg, 1)) {
        // 最初のセクタのIDフィールドを読む
        fdc[drvreg].buffer[0] = disk[drvreg]->id[0];  // Track
        fdc[drvreg].buffer[1] = disk[drvreg]->id[1];  // Head
        fdc[drvreg].buffer[2] = disk[drvreg]->id[2];  // Sector
        fdc[drvreg].buffer[3] = disk[drvreg]->id[3];  // Size
        fdc[drvreg].buffer[4] = disk[drvreg]->id[4];  // CRC1
        fdc[drvreg].buffer[5] = disk[drvreg]->id[5];  // CRC2
        
        fdc[drvreg].count = 6;
        fdc[drvreg].index = 0;
        
        // セクタレジスタ更新
        sector = disk[drvreg]->id[2];
        
        // DRQ設定
        set_drq(true);
        register_my_event(EVENT_DRQ, 30);
    } else {
        // IDフィールドが見つからない
        status = FDC_ST_RECNFND;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
    }
    break;
```

#### 1.2 read_io8でのIDデータ読み取り
```cpp
// mb8877_compat.cpp - read_io8修正
case 3:  // データレジスタ
    if(status & FDC_ST_DRQ) {
        if(cmdtype == FDC_CMD_TYPE3 && (command & 0xF0) == 0xC0) {
            // READ ADDRESS
            if(fdc[drvreg].index < fdc[drvreg].count) {
                val = fdc[drvreg].buffer[fdc[drvreg].index++];
                
                if(fdc[drvreg].index >= fdc[drvreg].count) {
                    // ID読み取り完了
                    status &= ~(FDC_ST_DRQ | FDC_ST_BUSY);
                    set_irq(true);
                }
            }
        }
        // 既存のType II処理...
    }
    break;
```

### Phase 35.2: READ TRACKコマンド実装（2時間）

#### 2.1 トラック全体読み取り
```cpp
// mb8877_compat.cpp - cmd_read_track実装
void cmd_read_track() {
    cmdtype = FDC_CMD_TYPE3;
    status = FDC_ST_BUSY;
    
    if(!check_drive_ready()) {
        status = FDC_ST_NOTREADY;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    // トラックデータ取得
    if(disk[drvreg] && disk[drvreg]->get_track(trkreg, sidereg)) {
        // トラック全体をバッファにコピー
        fdc[drvreg].count = disk[drvreg]->track_size.sd;
        if(fdc[drvreg].count > sizeof(fdc[drvreg].buffer)) {
            fdc[drvreg].count = sizeof(fdc[drvreg].buffer);
        }
        
        memcpy(fdc[drvreg].buffer, disk[drvreg]->track, fdc[drvreg].count);
        fdc[drvreg].index = 0;
        
        // インデックスホールから開始
        fdc[drvreg].track_read_state = TRACK_READ_GAP;
        
        set_drq(true);
        register_my_event(EVENT_DRQ, 30);
    } else {
        status = FDC_ST_RECNFND;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
    }
}

// トラックデータ生成（GAP、SYNC、データ含む）
void generate_track_data(uint8_t* buffer, int& size) {
    int pos = 0;
    
    // トラック開始GAP
    for(int i = 0; i < 80; i++) {
        buffer[pos++] = 0x4E;  // GAP4A
    }
    
    // 各セクタ
    for(int sect = 1; sect <= disk[drvreg]->sector_num.sd; sect++) {
        // SYNC
        for(int i = 0; i < 12; i++) {
            buffer[pos++] = 0x00;
        }
        for(int i = 0; i < 3; i++) {
            buffer[pos++] = 0xA1;  // SYNC（クロック欠落）
        }
        
        // ID AM
        buffer[pos++] = 0xFE;
        
        // IDフィールド
        if(disk[drvreg]->get_sector(trkreg, sidereg, sect)) {
            buffer[pos++] = disk[drvreg]->id[0];  // Track
            buffer[pos++] = disk[drvreg]->id[1];  // Side
            buffer[pos++] = disk[drvreg]->id[2];  // Sector
            buffer[pos++] = disk[drvreg]->id[3];  // Size
            buffer[pos++] = disk[drvreg]->id[4];  // CRC1
            buffer[pos++] = disk[drvreg]->id[5];  // CRC2
        }
        
        // GAP2
        for(int i = 0; i < 22; i++) {
            buffer[pos++] = 0x4E;
        }
        
        // Data SYNC
        for(int i = 0; i < 12; i++) {
            buffer[pos++] = 0x00;
        }
        for(int i = 0; i < 3; i++) {
            buffer[pos++] = 0xA1;
        }
        
        // Data AM
        buffer[pos++] = disk[drvreg]->deleted ? 0xF8 : 0xFB;
        
        // セクタデータ
        int data_size = disk[drvreg]->sector_size.sd;
        memcpy(buffer + pos, disk[drvreg]->sector, data_size);
        pos += data_size;
        
        // CRC
        buffer[pos++] = 0x00;  // CRC1（仮）
        buffer[pos++] = 0x00;  // CRC2（仮）
        
        // GAP3
        for(int i = 0; i < 54; i++) {
            buffer[pos++] = 0x4E;
        }
    }
    
    size = pos;
}
```

### Phase 35.3: WRITE TRACKコマンド実装（3時間）

#### 3.1 フォーマット処理
```cpp
// mb8877_compat.cpp - cmd_write_track実装
void cmd_write_track() {
    cmdtype = FDC_CMD_TYPE3;
    status = FDC_ST_BUSY;
    
    if(!check_drive_ready()) {
        status = FDC_ST_NOTREADY;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    if(disk[drvreg] && disk[drvreg]->write_protected) {
        status = FDC_ST_WRITEP;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    // フォーマットデータ受信準備
    fdc[drvreg].format_count = 0;
    fdc[drvreg].format_state = FORMAT_IDLE;
    
    // DRQ設定してデータ受信開始
    set_drq(true);
    register_my_event(EVENT_DRQ, 30);
}

// フォーマットデータ解析
void parse_format_data() {
    uint8_t* data = fdc[drvreg].buffer;
    int size = fdc[drvreg].format_count;
    int pos = 0;
    
    std::vector<SectorFormat> sectors;
    
    while(pos < size) {
        // SYNC検索
        if(pos + 4 < size && 
           data[pos] == 0x00 && data[pos+1] == 0x00 && 
           data[pos+2] == 0x00 && data[pos+3] == 0xA1) {
            pos += 4;
            
            if(pos < size && data[pos] == 0xFE) {
                // IDフィールド発見
                pos++;
                if(pos + 6 <= size) {
                    SectorFormat fmt;
                    fmt.track = data[pos++];
                    fmt.side = data[pos++];
                    fmt.sector = data[pos++];
                    fmt.size = data[pos++];
                    fmt.crc1 = data[pos++];
                    fmt.crc2 = data[pos++];
                    
                    sectors.push_back(fmt);
                }
            }
        } else {
            pos++;
        }
    }
    
    // トラックフォーマット実行
    if(!sectors.empty()) {
        format_track_with_sectors(sectors);
    }
}

// write_io8でのフォーマットデータ受信
case 3:  // データレジスタ
    if(status & FDC_ST_DRQ) {
        if(cmdtype == FDC_CMD_TYPE3 && (command & 0xF0) == 0xF0) {
            // WRITE TRACK
            if(fdc[drvreg].format_count < sizeof(fdc[drvreg].buffer)) {
                fdc[drvreg].buffer[fdc[drvreg].format_count++] = data;
                
                // インデックスホール検出でフォーマット実行
                if(check_index_hole()) {
                    status &= ~FDC_ST_DRQ;
                    register_my_event(EVENT_FORMAT, 100);
                }
            }
        }
    }
    break;

// EVENT_FORMAT処理
case EVENT_FORMAT:
    parse_format_data();
    
    // フォーマット完了
    status &= ~FDC_ST_BUSY;
    set_irq(true);
    break;
```

### Phase 35.4: FORCE INTERRUPTコマンド実装（1時間）

#### 4.1 強制割り込み処理
```cpp
// mb8877_compat.cpp - cmd_force_interrupt実装
void cmd_force_interrupt() {
    // Type IVコマンド
    cmdtype = FDC_CMD_TYPE4;
    
    // 実行中のコマンドを中断
    if(status & FDC_ST_BUSY) {
        // 全イベントキャンセル
        cancel_my_event(EVENT_SEEK);
        cancel_my_event(EVENT_SEARCH);
        cancel_my_event(EVENT_DRQ);
        cancel_my_event(EVENT_LOST);
        cancel_my_event(EVENT_RESTORE);
        
        // ステータスクリア
        status &= ~(FDC_ST_BUSY | FDC_ST_DRQ);
        
        // データ転送中断
        if(fdc[drvreg].index < fdc[drvreg].count) {
            status |= FDC_ST_LOST;
        }
    }
    
    // 条件付き割り込み
    uint8_t condition = command & 0x0F;
    
    if(condition & 0x08) {
        // 即座に割り込み（I3=1）
        set_irq(true);
    } else if(condition & 0x04) {
        // インデックスパルスで割り込み（I2=1）
        register_my_event(EVENT_INDEX_INTERRUPT, 1000);
    } else if(condition & 0x02) {
        // Ready→Not Readyで割り込み（I1=1）
        fdc[drvreg].force_interrupt_flags = condition;
    } else if(condition & 0x01) {
        // Not Ready→Readyで割り込み（I0=1）
        fdc[drvreg].force_interrupt_flags = condition;
    } else {
        // 条件なし - 実行中コマンドの終了のみ
        if(status & FDC_ST_BUSY) {
            set_irq(true);
        }
    }
    
    // Type Iステータスレジスタに更新
    update_type1_status();
}

// インデックスパルス割り込み
case EVENT_INDEX_INTERRUPT:
    if(fdc[drvreg].force_interrupt_flags & 0x04) {
        set_irq(true);
        fdc[drvreg].force_interrupt_flags = 0;
    }
    break;
```

### Phase 35.5: 統合テスト実装（2時間）

#### 5.1 Type III/IVコマンドテスト
```cpp
// test_mb8877_type3_type4.cpp
#include "test_framework.h"
#include "../../../src/vm/mb8877_compat.h"

void test_read_address(TestFramework& test) {
    TEST_SECTION("READ ADDRESS Command");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    setup_fdc(&fdc, &event);
    
    // ディスクセットアップ
    fdc.open_disk(0, nullptr, 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // トラック10、セクタ5にシーク
    fdc.write_io8(1, 10);  // Track register
    fdc.write_io8(0, 0x10);  // SEEK
    wait_command(&fdc, &event);
    
    // READ ADDRESS実行
    fdc.write_io8(0, 0xC0);  // READ ADDRESS
    
    // IDフィールド読み取り
    std::vector<uint8_t> id_data;
    while(fdc.read_io8(0) & 0x01) {  // BUSY
        if(fdc.read_io8(0) & 0x02) {  // DRQ
            id_data.push_back(fdc.read_io8(3));
        }
        event.advance_clock(10);
    }
    
    test.assert_equal(id_data.size(), 6, "Read 6 bytes ID field");
    test.assert_equal(id_data[0], 10, "Track ID = 10");
    test.assert_equal(id_data[1], 0, "Side ID = 0");
    test.assert_true(id_data[2] >= 1 && id_data[2] <= 16, "Valid sector ID");
}

void test_read_track(TestFramework& test) {
    TEST_SECTION("READ TRACK Command");
    
    // セットアップ（省略）
    
    // READ TRACK実行
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // トラックデータ読み取り
    std::vector<uint8_t> track_data;
    int gap_count = 0;
    int sync_count = 0;
    
    while(fdc.read_io8(0) & 0x01 && track_data.size() < 8192) {
        if(fdc.read_io8(0) & 0x02) {
            uint8_t byte = fdc.read_io8(3);
            track_data.push_back(byte);
            
            if(byte == 0x4E) gap_count++;
            if(byte == 0xA1) sync_count++;
        }
        event.advance_clock(10);
    }
    
    test.assert_true(track_data.size() > 3000, "Read substantial track data");
    test.assert_true(gap_count > 100, "Contains GAP bytes");
    test.assert_true(sync_count > 10, "Contains SYNC bytes");
}

void test_write_track(TestFramework& test) {
    TEST_SECTION("WRITE TRACK (Format) Command");
    
    // セットアップ（省略）
    
    // フォーマットデータ準備
    std::vector<uint8_t> format_data = create_format_data(16);  // 16セクタ
    
    // WRITE TRACK実行
    fdc.write_io8(0, 0xF0);  // WRITE TRACK
    
    // フォーマットデータ送信
    for(size_t i = 0; i < format_data.size(); i++) {
        wait_drq(&fdc, &event);
        fdc.write_io8(3, format_data[i]);
    }
    
    wait_command(&fdc, &event);
    
    // フォーマット結果確認
    uint32_t status = fdc.read_io8(0);
    test.assert_false(status & 0x04, "No write fault");
    test.assert_false(status & 0x40, "Not write protected");
    
    // フォーマット後の読み取り確認
    verify_formatted_track(&fdc, &event, test);
}

void test_force_interrupt(TestFramework& test) {
    TEST_SECTION("FORCE INTERRUPT Command");
    
    // セットアップ（省略）
    
    // 長時間コマンド開始
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // 実行中確認
    event.advance_clock(1000);
    test.assert_true(fdc.read_io8(0) & 0x01, "Command busy");
    
    // FORCE INTERRUPT
    fdc.write_io8(0, 0xD8);  // Immediate interrupt (I3=1)
    
    // 即座に中断確認
    event.advance_clock(100);
    test.assert_false(fdc.read_io8(0) & 0x01, "Command terminated");
    
    // 条件付き割り込みテスト
    test_conditional_interrupts(&fdc, &event, test);
}

// 全Type III/IVテスト実行
bool run_type3_type4_tests() {
    TestFramework test;
    
    TEST_SUITE("MB8877 Type III/IV Command Tests");
    
    test_read_address(test);
    test_read_track(test);
    test_write_track(test);
    test_force_interrupt(test);
    
    // 統合シナリオテスト
    test_format_and_verify(test);
    test_mixed_commands(test);
    
    test.print_summary();
    test.save_results("test/results/type3_type4_test_results.txt");
    
    return test.all_tests_passed();
}
```

#### 5.2 全コマンド統合テスト
```cpp
// test_mb8877_all_commands.cpp
void test_all_commands_integration(TestFramework& test) {
    TEST_SECTION("All Command Types Integration");
    
    // Type I: ヘッドポジショニング
    test_seek_operations(&fdc, &event, test);
    
    // Type II: データ転送
    test_read_write_operations(&fdc, &event, test);
    
    // Type III: 特殊操作
    test_special_operations(&fdc, &event, test);
    
    // Type IV: 制御
    test_control_operations(&fdc, &event, test);
    
    // 複合シナリオ
    test_real_world_scenarios(&fdc, &event, test);
}
```

## テスト手順

### 1. Type III/IVテストビルド
```bash
cd tool/fdc_porting/test
make test_mb8877_type3_type4
./test_mb8877_type3_type4 -v
```

### 2. 全コマンド統合テスト
```bash
make test_mb8877_all_commands
./test_mb8877_all_commands
```

### 3. 最終検証
```bash
./run_all_tests.sh
```

## 成果物

### 1. 実装レポート
- `tool/fdc_porting/docs/reports/phase35-type3-type4-commands-report.md`

### 2. 更新ファイル
- `src/vm/mb8877_compat.cpp` - Type III/IVコマンド追加
- `src/vm/mb8877_compat.h` - 新イベント定義
- `tool/fdc_porting/test/test_mb8877_type3_type4.cpp` - Type III/IVテスト
- `tool/fdc_porting/test/test_mb8877_all_commands.cpp` - 統合テスト

### 3. 標準出力（JSON形式）
```json
{
  "phase": 35,
  "task": "type3_type4_command_implementation",
  "implementation": {
    "read_address": "complete",
    "read_track": "complete",
    "write_track": "complete",
    "force_interrupt": "complete"
  },
  "test_results": {
    "type3_tests": {
      "total": "X",
      "passed": "X",
      "success_rate": "X%"
    },
    "type4_tests": {
      "total": "X",
      "passed": "X",
      "success_rate": "X%"
    },
    "all_commands": {
      "type1": "100%",
      "type2": "96.4%",
      "type3": "X%",
      "type4": "X%",
      "overall": "X%"
    }
  },
  "completion_status": {
    "functional_completeness": "X%",
    "command_coverage": "16/16",
    "test_coverage": "X%",
    "android_ready": true
  },
  "project_summary": {
    "total_phases": 35,
    "lines_of_code": "X",
    "test_cases": "X",
    "success_rate": "X%"
  }
}
```

## 成功基準

### 最低基準
- Type III/IVコマンド基本動作
- 全体完成率 90%以上
- 既存機能の維持

### 目標基準
- Type III/IVコマンド完全動作
- 全体完成率 95%以上
- 全コマンド統合テスト成功

### 理想基準
- 全コマンド100%動作
- Android環境対応確認
- 実機相当の互換性

## 注意事項
- トラックフォーマットの正確性
- CRC計算の実装
- インデックスホールタイミング
- 割り込み条件の正確な実装