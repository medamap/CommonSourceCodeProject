# Phase 37: セグフォルト修正・メモリ安全性確保指示書

## エージェント名
SegfaultFixAgent-Phase37

## 作業目的
run_all_tests.shで発生している複数のセグフォルトを修正し、全テストの安定動作を実現する。Type I/II/III/IVコマンドテストのメモリ安全性を確保し、95%完成率の維持を目指す。

## 前提情報
- 現在6/14テスト成功（SafeDISK、レジスタ、RPM関連は完璧）
- 8/14テストでセグフォルトまたは部分失敗
- 主要なセグフォルト発生箇所：
  - test_mb8877_type1_commands
  - test_mb8877_type2_commands  
  - test_mb8877_type2_complete
  - test_mb8877_type3_type4_commands

## セグフォルト原因仮説
```
1. NULLポインタアクセス
   - disk[drvreg]がNULLの場合のアクセス
   - event managerの未初期化
   
2. 配列境界越え
   - drvreg値の範囲チェック不備
   - バッファオーバーフロー

3. 削除済みオブジェクトアクセス
   - MockEnvironmentの早期解放
   - イベントコールバック時のオブジェクト消失

4. 初期化順序問題
   - MB8877初期化前のアクセス
   - SafeDISK作成前のdisk配列アクセス
```

## 修正戦略

### Phase 37.1: セグフォルト特定（1時間）

#### 1.1 GDBによるスタックトレース取得
```bash
# セグフォルト原因特定
cd tool/fdc_porting/test

# Type I テスト
gdb --batch --ex run --ex bt --args ./test_mb8877_type1_commands > type1_crash.log 2>&1

# Type II テスト  
gdb --batch --ex run --ex bt --args ./test_mb8877_type2_commands > type2_crash.log 2>&1

# Type II Complete テスト
gdb --batch --ex run --ex bt --args ./test_mb8877_type2_complete > type2_complete_crash.log 2>&1

# Type III/IV テスト
gdb --batch --ex run --ex bt --args ./test_mb8877_type3_type4_commands > type3_4_crash.log 2>&1
```

#### 1.2 Address Sanitizerビルド
```bash
# AddressSanitizerでリビルド
make clean
export CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
make test_mb8877_type1_commands

# ASanでテスト実行
./test_mb8877_type1_commands 2>&1 | head -100
```

### Phase 37.2: NULLポインタ防御強化（2時間）

#### 2.1 get_disk_safe関数の強化
```cpp
// mb8877_compat_impl.cpp - get_disk_safe修正
DISK* get_disk_safe(int drv) {
    // 範囲チェック
    if(drv < 0 || drv >= MAX_DRIVE) {
        fprintf(stderr, "ERROR: Invalid drive number %d\n", drv);
        return nullptr;
    }
    
    // NULLチェック
    DISK* d = disk[drv];
    if(!d) {
        fprintf(stderr, "ERROR: Disk %d not initialized\n", drv);
        return nullptr;
    }
    
    // 仮想関数テーブルチェック（軽量版）
    try {
        // 安全なメンバーアクセステスト
        volatile bool test = d->inserted;
        (void)test;  // 未使用警告回避
        return d;
    } catch(...) {
        fprintf(stderr, "ERROR: Disk %d corrupted or deleted\n", drv);
        return nullptr;
    }
}

// すべてのdisk[drvreg]アクセスをget_disk_safe()経由に変更
void cmd_read_sector(bool first_sector) {
    DISK* d = get_disk_safe(drvreg);
    if(!d) {
        status = FDC_ST_NOTREADY | FDC_ST_RECNFND;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    // 以降の処理でdiskの代わりにdを使用
    if(!d->inserted) {
        status = FDC_ST_NOTREADY;
        status &= ~FDC_ST_BUSY;
        set_irq(true);
        return;
    }
    
    // 既存の処理...
}
```

#### 2.2 イベントマネージャー安全性強化
```cpp
// mb8877_compat_impl.cpp - event_manager安全チェック
void register_my_event(int event_id, int usec) {
    if(!event_manager) {
        fprintf(stderr, "ERROR: Event manager not initialized\n");
        return;
    }
    
    if(event_id < 0 || event_id >= EVENT_MAX) {
        fprintf(stderr, "ERROR: Invalid event ID %d\n", event_id);
        return;
    }
    
    try {
        event_manager->register_event(this, event_id, usec, false, nullptr);
    } catch(...) {
        fprintf(stderr, "ERROR: Event registration failed\n");
    }
}

void cancel_my_event(int event_id) {
    if(!event_manager) {
        return;  // 既に解放済み
    }
    
    try {
        event_manager->cancel_event(this, event_id);
    } catch(...) {
        // イベントキャンセル失敗は無視
    }
}
```

### Phase 37.3: 初期化順序修正（2時間）

#### 3.1 MB8877初期化プロセス強化
```cpp
// mb8877_compat_impl.cpp - initialize修正
void initialize() override {
    // Step 1: 基本メンバー初期化
    for(int i = 0; i < MAX_DRIVE; i++) {
        disk[i] = nullptr;
        fdc[i] = {};  // 構造体ゼロ初期化
    }
    
    status = 0;
    cmdtype = 0;
    drvreg = 0;
    
    // Step 2: SafeDISK作成
    for(int i = 0; i < MAX_DRIVE; i++) {
        try {
            #ifdef STANDALONE_TEST
            disk[i] = new SafeDISK();
            #else
            // 本来のDISK実装を使用
            #endif
        } catch(...) {
            fprintf(stderr, "ERROR: Failed to create disk %d\n", i);
            disk[i] = nullptr;
        }
    }
    
    // Step 3: 初期化完了フラグ
    initialized = true;
    
    printf("MB8877: Initialized with %d drives\n", MAX_DRIVE);
}

void release() override {
    initialized = false;
    
    // イベントキャンセル
    if(event_manager) {
        cancel_my_event(EVENT_SEEK);
        cancel_my_event(EVENT_SEARCH);
        cancel_my_event(EVENT_DRQ);
        cancel_my_event(EVENT_LOST);
        cancel_my_event(EVENT_RESTORE);
    }
    
    // ディスク解放
    for(int i = 0; i < MAX_DRIVE; i++) {
        if(disk[i]) {
            #ifdef STANDALONE_TEST
            delete disk[i];
            #endif
            disk[i] = nullptr;
        }
    }
}

// すべての処理で初期化チェック
bool check_initialized() {
    if(!initialized) {
        fprintf(stderr, "ERROR: MB8877 not initialized\n");
        return false;
    }
    return true;
}
```

#### 3.2 テスト環境セットアップ修正
```cpp
// test_mb8877_type1_commands.cpp - セットアップ修正
void setup_fdc_safe(MB8877* fdc, MockEVENT* event) {
    // Step 1: イベントマネージャー設定
    fdc->set_context_event_manager(event, 0, 0, 0);
    
    // Step 2: 初期化
    fdc->initialize();
    
    // Step 3: リセット
    fdc->reset();
    
    // Step 4: モーターON（必要に応じて）
    fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Step 5: 初期化確認
    if(!fdc->check_initialized()) {
        throw std::runtime_error("FDC initialization failed");
    }
    
    printf("FDC setup completed safely\n");
}

void test_restore_command(TestFramework& test) {
    TEST_SECTION("Restore Command Tests");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    
    try {
        setup_fdc_safe(&fdc, &event);
    } catch(const std::exception& e) {
        test.assert_true(false, std::string("Setup failed: ") + e.what());
        return;
    }
    
    // テスト実行...
}
```

### Phase 37.4: バッファオーバーフロー防止（2時間）

#### 4.1 配列境界チェック強化
```cpp
// mb8877_compat_impl.cpp - 境界チェック追加
void write_io8(uint32_t addr, uint32_t data) override {
    if(addr > 3) {
        fprintf(stderr, "ERROR: Invalid I/O address %u\n", addr);
        return;
    }
    
    // drvreg境界チェック
    if(addr == 3) {  // ドライブ選択
        int new_drvreg = data & 3;
        if(new_drvreg >= MAX_DRIVE) {
            fprintf(stderr, "ERROR: Invalid drive selection %d\n", new_drvreg);
            return;
        }
        drvreg = new_drvreg;
    }
    
    // 初期化チェック
    if(!check_initialized()) {
        return;
    }
    
    // 既存の処理...
}

uint32_t read_io8(uint32_t addr) override {
    if(addr > 3) {
        fprintf(stderr, "ERROR: Invalid I/O address %u\n", addr);
        return 0xFF;
    }
    
    if(!check_initialized()) {
        return 0xFF;
    }
    
    // drvreg境界チェック
    if(drvreg >= MAX_DRIVE) {
        fprintf(stderr, "ERROR: Current drive %d out of range\n", drvreg);
        drvreg = 0;  // 安全な値にリセット
    }
    
    // 既存の処理...
}
```

#### 4.2 セクタバッファ保護
```cpp
// mb8877_compat.h - バッファ境界チェック
struct {
    uint8_t buffer[8192];
    int count;
    int index;
    
    // 安全な書き込み
    bool write_byte(uint8_t data) {
        if(index >= count || index >= sizeof(buffer)) {
            fprintf(stderr, "ERROR: Buffer overflow prevented\n");
            return false;
        }
        buffer[index++] = data;
        return true;
    }
    
    // 安全な読み込み
    bool read_byte(uint8_t& data) {
        if(index >= count || index >= sizeof(buffer)) {
            fprintf(stderr, "ERROR: Buffer underflow prevented\n");
            return false;
        }
        data = buffer[index++];
        return true;
    }
} fdc[MAX_DRIVE];
```

### Phase 37.5: テスト修正（2時間）

#### 5.1 Type Iテスト安全化
```cpp
// test_mb8877_type1_commands.cpp - 安全なテスト実装
void test_restore_command_safe(TestFramework& test) {
    TEST_SECTION("Restore Command Tests - Safe Version");
    
    try {
        MockEMU emu;
        MockVM vm(&emu);
        MockEVENT event(&vm, &emu);
        
        MB8877 fdc(&vm, &emu);
        setup_fdc_safe(&fdc, &event);
        
        // 安全なトラック設定
        fdc.write_io8(1, 10);  // Track register = 10
        
        // RESTORE実行
        fdc.write_io8(0, 0x00);  // RESTORE command
        
        // BUSY確認（タイムアウト付き）
        int timeout = 1000;
        while((fdc.read_io8(0) & 0x01) && timeout-- > 0) {
            event.advance_clock(100);
        }
        
        test.assert_true(timeout > 0, "Command completed within timeout");
        
        uint32_t final_status = fdc.read_io8(0);
        test.assert_true((final_status & 0x01) == 0, "BUSY cleared after restore");
        
    } catch(const std::exception& e) {
        test.assert_true(false, std::string("Exception: ") + e.what());
    }
}
```

#### 5.2 Type IIテスト安全化
```cpp
// test_mb8877_type2_commands.cpp - セグフォルト防止
void test_read_sector_safe(TestFramework& test) {
    TEST_SECTION("Read Sector Tests - Safe Version");
    
    try {
        MockEMU emu;
        MockVM vm(&emu);
        MockEVENT event(&vm, &emu);
        
        MB8877 fdc(&vm, &emu);
        setup_fdc_safe(&fdc, &event);
        
        // ディスク確認
        if(!fdc.get_disk_safe(0)) {
            test.assert_true(false, "Disk 0 not available");
            return;
        }
        
        // READ SECTOR実行
        fdc.write_io8(1, 0);   // Track 0
        fdc.write_io8(2, 1);   // Sector 1
        fdc.write_io8(0, 0x80); // READ SECTOR
        
        // 安全な完了待ち
        int timeout = 2000;
        while((fdc.read_io8(0) & 0x01) && timeout-- > 0) {
            event.advance_clock(50);
            
            // DRQチェック
            if(fdc.read_io8(0) & 0x02) {
                uint8_t data = fdc.read_io8(3);  // データ読み取り
                (void)data;  // 未使用警告回避
            }
        }
        
        test.assert_true(timeout > 0, "Read sector completed");
        
    } catch(const std::exception& e) {
        test.assert_true(false, std::string("Exception: ") + e.what());
    }
}
```

## テスト手順

### 1. AddressSanitizer確認
```bash
cd tool/fdc_porting/test
export CXXFLAGS="-fsanitize=address -g"
make clean && make test_mb8877_type1_commands
./test_mb8877_type1_commands
```

### 2. 修正テスト
```bash
make clean && make all
./run_all_tests.sh
```

### 3. 個別確認
```bash
./test_mb8877_type1_commands -v
./test_mb8877_type2_commands -v
./test_mb8877_type2_complete -v
./test_mb8877_type3_type4_commands -v
```

## 成果物

### 1. 修正レポート
- `tool/fdc_porting/docs/reports/phase37-segfault-fix-report.md`

### 2. 更新ファイル
- `tool/fdc_porting/test/mb8877_compat_impl.cpp` - 安全性強化
- `tool/fdc_porting/test/test_mb8877_type1_commands.cpp` - セグフォルト修正
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - セグフォルト修正
- `tool/fdc_porting/test/test_mb8877_type2_complete.cpp` - セグフォルト修正
- `tool/fdc_porting/test/test_mb8877_type3_type4_commands.cpp` - セグフォルト修正

### 3. 標準出力（JSON形式）
```json
{
  "phase": 37,
  "task": "segfault_fix",
  "fixes_applied": {
    "null_pointer_prevention": "complete",
    "boundary_checks": "complete", 
    "initialization_order": "complete",
    "memory_safety": "complete"
  },
  "test_results": {
    "before": {
      "passing": 6,
      "failing": 8,
      "segfaults": 4
    },
    "after": {
      "passing": "X",
      "failing": "X", 
      "segfaults": 0
    }
  },
  "stability": {
    "memory_leaks": 0,
    "crashes": 0,
    "safety_score": "100%"
  },
  "next_phase": {
    "phase": 38,
    "focus": "type4_force_interrupt_completion",
    "expected_success_rate": "90%+"
  }
}
```

## 成功基準

### 必須基準
- 全テストでセグフォルト0
- AddressSanitizer クリーン
- 基本機能動作維持

### 目標基準
- 10/14テスト以上成功
- Type I/IIコマンド安定動作
- メモリ安全性100%

### 理想基準
- 12/14テスト以上成功
- 全コマンドタイプ基本動作
- 実用レベルの安定性

## 注意事項
- 修正は段階的に実施
- 各修正後にテスト実行
- 既存の成功テストを破壊しない
- デバッグ情報は適切にログ出力