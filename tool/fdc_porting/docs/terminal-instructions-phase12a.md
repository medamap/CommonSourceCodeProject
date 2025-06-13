# Phase 12a ターミナル実行指示

## Phase 12a: set_drive_mfm()実装

Phase 11完了後、新しいターミナルを開いて以下のコマンドを実行してください：

```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject
claude --dangerously-skip-permissions --print '{
  "command": "execute_phase12a_set_drive_mfm",
  "args": {
    "task_id": "mb8877_compat/phase12a_drive_mfm",
    "instruction_file": "tool/fdc_porting/docs/instructions/phase12a-set-drive-mfm.md",
    "reference_files": [
      "src/vm/mb8877_compat.cpp",
      "src/vm/mb8877.cpp",
      "src/vm/disk.h",
      "tool/fdc_porting/docs/reports/phase11-timing-report.md",
      "tool/fdc_porting/test/test_mb8877_timing.cpp"
    ],
    "implementation_target": {
      "function": "set_drive_mfm",
      "file": "src/vm/mb8877_compat.cpp",
      "lines": "1071-1077",
      "current_state": "empty_mock_implementation",
      "requirement": "full_fm_mfm_switching"
    },
    "functional_requirements": {
      "fm_mode": {
        "transfer_rate": "125kbps",
        "byte_time": "62.5µs",
        "density": "single"
      },
      "mfm_mode": {
        "transfer_rate": "250kbps", 
        "byte_time": "31.25µs",
        "density": "double"
      },
      "drive_support": "per_drive_individual_setting",
      "error_handling": "invalid_drive_number_check"
    },
    "test_requirements": {
      "new_test_file": "tool/fdc_porting/test/test_mb8877_drive_mfm.cpp",
      "test_cases": [
        "basic_fm_mfm_switching",
        "transfer_rate_verification",
        "individual_drive_setting",
        "invalid_drive_handling"
      ],
      "timing_integration": "update_existing_timing_tests",
      "maintain_compatibility": true
    },
    "validation_requirements": {
      "cpp_compilation": true,
      "test_execution": true,
      "timing_accuracy": "within_existing_precision",
      "fm_mfm_switching": "verified_functional"
    },
    "output_files": [
      "src/vm/mb8877_compat.cpp",
      "tool/fdc_porting/test/test_mb8877_drive_mfm.cpp",
      "tool/fdc_porting/docs/reports/phase12a-drive-mfm-report.md"
    ],
    "output_instruction": "Phase 12aの作業完了後は必ずJSON形式でレポートし、set_drive_mfm()の実装仕様と検証結果を記載してください。作業完了時は必ずコミットしてください。"
  },
  "implementation_focus": "fm_mfm_density_switching",
  "critical_aspects": [
    "transfer_rate_control",
    "per_drive_setting",
    "timing_integration",
    "error_boundary_handling"
  ],
  "output_format": "json",
  "strict_json_output": true,
  "no_text_output": true
}'
```

## エージェントへの指示内容

このコマンドにより、DriveSettingsAgent-Phase12aは以下を実行します：

1. **set_drive_mfm()の完全実装**
   - 1071-1077行目の空実装を完全機能に置き換え
   - FM（125kbps）/MFM（250kbps）の転送レート制御
   - ドライブごとの個別設定管理

2. **テストの作成と検証**
   - test_mb8877_drive_mfm.cpp新規作成
   - FM/MFM切り替えの包括的テスト
   - 転送レート変更の検証

3. **JSON形式でのレポート**
   - 実装の詳細仕様
   - テスト結果と検証データ
   - Phase 12bへの引き継ぎ情報

## 期待される成果

- FM/MFM密度切り替えの完全動作
- ドライブごとの個別設定機能
- 転送レート制御の正確な実装
- 詳細なJSON形式レポート