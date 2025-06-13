# Phase 13 ターミナル実行指示

## Phase 13: MB89311拡張機能実装

Phase 12完了後、新しいターミナルを開いて以下のコマンドを実行してください：

```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject
claude --dangerously-skip-permissions --print '{
  "command": "execute_phase13_mb89311_extension",
  "args": {
    "task_id": "mb8877_compat/phase13_mb89311",
    "instruction_file": "tool/fdc_porting/docs/instructions/phase13-mb89311-extension.md",
    "reference_files": [
      "src/vm/mb8877_compat.cpp",
      "src/vm/mb8877.cpp",
      "src/vm/mb8877_compat.h",
      "tool/fdc_porting/docs/reports/phase12c-drive-rpm-report.md",
      "tool/fdc_porting/test/test_mb8877_type3_commands.cpp"
    ],
    "implementation_targets": [
      {
        "area": "extended_commands",
        "location": "lines_715-749",
        "commands": ["0xFC_delay", "0xFD_assign_param", "0xFE_assign_mode", "0xFF_reset"],
        "requirement": "enhanced_mb89311_processing"
      },
      {
        "area": "cmd_format",
        "location": "lines_1752-1758", 
        "current": "simple_writetrack_call",
        "requirement": "mb89311_specific_format"
      },
      {
        "area": "read_write_after_seek",
        "location": "lines_550-560",
        "commands": ["0x44_read_after_seek", "0x64_write_after_seek"],
        "requirement": "extended_mode_operations"
      }
    ],
    "functional_requirements": {
      "extended_commands": {
        "delay_command": "0xFC_parameter_based_delay",
        "assign_parameter": "0xFD_parameter_storage",
        "assign_mode": "0xFE_mode_switching",
        "reset_command": "0xFF_chip_reset"
      },
      "format_enhancements": {
        "mb89311_format": "extended_format_options",
        "parameter_usage": "stored_parameters_application",
        "special_formats": "non_standard_format_support"
      },
      "seek_extensions": {
        "read_after_seek": "automatic_read_after_positioning",
        "write_after_seek": "automatic_write_after_positioning"
      }
    },
    "test_requirements": {
      "new_test_file": "tool/fdc_porting/test/test_mb8877_mb89311.cpp",
      "test_categories": [
        "extended_command_processing",
        "mb89311_format_operations", 
        "parameter_management",
        "mode_switching",
        "seek_extension_functions"
      ],
      "integration_tests": "full_mb89311_compatibility",
      "maintain_compatibility": "standard_mb8877_unchanged"
    },
    "validation_requirements": {
      "cpp_compilation": true,
      "test_execution": true,
      "mb89311_compatibility": "verified_functional",
      "backward_compatibility": "mb8877_standard_preserved"
    },
    "output_files": [
      "src/vm/mb8877_compat.cpp",
      "tool/fdc_porting/test/test_mb8877_mb89311.cpp",
      "tool/fdc_porting/docs/reports/phase13-mb89311-report.md"
    ],
    "project_completion": {
      "final_phase": true,
      "implementation_target": "100%",
      "compatibility_verification": "complete_mb8877_mb89311_support"
    },
    "output_instruction": "Phase 13の作業完了後は必ずJSON形式でレポートし、MB89311拡張機能の実装詳細、最終的な実装完成度、プロジェクト全体の総括を記載してください。作業完了時は必ずコミットしてください。"
  },
  "implementation_focus": "mb89311_extended_functionality",
  "critical_aspects": [
    "extended_command_processing",
    "format_command_enhancement",
    "parameter_management_system",
    "backward_compatibility_preservation"
  ],
  "output_format": "json",
  "strict_json_output": true,
  "no_text_output": true
}'
```

## エージェントへの指示内容

このコマンドにより、ExtensionAgent-Phase13は以下を実行します：

1. **MB89311拡張コマンド強化**
   - 0xFC-0xFF拡張コマンドの完全実装
   - パラメータ管理システムの構築
   - 拡張モードでの動作変更

2. **cmd_format()独自実装**
   - MB89311固有のフォーマット機能
   - 拡張パラメータの活用
   - 特殊フォーマットオプション

3. **拡張機能の統合**
   - Read-after-seek/Write-after-seek実装
   - 拡張モードと標準モードの互換性
   - 包括的なテストスイート

4. **プロジェクト完成**
   - 実装完成度100%達成
   - 最終的な互換性確認
   - 総合評価レポート

## 期待される成果

- MB89311拡張機能の完全実装
- 実装完成度100%の達成
- 完全なMB8877/MB89311互換性
- プロジェクト完成の総括レポート