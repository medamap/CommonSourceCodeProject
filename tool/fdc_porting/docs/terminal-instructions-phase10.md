# Phase 10 ターミナル実行指示

## Phase 10: Write Track実装

新しいターミナルを開いて以下のコマンドを実行してください：

```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject
claude --dangerously-skip-permissions --print '{
  "command": "execute_phase10_write_track_implementation",
  "args": {
    "task_id": "mb8877_compat/phase10_write_track",
    "instruction_file": "tool/fdc_porting/docs/instructions/phase10-write-track-implementation.md",
    "reference_files": [
      "src/vm/mb8877_compat.cpp",
      "src/vm/mb8877.cpp",
      "src/vm/disk.h",
      "tool/fdc_porting/docs/reports/phase1-completion-report.md",
      "tool/fdc_porting/docs/write_track_implementation_plan.md"
    ],
    "implementation_targets": {
      "primary_file": "src/vm/mb8877_compat.cpp",
      "target_lines": "282-285",
      "function": "write_io8() WRITE_TRACK case"
    },
    "test_requirements": {
      "new_test_file": "tool/fdc_porting/test/test_mb8877_write_track.cpp",
      "update_test_file": "tool/fdc_porting/test/test_mb8877_type3_commands.cpp",
      "maintain_pass_rate": "72.7%",
      "format_verification": true
    },
    "validation_requirements": {
      "cpp_compilation": true,
      "test_pass": true,
      "format_operations": [
        "single_density_format",
        "double_density_format",
        "sector_size_variations"
      ]
    },
    "output_files": [
      "src/vm/mb8877_compat.cpp",
      "tool/fdc_porting/test/test_mb8877_write_track.cpp",
      "tool/fdc_porting/docs/reports/phase10-write-track-report.md"
    ],
    "output_instruction": "Phase 10の作業を完了したら、レポートをphase10-write-track-report.mdに出力し、実装の詳細とテスト結果を記載してください。"
  },
  "implementation_focus": "write_track_data_handling",
  "critical_features": [
    "format_byte_parsing",
    "id_field_writing", 
    "data_field_writing",
    "crc_calculation",
    "gap_processing"
  ]
}'
```

## エージェントへの指示内容

このコマンドにより、WriteTrackAgent-Phase10は以下を実行します：

1. **Write Trackデータ処理の実装**
   - mb8877_compat.cppの282-285行目のTODO部分を実装
   - フォーマットバイトの解析処理
   - IDフィールドとデータフィールドの書き込み
   - CRC計算とギャップ処理

2. **テストの作成と実行**
   - 新規test_mb8877_write_track.cppの作成
   - 既存のtype3_commands.cppテストの更新
   - フォーマット機能の包括的テスト

3. **検証とレポート**
   - 実装の動作確認
   - 既存テストの回帰確認（72.7%維持）
   - phase10-write-track-report.mdの作成

## 期待される成果

- ディスクフォーマット機能の完全動作
- Write Trackコマンドの正確な実装
- テストカバレッジの向上
- 詳細な実装レポート