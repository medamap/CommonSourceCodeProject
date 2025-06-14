# Phase 23: 機能テスト修正・FDC動作精度向上指示書

## エージェント名
FunctionalTestFixAgent-Phase23

## 作業目的
Phase 22でセグフォルト完全解消を達成したため、機能テスト成功率を35%から70-80%に向上させる。FDCステートマシンの適切な実装、タイミング精度向上、モックディスク動作改善に集中する。

## 前提情報
- Phase 22でセグフォルト100%解消達成（安定性100%）
- 現在の機能成功率：35%（test_mb8877_registers: 70%, type1: 25%, type2: 43%）
- 142箇所のディスクアクセス安全化完了
- 安全性インフラ（SafetyError、get_disk_safe等）導入済み

## 参照すべきファイル
- `tool/fdc_porting/test/test_mb8877_registers.cpp` - 基本機能テスト
- `tool/fdc_porting/test/test_mb8877_type1_commands.cpp` - シークコマンドテスト  
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - データ転送テスト
- `src/vm/mb8877_compat.cpp` - 機能実装対象
- `tool/fdc_porting/test/mock_environment.h` - モック環境改善
- `src/vm/mb8877.cpp` - オリジナル動作参考

## 修正戦略

### Phase 23.1: テスト失敗分析と優先度付け（2時間）

#### 1.1 個別テスト詳細分析
```bash
# 各テストの個別実行と詳細ログ収集
./test_mb8877_registers -v 2>&1 | tee registers_detailed.log
./test_mb8877_type1_commands -v 2>&1 | tee type1_detailed.log  
./test_mb8877_type2_commands -v 2>&1 | tee type2_detailed.log
```

#### 1.2 失敗パターンの分類
- **初期化関連**: レジスタ初期値、状態設定
- **ステートマシン**: 状態遷移の不正確性
- **タイミング**: イベント処理、遅延計算
- **ディスク操作**: セクタ検索、データ転送
- **割り込み処理**: IRQ/DRQ信号の適切な制御

#### 1.3 修正優先度マトリックス
```
優先度1（緊急）: 基本レジスタアクセス、初期化
優先度2（重要）: Type Iコマンド（RESTORE/SEEK）
優先度3（中程度）: Type IIコマンド（READ/WRITE SECTOR）
優先度4（低い）: 拡張機能、エラーハンドリング詳細
```

### Phase 23.2: 基本機能修正（4時間）

#### 2.1 レジスタアクセス精度向上
```cpp
// read_io8/write_io8の動作精度向上
uint32_t MB8877::read_io8(uint32_t addr) {
    // 1. 正確なステータス反映
    update_fdc_status_accurate();
    
    // 2. DRQ/IRQタイミングの正確な制御
    handle_drq_irq_timing();
    
    // 3. レジスタ値の適切な返却
    return get_register_value_accurate(addr & 3);
}
```

#### 2.2 ステートマシン実装強化
```cpp
enum FDCState {
    STATE_IDLE,
    STATE_SEEK_IN_PROGRESS,
    STATE_SECTOR_SEARCH,
    STATE_DATA_TRANSFER,
    STATE_COMMAND_COMPLETION
};

void MB8877::update_state_machine() {
    switch(main_state) {
        case STATE_SEEK_IN_PROGRESS:
            handle_seek_progress();
            break;
        case STATE_SECTOR_SEARCH:
            handle_sector_search_progress();
            break;
        case STATE_DATA_TRANSFER:
            handle_data_transfer_progress();
            break;
    }
}
```

#### 2.3 初期化プロセス強化
```cpp
void MB8877::reset() {
    // 1. 完全な状態初期化
    initialize_all_registers();
    
    // 2. ディスク状態の適切な設定
    setup_disk_initial_state();
    
    // 3. タイミング系の初期化
    reset_timing_subsystem();
}
```

### Phase 23.3: Type Iコマンド修正（3時間）

#### 3.1 RESTOREコマンド精度向上
```cpp
void MB8877::cmd_restore() {
    // 1. トラック00検出の正確な実装
    if (detect_track00_accurately()) {
        set_track00_status();
        complete_restore_immediately();
    } else {
        start_seek_to_track00();
    }
    
    // 2. ヘッドロードタイミングの正確な制御
    if (cmdreg & 0x08) {
        schedule_head_load_delay();
    }
    
    // 3. ベリファイ動作の実装
    if (cmdreg & 0x04) {
        schedule_verify_operation();
    }
}
```

#### 3.2 SEEKコマンド精度向上
```cpp
void MB8877::cmd_seek() {
    // 1. 目標トラックへの段階的移動
    int target_track = datareg;
    int current_track = fdc[drvreg].track;
    
    // 2. ステップレート制御の正確な実装
    double step_time = calculate_step_time_accurate();
    
    // 3. シーク完了検出の改善
    schedule_seek_completion_check();
}
```

#### 3.3 STEPコマンド系統の修正
```cpp
void MB8877::cmd_step_family() {
    // 1. ディレクションフラグの正確な管理
    bool direction_in = determine_step_direction();
    
    // 2. トラックレジスタ更新制御
    if (cmdreg & 0x10) {  // Update flag
        update_track_register_conditionally();
    }
    
    // 3. 物理的位置との同期
    synchronize_physical_position();
}
```

### Phase 23.4: Type IIコマンド修正（3時間）

#### 4.1 READ SECTORコマンド修正
```cpp
void MB8877::cmd_readdata(bool first_sector) {
    // 1. セクタ検索の精度向上
    SectorSearchResult result = search_sector_enhanced();
    
    // 2. データ転送タイミングの正確な制御
    if (result.found) {
        setup_data_transfer_timing(result);
        initiate_read_sequence();
    }
    
    // 3. マルチセクタ対応の改善
    handle_multi_sector_read();
}
```

#### 4.2 WRITE SECTORコマンド修正
```cpp
void MB8877::cmd_writedata(bool first_sector) {
    // 1. 書き込み保護チェックの強化
    if (enhanced_write_protect_check()) {
        abort_write_with_proper_status();
        return;
    }
    
    // 2. セクタバッファ管理の改善
    prepare_sector_buffer_for_write();
    
    // 3. DRQ信号制御の精密化
    control_drq_timing_precisely();
}
```

### Phase 23.5: タイミング・割り込み制御強化（2時間）

#### 5.1 イベント処理精度向上
```cpp
void MB8877::event_callback(int event_id, int err) {
    // 1. イベントタイプ別の精密処理
    EventType type = decode_event_type(event_id);
    
    // 2. タイミング計算の高精度化
    double precise_timing = calculate_precise_timing(type);
    
    // 3. 状態遷移の確実な実行
    execute_state_transition_safely(type);
}
```

#### 5.2 DRQ/IRQ制御の改善
```cpp
void MB8877::control_drq_irq_precisely() {
    // 1. DRQ信号の適切なタイミング制御
    if (should_set_drq()) {
        set_drq_with_timing(calculate_drq_delay());
    }
    
    // 2. IRQ信号の正確な生成
    if (command_completed()) {
        set_irq_with_proper_status();
    }
}
```

### Phase 23.6: モック環境改善（1時間）

#### 6.1 DISKクラス動作改善
```cpp
// mock_environment.h の拡張
class DISK {
public:
    // より現実的なディスク動作
    bool simulate_realistic_seek_time();
    bool simulate_sector_rotation();
    bool simulate_data_transfer_rate();
    
    // テスト用の制御可能な動作
    void set_test_mode(TestMode mode);
    void inject_controlled_errors(ErrorType type);
};
```

## テスト戦略

### Phase 23.7: 段階的検証（1時間）

#### 7.1 個別機能テスト
```bash
# レジスタテスト
./test_mb8877_registers
# 目標: 7/10 PASS → 9/10 PASS

# Type I テスト  
./test_mb8877_type1_commands
# 目標: 1/4 PASS → 3/4 PASS

# Type II テスト
./test_mb8877_type2_commands  
# 目標: 6/14 PASS → 10/14 PASS
```

#### 7.2 統合テスト
```bash
# 全テスト実行
make clean && make all && ./run_all_tests.sh
# 目標: 機能成功率 35% → 70%以上
```

#### 7.3 回帰テスト
```bash
# セグフォルト回帰チェック
valgrind --tool=memcheck ./test_mb8877_registers
valgrind --tool=memcheck ./test_mb8877_type1_commands
valgrind --tool=memcheck ./test_mb8877_type2_commands
# 目標: セグフォルト0維持
```

## 成果物

### 1. 修正レポート（Markdownファイル）
- `tool/fdc_porting/docs/reports/phase23-functional-test-fixes-report.md`

### 2. 修正ファイル
- `src/vm/mb8877_compat.cpp` - 機能精度向上
- `tool/fdc_porting/test/mock_environment.h` - モック動作改善

### 3. 標準出力（JSON形式）
```json
{
  "phase": 23,
  "task": "functional_test_fixes",
  "test_improvements": {
    "test_mb8877_registers": {
      "before": "7/10 PASS (70%)",
      "after": "X/10 PASS (X%)",
      "improvement": "+X%"
    },
    "test_mb8877_type1_commands": {
      "before": "1/4 PASS (25%)",
      "after": "X/4 PASS (X%)",
      "improvement": "+X%"
    },
    "test_mb8877_type2_commands": {
      "before": "6/14 PASS (43%)",
      "after": "X/14 PASS (X%)",
      "improvement": "+X%"
    }
  },
  "functional_improvements": {
    "register_accuracy": "enhanced",
    "state_machine": "implemented",
    "timing_precision": "improved",
    "interrupt_control": "refined"
  },
  "overall_success_rate": {
    "before": "35%",
    "after": "X%",
    "target_achieved": "70%+"
  },
  "stability_maintained": true,
  "segfaults": 0,
  "phase24_readiness": "ready/needs_more_work"
}
```

## 成功基準

### 最低基準
- 機能成功率 35% → 55%以上
- セグフォルト0維持
- 各テストカテゴリで部分的改善

### 目標基準
- 機能成功率 35% → 70%以上
- レジスタテスト 90%以上成功
- Type I/IIコマンドで顕著な改善

### 理想基準
- 機能成功率 35% → 80%以上
- 全テストカテゴリで大幅改善
- 実用レベルの動作精度達成

## 品質保証

### 1. 機能品質
- コマンド実行の正確性
- 状態遷移の適切性
- タイミングの現実性

### 2. 安定性維持
- セグフォルト発生なし
- メモリリークなし
- 予期しない動作なし

### 3. テスト品質
- テスト実行の確実性
- 結果の再現性
- エラー原因の特定可能性

## 注意事項
- Phase 22の安全性インフラを維持
- パフォーマンスより正確性重視
- オリジナル仕様との互換性確保
- 段階的修正による安全な進歩
- テスト結果の詳細な記録と分析