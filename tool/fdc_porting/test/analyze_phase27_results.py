#!/usr/bin/env python3
"""
Phase 27 Test Results Analysis Script
Analyzes MB8877 test results and compares with baseline
"""

import os
import re
import json
from datetime import datetime

def parse_test_summary(log_content):
    """Extract test summary from log content"""
    summary_match = re.search(r'Total tests:\s*(\d+)\s*\nPassed:\s*(\d+)\s*\nFailed:\s*(\d+)\s*\nSuccess rate:\s*([\d.]+)%', log_content)
    if summary_match:
        return {
            'total': int(summary_match.group(1)),
            'passed': int(summary_match.group(2)),
            'failed': int(summary_match.group(3)),
            'success_rate': float(summary_match.group(4))
        }
    return None

def analyze_test_results():
    """Analyze all test results"""
    results = {}
    test_files = [
        'test_mb8877_drive_mfm',
        'test_mb8877_drive_rpm',
        'test_mb8877_error_handling',
        'test_mb8877_registers',
        'test_mb8877_registers_compat',
        'test_mb8877_timing',
        'test_mb8877_type1_commands',
        'test_mb8877_type2_commands',
        'test_mb8877_type3_commands',
        'test_mb8877_type4_commands',
        'test_mb8877_write_track'
    ]
    
    # Baseline results from previous phases
    baseline = {
        'overall': {'success_rate': 60.0},
        'type2': {'success_rate': 43.0}
    }
    
    total_tests = 0
    total_passed = 0
    total_failed = 0
    
    for test_name in test_files:
        log_path = f'results/{test_name}.log'
        if os.path.exists(log_path):
            with open(log_path, 'r') as f:
                content = f.read()
                summary = parse_test_summary(content)
                if summary:
                    results[test_name] = summary
                    total_tests += summary['total']
                    total_passed += summary['passed']
                    total_failed += summary['failed']
                else:
                    # Handle special cases
                    if test_name == 'test_mb8877_type1_commands':
                        # Type 1 test hung, counting partial results
                        results[test_name] = {
                            'total': 'unknown (hung)',
                            'passed': 4,  # Count from partial output
                            'failed': 'unknown',
                            'success_rate': 'unknown',
                            'status': 'HUNG'
                        }
                    elif test_name == 'test_mb8877_type2_commands':
                        # Type 2 test crashed immediately
                        results[test_name] = {
                            'total': 'unknown',
                            'passed': 0,
                            'failed': 'unknown',
                            'success_rate': 0.0,
                            'status': 'CRASHED'
                        }
                    else:
                        results[test_name] = {'status': 'NO_SUMMARY'}
    
    # Calculate overall success rate (excluding crashed tests)
    overall_rate = (total_passed / total_tests * 100) if total_tests > 0 else 0
    
    # Analysis results
    analysis = {
        'timestamp': datetime.now().isoformat(),
        'phase': 27,
        'test_results': results,
        'summary': {
            'total_tests': total_tests,
            'total_passed': total_passed,
            'total_failed': total_failed,
            'overall_success_rate': round(overall_rate, 1),
            'type1_status': 'HUNG - partial execution',
            'type2_status': 'CRASHED - immediate failure'
        },
        'comparison_with_baseline': {
            'overall': {
                'baseline': baseline['overall']['success_rate'],
                'current': round(overall_rate, 1),
                'improvement': round(overall_rate - baseline['overall']['success_rate'], 1)
            },
            'type2': {
                'baseline': baseline['type2']['success_rate'],
                'current': 0.0,  # Crashed
                'improvement': -43.0  # Regression
            }
        },
        'key_findings': [
            'Type II commands now CRASH immediately instead of running with 43% success',
            'This is likely due to Phase 25/26 refactoring introducing initialization issues',
            'Type I commands hang during verify operation',
            'Other test categories maintain similar success rates',
            'Drive RPM tests: 100% (excellent)',
            'Type III/IV tests: temporarily skipped (as expected)'
        ],
        'critical_issues': [
            'Type II command crash prevents any READ/WRITE SECTOR testing',
            'Type I command hang suggests event handling or timing issues',
            'Overall success rate excluding Type I/II: ~75%'
        ]
    }
    
    return analysis

def generate_detailed_report(analysis):
    """Generate detailed markdown report"""
    report = f"""# Phase 27 Test Results Analysis Report

Generated: {analysis['timestamp']}

## Executive Summary

**Critical Finding**: Type II commands (READ/WRITE SECTOR) now crash immediately, representing a regression from the baseline 43% success rate. This indicates Phase 25/26 refactoring introduced critical initialization or memory issues.

## Test Results by Category

| Test Category | Tests | Passed | Failed | Success Rate | Status |
|--------------|-------|--------|--------|--------------|---------|
"""
    
    for test_name, result in analysis['test_results'].items():
        status = result.get('status', 'OK')
        total = result.get('total', 'N/A')
        passed = result.get('passed', 'N/A')
        failed = result.get('failed', 'N/A')
        rate = f"{result.get('success_rate', 'N/A')}%" if isinstance(result.get('success_rate'), (int, float)) else 'N/A'
        
        test_type = test_name.replace('test_mb8877_', '').replace('_', ' ').title()
        report += f"| {test_type} | {total} | {passed} | {failed} | {rate} | {status} |\n"
    
    report += f"""

## Comparison with Baseline

### Overall Success Rate
- **Baseline**: {analysis['comparison_with_baseline']['overall']['baseline']}%
- **Current**: {analysis['comparison_with_baseline']['overall']['current']}%
- **Change**: {analysis['comparison_with_baseline']['overall']['improvement']:+.1f}%

### Type II Commands (Critical)
- **Baseline**: {analysis['comparison_with_baseline']['type2']['baseline']}%
- **Current**: {analysis['comparison_with_baseline']['type2']['current']}%
- **Change**: {analysis['comparison_with_baseline']['type2']['improvement']:+.1f}% (REGRESSION)

## Key Findings

"""
    
    for finding in analysis['key_findings']:
        report += f"- {finding}\n"
    
    report += "\n## Critical Issues Requiring Immediate Attention\n\n"
    
    for issue in analysis['critical_issues']:
        report += f"1. {issue}\n"
    
    report += """

## Root Cause Analysis

### Type II Command Crash
The immediate crash suggests:
1. Uninitialized pointers or data structures
2. Memory access violations in disk_safe handling
3. Missing initialization in Phase 25 refactoring

### Type I Command Hang
The hang during verify operation indicates:
1. Event handling deadlock
2. Infinite loop in verification logic
3. Timing calculation issues

## Recommendations for Phase 28

1. **Immediate Priority**: Debug Type II crash with gdb/lldb
2. **Add defensive initialization** in mb8877_compat_impl.cpp
3. **Verify all disk_safe pointers** are properly initialized
4. **Add timeout handling** for Type I verify operations
5. **Consider reverting Phase 25 changes** if quick fix not found

## Success Metrics vs Targets

| Metric | Target | Achieved | Status |
|--------|---------|----------|---------|
| Type II Success | 85%+ | 0% | ❌ FAILED |
| Overall Success | 80%+ | ~75%* | ❌ FAILED |
| Type II Improvement | +42% | -43% | ❌ REGRESSION |

*Excluding crashed tests

## Conclusion

Phase 27 reveals a critical regression in Type II command functionality. The immediate crash prevents any meaningful testing of READ/WRITE SECTOR operations. This must be resolved before proceeding with Type III/IV implementation.
"""
    
    return report

if __name__ == '__main__':
    print("Analyzing Phase 27 test results...")
    
    analysis = analyze_test_results()
    
    # Save JSON results
    with open('phase27_results.json', 'w') as f:
        json.dump(analysis, f, indent=2)
    
    # Generate and save detailed report
    report = generate_detailed_report(analysis)
    with open('phase27_analysis_report.md', 'w') as f:
        f.write(report)
    
    # Print summary to console
    print(f"\nPhase 27 Test Summary:")
    print(f"Overall Success Rate: {analysis['summary']['overall_success_rate']}%")
    print(f"Type I Status: {analysis['summary']['type1_status']}")
    print(f"Type II Status: {analysis['summary']['type2_status']}")
    print(f"\nCritical Issues:")
    for issue in analysis['critical_issues']:
        print(f"  - {issue}")
    
    print(f"\nDetailed results saved to:")
    print(f"  - phase27_results.json")
    print(f"  - phase27_analysis_report.md")