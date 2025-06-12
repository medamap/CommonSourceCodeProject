# Phase 7 精密タイミング調整エージェント起動のためのターミナル作業指示

## 🎯 目的
Phase 6で72.7%達成した基盤を95%+に向上させる精密タイミング調整を実行

## 📊 Phase 6査定結果

### 主要成果 ✅
- Type III Commands重複定義解決
- テスト通過率: 72.7% (8/11)
- GPL互換性問題完全解決
- 基本FDC実装完了

### 残存課題 🔧
- **22.3%のギャップ**: タイミング精度とエッジケース処理
- **実用レベル到達**: 95%目標まで残り3テスト
- **安定性向上**: 長時間動作での信頼性

## 📋 事前準備（あなたが実行）

### 1. Phase 6完了状況確認
```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject

# 修正された実装の確認
echo "=== Phase 6修正確認 ==="
grep -n "Phase 6 cleanup" src/vm/mb8877_compat.cpp
grep -c "void MB8877::cmd_readaddr" src/vm/mb8877_compat.cpp  # 1個のみであることを確認
grep -c "void MB8877::cmd_readtrack" src/vm/mb8877_compat.cpp  # 1個のみであることを確認

# タイミング関連のデバッグ出力確認
grep -n "printf.*timing" src/vm/mb8877_compat.cpp | head -5
```

### 2. 現在のテスト失敗状況詳細確認
```bash
# 詳細テスト実行
cd test
echo "=== 詳細テスト結果分析 ==="

# 個別テスト実行で失敗原因特定
make clean && make

# 各テストカテゴリ別実行
echo "Type I Commands:"
./test_mb8877_type1_commands 2>&1 | tail -10

echo "Type II Commands:"
./test_mb8877_type2_commands 2>&1 | tail -10

echo "Type III Commands:"
./test_mb8877_type3_commands 2>&1 | tail -10

echo "Type IV Commands:"
./test_mb8877_type4_commands 2>&1 | tail -10
```

### 3. タイミング精度測定基準値取得
```bash
# タイミング測定スクリプト作成
cat > measure_current_timing.sh << 'EOF'
#!/bin/bash

echo "=== Current Timing Analysis ==="
cd test

# 複数回実行でのタイミング一貫性確認
echo "Running timing consistency test (5 iterations)..."
for i in {1..5}; do
    echo "--- Iteration $i ---"
    time ./test_mb8877_type1_commands > timing_$i.log 2>&1
    grep -E "(PASS|FAIL)" timing_$i.log | wc -l
done

echo ""
echo "Timing variation analysis:"
ls -la timing_*.log

# 失敗テストの詳細分析
echo ""
echo "=== Failed Test Details ==="
./test_mb8877_type1_commands 2>&1 | grep -A5 -B5 "FAIL"
EOF

chmod +x measure_current_timing.sh
./measure_current_timing.sh
```

### 4. 実装の現在のタイミング値確認
```bash
# タイミング定数の現在値確認
echo "=== Current Timing Constants ==="
grep -n "usec.*=" src/vm/mb8877_compat.cpp | grep -E "(6000|12000|32\.0)"
grep -n "step_rate" src/vm/mb8877_compat.cpp
grep -n "register_seek_event" src/vm/mb8877_compat.cpp
```

## 🚀 Phase 7エージェント起動手順

### Step 1: タイミング改善計画確認
```bash
# 現在の問題点整理
cat > timing_issues_summary.txt << 'EOF'
=== Phase 7 Timing Improvement Plan ===

Current Status:
- Test pass rate: 72.7% (8/11)
- Main issues: Timing precision
- Type III Commands: Fixed (duplicates removed)

Target Areas:
1. Seek operation timing (6ms/12ms/20ms/30ms rates)
2. DRQ event timing (32.0 * bytes calculation)
3. Index hole detection timing
4. Status bit update timing

Expected Outcome:
- Target: 95%+ test pass rate
- Focus: Real-hardware timing accuracy
EOF

cat timing_issues_summary.txt
```

### Step 2: 新規エージェントへの指示（コピー&ペーストで使用）

```
あなたはTimingTuner-Phase7-Precisionです。MB8877 FDC移植プロジェクトの精密タイミング調整を担当します。

【重要な使命】
Phase 6で72.7%達成した基盤から95%+テスト通過率を達成するタイミング精度向上

【作業指示書】
docs/instructions/phase7-precision-tuning.md

【現在の状況】
- テスト通過率: 72.7% (8/11テスト通過)
- 基本FDC実装: 完了
- Type III重複問題: 解決済み
- 主要課題: タイミング精度とエッジケース処理

【最優先作業】
1. 失敗している3テストの根本原因分析
2. seek/DRQ/indexタイミング計算式の精密化
3. Status bit更新タイミングの実機準拠化
4. Edge case処理の改善

【技術的焦点】
- Seek timing: 現在の固定値 → 動的計算
- DRQ timing: 32.0*bytes → RPM/data rate based
- Index pulse: Track回転時間考慮
- Event競合: 同時発生処理

【目標達成基準】
- 95%+テスト通過率 (105/110以上)
- タイミング精度 ±5%以内
- 長時間安定動作確認

まず、現在失敗している3テストの詳細分析から開始してください。
```

### Step 3: 精密測定ツール準備

#### 3.1 タイミング測定ツール
```bash
# 高精度タイミング測定スクリプト
cat > precision_timing_tool.sh << 'EOF'
#!/bin/bash

echo "=== Precision Timing Measurement Tool ==="

cd test

# 高解像度タイミング測定
measure_test_timing() {
    local test_name=$1
    local iterations=${2:-10}
    
    echo "Measuring $test_name ($iterations iterations)..."
    
    for i in $(seq 1 $iterations); do
        /usr/bin/time -p ./$test_name > test_output_$i.log 2> timing_$i.log
        
        # 実行時間抽出
        real_time=$(grep "real" timing_$i.log | awk '{print $2}')
        echo "Iteration $i: ${real_time}s"
    done
    
    # 統計計算
    echo "Timing statistics for $test_name:"
    cat timing_*.log | grep "real" | awk '{sum+=$2; sumsq+=$2*$2} END {
        mean=sum/NR; 
        stddev=sqrt(sumsq/NR - mean*mean); 
        printf "Mean: %.3fs, StdDev: %.3fs, CV: %.1f%%\n", mean, stddev, (stddev/mean)*100
    }'
    
    # クリーンアップ
    rm -f test_output_*.log timing_*.log
    echo ""
}

# 各テストカテゴリの測定
measure_test_timing "test_mb8877_type1_commands" 5
measure_test_timing "test_mb8877_type2_commands" 5
measure_test_timing "test_mb8877_type3_commands" 5

EOF

chmod +x precision_timing_tool.sh
```

#### 3.2 失敗テスト詳細分析ツール
```bash
# 失敗テスト分析ツール
cat > analyze_failed_tests.sh << 'EOF'
#!/bin/bash

echo "=== Failed Test Analysis Tool ==="

cd test

# 詳細ログ付きテスト実行
run_detailed_test() {
    local test_name=$1
    echo "Analyzing $test_name..."
    
    # デバッグ出力付きで実行
    ./$test_name 2>&1 | tee ${test_name}_detailed.log
    
    # 失敗テストの抽出
    echo "Failed tests in $test_name:"
    grep -B3 -A3 "FAIL" ${test_name}_detailed.log
    
    echo "Pass/Fail summary:"
    grep -c "PASS" ${test_name}_detailed.log
    grep -c "FAIL" ${test_name}_detailed.log
    echo ""
}

# 全テストカテゴリの詳細分析
run_detailed_test "test_mb8877_type1_commands"
run_detailed_test "test_mb8877_type2_commands"
run_detailed_test "test_mb8877_type3_commands"
run_detailed_test "test_mb8877_type4_commands"

# 共通失敗パターンの分析
echo "=== Common Failure Patterns ==="
cat *_detailed.log | grep "FAIL" | sort | uniq -c | sort -nr

EOF

chmod +x analyze_failed_tests.sh
```

### Step 4: 進捗監視システム

#### 4.1 継続テスト監視
```bash
# 継続テスト監視スクリプト
cat > continuous_monitoring.sh << 'EOF'
#!/bin/bash

MONITOR_DIR="phase7_monitoring"
mkdir -p $MONITOR_DIR

echo "=== Phase 7 Continuous Monitoring ==="
echo "Started at: $(date)"

monitor_iteration=1

while true; do
    echo ""
    echo "=== Monitoring Iteration $monitor_iteration at $(date) ==="
    
    cd test
    
    # 全テスト実行
    total_tests=0
    passed_tests=0
    
    for test in test_mb8877_*_commands; do
        if [ -x "$test" ]; then
            echo "Running $test..."
            ./$test > "../$MONITOR_DIR/${test}_iter${monitor_iteration}.log" 2>&1
            
            # 結果集計
            test_passed=$(grep -c "PASS" "../$MONITOR_DIR/${test}_iter${monitor_iteration}.log")
            test_failed=$(grep -c "FAIL" "../$MONITOR_DIR/${test}_iter${monitor_iteration}.log")
            
            echo "  $test: $test_passed passed, $test_failed failed"
            
            total_tests=$((total_tests + test_passed + test_failed))
            passed_tests=$((passed_tests + test_passed))
        fi
    done
    
    # 成功率計算
    if [ $total_tests -gt 0 ]; then
        success_rate=$(echo "scale=1; $passed_tests * 100 / $total_tests" | bc)
        echo "Overall success rate: $success_rate% ($passed_tests/$total_tests)"
        
        # 95%達成チェック
        if (( $(echo "$success_rate >= 95.0" | bc -l) )); then
            echo "🎉 95% SUCCESS RATE ACHIEVED! 🎉"
            echo "Final result: $success_rate% at $(date)"
            exit 0
        fi
    fi
    
    cd ..
    
    # 次の監視まで待機
    echo "Waiting 5 minutes before next check..."
    sleep 300
    
    monitor_iteration=$((monitor_iteration + 1))
    
    # 最大50回で停止
    if [ $monitor_iteration -gt 50 ]; then
        echo "Maximum monitoring iterations reached."
        break
    fi
done

EOF

chmod +x continuous_monitoring.sh
```

#### 4.2 パフォーマンス追跡
```bash
# パフォーマンス追跡ツール
cat > performance_tracker.sh << 'EOF'
#!/bin/bash

echo "=== Performance Tracking for Phase 7 ==="

# システムリソース監視開始
while true; do
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    # CPU使用率
    cpu_usage=$(top -l 1 | grep "CPU usage" | head -1)
    
    # メモリ使用量
    memory_usage=$(vm_stat | head -10)
    
    # プロセス情報（テスト実行中の場合）
    test_processes=$(ps aux | grep test_mb8877 | grep -v grep)
    
    echo "[$timestamp] CPU: $cpu_usage"
    if [ ! -z "$test_processes" ]; then
        echo "[$timestamp] Active test processes:"
        echo "$test_processes"
    fi
    
    sleep 30
done > performance_log.txt &

PERF_PID=$!
echo "Performance monitoring started (PID: $PERF_PID)"
echo "To stop: kill $PERF_PID"

EOF

chmod +x performance_tracker.sh
```

## ⚠️ 重要な注意事項

### 1. 段階的アプローチの重要性
```bash
# Phase 7は3段階で進行
echo "Phase 7 Milestone Checkpoints:"
echo "Week 1: 80%+ success rate target"
echo "Week 2: 90%+ success rate target"  
echo "Week 3: 95%+ success rate achievement"
```

### 2. バックアップとロールバック準備
```bash
# Phase 6完了時点のバックアップ
cp -r src/vm/mb8877_compat.* backup_phase6/
git tag phase6-complete
git commit -m "Phase 6 completion - 72.7% success rate"
```

### 3. リアルタイム問題対応
```bash
# 問題発生時の即座対応
cat > emergency_response.sh << 'EOF'
#!/bin/bash

echo "=== Emergency Response Tool ==="
echo "If success rate drops below 70%:"
echo "1. Revert to Phase 6 backup"
echo "2. Analyze regression cause"
echo "3. Apply targeted fix only"

cp backup_phase6/mb8877_compat.* src/vm/
echo "Reverted to Phase 6 stable version"
EOF

chmod +x emergency_response.sh
```

## 🎯 Phase 7完了確認

### 最終チェックリスト
```bash
cat > phase7_completion_check.sh << 'EOF'
#!/bin/bash

echo "=== Phase 7 Completion Verification ==="

cd test

# 最終テスト実行
final_passed=0
final_total=0

for test in test_mb8877_*_commands; do
    if [ -x "$test" ]; then
        ./$test > final_${test}.log 2>&1
        passed=$(grep -c "PASS" final_${test}.log)
        failed=$(grep -c "FAIL" final_${test}.log)
        
        echo "$test: $passed passed, $failed failed"
        final_passed=$((final_passed + passed))
        final_total=$((final_total + passed + failed))
    fi
done

final_rate=$(echo "scale=2; $final_passed * 100 / $final_total" | bc)

echo ""
echo "=== FINAL PHASE 7 RESULT ==="
echo "Success Rate: $final_rate% ($final_passed/$final_total)"

if (( $(echo "$final_rate >= 95.0" | bc -l) )); then
    echo "✅ Phase 7 SUCCESSFULLY COMPLETED!"
    echo "🎯 95% Target ACHIEVED!"
else
    echo "❌ Phase 7 target not met"
    echo "📊 Current: $final_rate%, Target: 95%+"
fi

EOF

chmod +x phase7_completion_check.sh
```

この手順でPhase 7エージェントを起動し、MB8877互換レイヤーの95%達成を目指してください。タイミング精度向上により、真の実用レベルに到達します。