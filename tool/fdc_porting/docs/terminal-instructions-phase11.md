# Phase 11 ターミナル実行指示

## Phase 11: タイミング精度向上

Phase 10完了後、新しいターミナルを開いて以下のコマンドを実行してください：

```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject
claude --dangerously-skip-permissions --print '{
  "command": "execute_phase11_timing_precision",
  "args": {
    "task_id": "mb8877_compat/phase11_timing",
    "instruction_file": "tool/fdc_porting/docs/instructions/phase11-timing-precision.md",
    "reference_files": [
      "src/vm/mb8877_compat.cpp",
      "src/vm/mb8877.cpp",
      "tool/fdc_porting/docs/reports/phase10-write-track-report.md",
      "tool/fdc_porting/test/test_mb8877_timing.cpp"
    ],
    "implementation_targets": [
      {
        "function": "get_cur_position",
        "lines": "1642-1646",
        "current": "return fdc[drvreg].cur_position",
        "requirement": "calculate_from_elapsed_time"
      },
      {
        "function": "get_usec_to_start_trans",
        "lines": "1648-1653",
        "current": "hardcoded_values",
        "requirement": "sector_position_based"
      },
      {
        "function": "get_usec_to_next_trans_pos",
        "lines": "1655-1659",
        "current": "hardcoded_values",
        "requirement": "transfer_rate_based"
      },
      {
        "function": "get_usec_to_detect_index_hole",
        "lines": "1661-1668",
        "current": "simplified_fixed_time",
        "requirement": "position_accurate"
      }
    ],
    "timing_specifications": {
      "disk_rpm": [300, 360],
      "mfm_byte_time": 31.25,
      "fm_byte_time": 62.5,
      "index_pulse_width": 4000,
      "motor_start_time": 500000
    },
    "test_requirements": {
      "update_test_file": "tool/fdc_porting/test/test_mb8877_timing.cpp",
      "new_test_file": "tool/fdc_porting/test/test_mb8877_performance.cpp",
      "timing_accuracy": "within_5_percent",
      "latency_measurement": true
    },
    "validation_requirements": {
      "timing_precision": "microsecond",
      "transfer_rate_accuracy": true,
      "index_detection_accuracy": true,
      "multi_density_support": ["FM", "MFM"]
    },
    "output_files": [
      "src/vm/mb8877_compat.cpp",
      "tool/fdc_porting/test/test_mb8877_performance.cpp",
      "tool/fdc_porting/docs/reports/phase11-timing-report.md"
    ],
    "output_instruction": "Phase 11の作業を完了したら、レポートをphase11-timing-report.mdに出力し、実装したタイミング計算の詳細と測定結果を記載してください。"
  },
  "implementation_focus": "precise_timing_calculation",
  "critical_accuracy": [
    "byte_transfer_timing",
    "sector_access_latency",
    "rotation_position_tracking",
    "index_hole_detection"
  ]
}'
```

## エージェントへの指示内容

このコマンドにより、TimingAgent-Phase11は以下を実行します：

1. **タイミング関数の正確な実装**
   - get_cur_position() - 経過時間ベースの位置計算
   - get_usec_to_start_trans() - セクタ位置ベースの時間計算
   - get_usec_to_next_trans_pos() - 転送レートベースの計算
   - get_usec_to_detect_index_hole() - 位置精密計算

2. **精度検証とテスト**
   - タイミング精度テストの拡張
   - パフォーマンステストの新規作成
   - FM/MFM両モードでの検証

3. **測定とレポート**
   - 転送速度とレイテンシの測定
   - 実機相当精度の確認
   - phase11-timing-report.mdの作成

## 期待される成果

- マイクロ秒精度のタイミング実装
- 実機相当の転送速度
- 正確なセクタアクセスタイミング
- 詳細な性能測定レポート