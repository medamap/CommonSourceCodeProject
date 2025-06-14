# Phase 38: Type IV FORCE INTERRUPT完全実装指示書

## エージェント名
ForceInterruptFixAgent-Phase38

## 作業目的
Phase 37で達成したメモリ安全性を維持しながら、Type IV FORCE INTERRUPTコマンド（現在58.8%成功率）を完全実装し、90%以上の成功率を達成する。条件付き割り込み、IRQタイミング、ステータス管理を修正する。

## 前提情報
- Phase 37でセグフォルト完全撲滅（0%）
- 現在のType IV成功率: 58.8%（10/17テスト）
- メモリ安全性: 100%維持済み
- 主要問題: IRQ条件制御、ステータス更新、タイミング

## 現在の問題分析

### Phase 37テスト結果から判明した課題
```
失敗パターン:
1. IRQ条件制御の不備
   - 条件なし（0x00）でもIRQが発生
   - I3ビット以外の条件が正常動作しない

2. IRQクリア処理の問題  
   - ステータス読み取り後もIRQが残る
   - 複数回のFORCE INTERRUPTでIRQ重複

3. コマンド中断処理の不完全
   - BUSYフラグが適切にクリアされない
   - 長時間コマンドの中断が不完全

4. ステート管理の問題
   - Type I/Type IIから適切にType IVに遷移しない
   - リセット後の状態が不正
```

## 実装戦略

### Phase 38.1: IRQ条件制御修正（2時間）

#### 1.1 FORCE INTERRUPT条件ビット実装
```cpp
// mb8877_compat_impl.cpp - cmd_forceint修正
void cmd_forceint() {
    uint8_t condition = command & 0x0F;
    
    // 実行中のコマンドを即座に中断
    if(status & S_BUSY) {
        // 全イベントキャンセル
        cancel_my_event(EVENT_SEEK);
        cancel_my_event(EVENT_SEARCH);
        cancel_my_event(EVENT_DRQ);
        cancel_my_event(EVENT_LOST);
        cancel_my_event(EVENT_RESTORE);
        cancel_my_event(EVENT_INDEX_HOLE);
        
        // データ転送中の場合はDATA LOSTを設定
        if((status & S_DRQ) && fdc[drvreg].index < fdc[drvreg].count) {
            status |= S_LOST;
        }
        
        // BUSYとDRQをクリア
        status &= ~(S_BUSY | S_DRQ);
    }
    
    // Type Iステータスに切り替え
    cmdtype = FDC_CMD_RD;
    
    // 条件ビット処理
    switch(condition) {
        case 0x00:  // 条件なし - コマンド中断のみ、IRQなし
            // IRQ生成しない
            break;
            
        case 0x08:  // I3=1: 即座にIRQ
            register_my_event(EVENT_IRQ, 10);  // 10μs後にIRQ
            break;
            
        case 0x04:  // I2=1: インデックスパルス毎にIRQ
            start_index_pulse_monitor();
            break;
            
        case 0x02:  // I1=1: Ready→Not Ready遷移でIRQ
            start_ready_transition_monitor(READY_TO_NOT_READY);
            break;
            
        case 0x01:  // I0=1: Not Ready→Ready遷移でIRQ
            start_ready_transition_monitor(NOT_READY_TO_READY);
            break;
            
        default:
            // 複数ビット設定の場合は優先順位で処理
            if(condition & 0x08) {
                register_my_event(EVENT_IRQ, 10);
            } else if(condition & 0x04) {
                start_index_pulse_monitor();
            } else if(condition & 0x02) {
                start_ready_transition_monitor(READY_TO_NOT_READY);
            } else if(condition & 0x01) {
                start_ready_transition_monitor(NOT_READY_TO_READY);
            }
            break;
    }
    
    // Type Iステータスレジスタに更新
    update_type1_status();
}

// インデックスパルス監視開始
void start_index_pulse_monitor() {
    if(disk[drvreg] && disk[drvreg]->inserted) {
        // 1回転時間を計算
        double usec_per_track = disk[drvreg]->get_usec_per_track();
        register_my_event(EVENT_INDEX_HOLE, (int)usec_per_track);
        fdc[drvreg].monitor_index = true;
    }
}

// Ready状態遷移監視開始
void start_ready_transition_monitor(int transition_type) {
    fdc[drvreg].monitor_ready_transition = transition_type;
    fdc[drvreg].last_ready_state = check_drive_ready();
    fdc[drvreg].monitor_ready = true;
}
```

#### 1.2 イベントハンドラ実装
```cpp
// mb8877_compat_impl.cpp - event_callback修正
void event_callback(int event_id, int err) {
    switch(event_id) {
        case EVENT_IRQ:
            // 遅延IRQ実行
            if(d_pic) write_signals(&outputs_irq, 0xffffffff);
            break;
            
        case EVENT_INDEX_HOLE:
            if(fdc[drvreg].monitor_index) {
                // インデックスパルスIRQ
                if(d_pic) write_signals(&outputs_irq, 0xffffffff);
                
                // 継続監視（次の回転も監視）
                if(disk[drvreg] && disk[drvreg]->inserted) {
                    double usec_per_track = disk[drvreg]->get_usec_per_track();
                    register_my_event(EVENT_INDEX_HOLE, (int)usec_per_track);
                } else {
                    fdc[drvreg].monitor_index = false;
                }
            }
            break;
            
        // 既存のイベント処理...
    }
}

// Ready状態変化チェック（write_signal内で呼び出し）
void check_ready_state_change() {
    if(!fdc[drvreg].monitor_ready) return;
    
    bool current_ready = check_drive_ready();
    bool prev_ready = fdc[drvreg].last_ready_state;
    
    if(current_ready != prev_ready) {
        bool should_irq = false;
        
        if(fdc[drvreg].monitor_ready_transition == READY_TO_NOT_READY && 
           prev_ready && !current_ready) {
            should_irq = true;
        } else if(fdc[drvreg].monitor_ready_transition == NOT_READY_TO_READY && 
                  !prev_ready && current_ready) {
            should_irq = true;
        }
        
        if(should_irq) {
            if(d_pic) write_signals(&outputs_irq, 0xffffffff);
            fdc[drvreg].monitor_ready = false;  // 一度だけ
        }
        
        fdc[drvreg].last_ready_state = current_ready;
    }
}
```

### Phase 38.2: IRQクリア処理修正（2時間）

#### 2.1 IRQ管理構造改善
```cpp
// mb8877_compat.h - IRQ状態管理追加
struct {
    // 既存のメンバー...
    
    // IRQ関連
    bool irq_pending;           // IRQ待機状態
    bool irq_source_active;     // IRQ源がアクティブ
    uint8_t irq_condition;      // FORCE INTERRUPT条件
    
    // 監視状態
    bool monitor_index;         // インデックスパルス監視
    bool monitor_ready;         // Ready状態監視
    int monitor_ready_transition; // 監視する遷移タイプ
    bool last_ready_state;      // 前回のReady状態
} fdc[MAX_DRIVE];

// IRQ制御改善
void set_irq(bool val) {
    if(val) {
        fdc[drvreg].irq_pending = true;
        fdc[drvreg].irq_source_active = true;
        // 実際のIRQ信号は遅延して送信
        register_my_event(EVENT_IRQ, 10);
    } else {
        fdc[drvreg].irq_pending = false;
        fdc[drvreg].irq_source_active = false;
        if(d_pic) write_signals(&outputs_irq, 0);
    }
}

// IRQクリア処理（ステータス読み取り時）
void clear_irq_on_status_read() {
    if(fdc[drvreg].irq_pending) {
        fdc[drvreg].irq_pending = false;
        
        // 継続的なIRQ源がない場合のみクリア
        if(!fdc[drvreg].monitor_index && !fdc[drvreg].monitor_ready) {
            fdc[drvreg].irq_source_active = false;
            if(d_pic) write_signals(&outputs_irq, 0);
        }
    }
}
```

#### 2.2 ステータス読み取り修正
```cpp
// mb8877_compat_impl.cpp - read_io8修正
uint32_t read_io8(uint32_t addr) override {
    uint32_t val = 0xff;
    
    switch(addr & 3) {
        case 0:  // ステータスレジスタ
            // ステータス読み取り時にIRQクリア
            clear_irq_on_status_read();
            
            if(cmdtype == FDC_CMD_RD) {
                // Type I ステータス
                val = get_type1_status();
            } else {
                // Type II/III ステータス
                val = status;
            }
            break;
            
        // 他のケース...
    }
    
    return val;
}

uint8_t get_type1_status() {
    uint8_t type1_status = 0;
    
    // BUSY (コマンド実行中)
    if(status & S_BUSY) {
        type1_status |= 0x01;
    }
    
    // INDEX (インデックスホール)
    if(now_search) {
        type1_status |= 0x02;
    }
    
    // TRACK 00 (トラック0)
    if(trkreg == 0) {
        type1_status |= 0x04;
    }
    
    // CRC ERROR (前回のType IIコマンドのCRCエラー)
    if(status & S_CRCERR) {
        type1_status |= 0x08;
    }
    
    // SEEK ERROR (シークエラー)
    if(seektrk != trkreg) {
        type1_status |= 0x10;
    }
    
    // HEAD LOADED (ヘッドロード)
    if(head_load) {
        type1_status |= 0x20;
    }
    
    // WRITE PROTECT (書き込み保護)
    DISK* d = get_disk_safe(drvreg);
    if(d && d->write_protected) {
        type1_status |= 0x40;
    }
    
    // NOT READY (ドライブ未準備)
    if(!check_drive_ready()) {
        type1_status |= 0x80;
    }
    
    return type1_status;
}
```

### Phase 38.3: コマンド中断処理強化（2時間）

#### 3.1 長時間コマンド中断テスト
```cpp
// test_mb8877_type4_commands.cpp - 中断テスト強化
void test_force_interrupt_abort(TestFramework& test) {
    TEST_SECTION("Force Interrupt - Command Abort");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // 長時間コマンド開始（READ TRACK）
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // コマンド実行中確認
    event.advance_clock(1000);
    uint32_t status_before = fdc.read_io8(0);
    test.assert_true(status_before & 0x01, "Command BUSY before interrupt");
    
    // FORCE INTERRUPT（即座に中断）
    fdc.write_io8(0, 0xD8);  // I3=1
    
    // 即座に中断されることを確認
    event.advance_clock(100);
    uint32_t status_after = fdc.read_io8(0);
    test.assert_false(status_after & 0x01, "BUSY cleared after force interrupt");
    
    // IRQ確認
    bool irq_generated = check_irq_signal();
    test.assert_true(irq_generated, "IRQ generated for immediate interrupt");
    
    // ステータス読み取り後のIRQクリア確認
    fdc.read_io8(0);  // ステータス読み取り
    event.advance_clock(50);
    bool irq_cleared = !check_irq_signal();
    test.assert_true(irq_cleared, "IRQ cleared after status read");
}
```

#### 3.2 条件付き割り込みテスト
```cpp
void test_conditional_interrupts(TestFramework& test) {
    TEST_SECTION("Force Interrupt - Conditional IRQ");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Test 1: 条件なし（0x00）- IRQなし
    fdc.write_io8(0, 0xD0);  // FORCE INTERRUPT, no conditions
    event.advance_clock(100);
    bool no_irq = !check_irq_signal();
    test.assert_true(no_irq, "No IRQ when no conditions set");
    
    // Test 2: 即座にIRQ（I3=1）
    clear_irq_signal();
    fdc.write_io8(0, 0xD8);  // I3=1
    event.advance_clock(50);
    bool immediate_irq = check_irq_signal();
    test.assert_true(immediate_irq, "Immediate IRQ generated");
    
    // IRQクリア
    fdc.read_io8(0);
    event.advance_clock(50);
    
    // Test 3: インデックスパルス（I2=1）
    clear_irq_signal();
    fdc.write_io8(0, 0xD4);  // I2=1
    
    // インデックスホール待ち（模擬）
    event.advance_clock(200000);  // 1回転分
    bool index_irq = check_irq_signal();
    test.assert_true(index_irq, "Index pulse IRQ generated");
}

void test_ready_transition_interrupts(TestFramework& test) {
    TEST_SECTION("Force Interrupt - Ready Transitions");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // 初期状態: モーターOFF（Not Ready）
    fdc.write_signal(SIG_MB8877_MOTOR, 0, 1);
    event.advance_clock(1000);
    
    // I0=1: Not Ready→Ready遷移監視
    clear_irq_signal();
    fdc.write_io8(0, 0xD1);  // I0=1
    
    // モーターON（Ready状態へ）
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    event.advance_clock(1000);
    
    bool ready_irq = check_irq_signal();
    test.assert_true(ready_irq, "Not Ready to Ready IRQ generated");
    
    // IRQクリア
    fdc.read_io8(0);
    event.advance_clock(50);
    
    // I1=1: Ready→Not Ready遷移監視
    clear_irq_signal();
    fdc.write_io8(0, 0xD2);  // I1=1
    
    // モーターOFF（Not Ready状態へ）
    fdc.write_signal(SIG_MB8877_MOTOR, 0, 1);
    event.advance_clock(1000);
    
    bool not_ready_irq = check_irq_signal();
    test.assert_true(not_ready_irq, "Ready to Not Ready IRQ generated");
}
```

### Phase 38.4: Type IV統合強化（2時間）

#### 4.1 マルチ条件テスト
```cpp
void test_multiple_conditions(TestFramework& test) {
    TEST_SECTION("Force Interrupt - Multiple Conditions");
    
    // 複数ビット同時設定のテスト
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // I3とI2を同時設定（優先順位: I3 > I2）
    clear_irq_signal();
    fdc.write_io8(0, 0xDC);  // I3=1, I2=1
    
    event.advance_clock(50);
    bool immediate_priority = check_irq_signal();
    test.assert_true(immediate_priority, "I3 takes priority over I2");
    
    // 全ビット設定
    fdc.read_io8(0);  // IRQクリア
    event.advance_clock(50);
    clear_irq_signal();
    
    fdc.write_io8(0, 0xDF);  // All conditions
    event.advance_clock(50);
    bool all_conditions = check_irq_signal();
    test.assert_true(all_conditions, "All conditions set generates IRQ");
}

void test_interrupt_persistence(TestFramework& test) {
    TEST_SECTION("Force Interrupt - IRQ Persistence");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // 継続的な監視（インデックスパルス）
    clear_irq_signal();
    fdc.write_io8(0, 0xD4);  // I2=1
    
    // 最初のインデックスパルス
    event.advance_clock(200000);
    bool first_irq = check_irq_signal();
    test.assert_true(first_irq, "First index pulse IRQ");
    
    // IRQクリア
    fdc.read_io8(0);
    event.advance_clock(50);
    clear_irq_signal();
    
    // 次のインデックスパルス
    event.advance_clock(200000);
    bool second_irq = check_irq_signal();
    test.assert_true(second_irq, "Second index pulse IRQ");
    
    // 監視停止
    fdc.write_io8(0, 0xD0);  // 条件なし
    fdc.read_io8(0);
    event.advance_clock(50);
    clear_irq_signal();
    
    // 次のインデックスパルスではIRQなし
    event.advance_clock(200000);
    bool no_third_irq = !check_irq_signal();
    test.assert_true(no_third_irq, "No IRQ after monitoring stopped");
}
```

## テスト手順

### 1. Type IV単体テスト
```bash
cd tool/fdc_porting/test
make test_mb8877_type4_commands
./test_mb8877_type4_commands -v
```

### 2. 統合テスト
```bash
make all
./run_all_tests.sh
```

### 3. Type IV特化検証
```bash
# FORCE INTERRUPT専用テスト
make test_mb8877_force_interrupt_complete
./test_mb8877_force_interrupt_complete
```

## 成果物

### 1. 実装レポート
- `tool/fdc_porting/docs/reports/phase38-type4-force-interrupt-fix-report.md`

### 2. 更新ファイル
- `tool/fdc_porting/test/mb8877_compat_impl.cpp` - FORCE INTERRUPT完全実装
- `tool/fdc_porting/test/mb8877_compat.h` - IRQ管理構造追加
- `tool/fdc_porting/test/test_mb8877_type4_commands.cpp` - テスト強化

### 3. 標準出力（JSON形式）
```json
{
  "phase": 38,
  "task": "type4_force_interrupt_complete",
  "improvements": {
    "irq_conditions": {
      "before": "部分実装",
      "after": "完全実装",
      "conditions_supported": ["I0", "I1", "I2", "I3", "複数同時"]
    },
    "irq_management": {
      "before": "不安定",
      "after": "完全制御",
      "features": ["遅延IRQ", "条件クリア", "継続監視"]
    },
    "command_abort": {
      "before": "不完全",
      "after": "完全実装",
      "support": "全コマンドタイプ対応"
    }
  },
  "test_results": {
    "type4_tests": {
      "before": "10/17 (58.8%)",
      "after": "X/17 (X%)",
      "target": "15/17 (90%+)"
    },
    "new_tests": [
      "multiple_conditions",
      "interrupt_persistence", 
      "ready_transitions",
      "command_abort_timing"
    ]
  },
  "next_phase": {
    "phase": 39,
    "focus": "type1_type2_command_fixes",
    "target": "12/14テスト成功"
  }
}
```

## 成功基準

### 必須基準
- Type IV成功率 58.8% → 80%以上
- IRQ条件制御の完全実装
- メモリ安全性100%維持

### 目標基準
- Type IV成功率 90%以上
- 全IRQ条件の正確な動作
- コマンド中断の完全実装

### 理想基準
- Type IV成功率 95%以上
- 実機相当のIRQタイミング
- 全テスト7/14以上成功

## 注意事項
- Phase 37の安全性を維持
- 既存の成功テストを破壊しない
- IRQタイミングの正確性重視
- 条件ビットの優先順位遵守