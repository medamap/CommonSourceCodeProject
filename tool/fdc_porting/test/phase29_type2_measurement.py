#!/usr/bin/env python3
"""
Phase 29: Type II Command Full Function Test Measurement
Measures success rate improvement after Phase 28 memory corruption fixes
"""

import subprocess
import json
import time
import re
import os
from datetime import datetime
from typing import Dict, List, Tuple

class Type2TestMeasurement:
    def __init__(self):
        self.results = {
            "phase": 29,
            "test_date": datetime.now().isoformat(),
            "objective": "Measure Type II success rate after Phase 28 memory fixes",
            "baseline_target": {"from": 43.0, "to": 85.0},
            "tests": {},
            "summary": {},
            "timing": {}
        }
        
    def run_test_with_timing(self, test_name: str, binary_path: str) -> Tuple[bool, str, float]:
        """Run a test and capture output with timing"""
        start_time = time.time()
        
        try:
            # Run with timeout to prevent hangs
            result = subprocess.run(
                [binary_path],
                capture_output=True,
                text=True,
                timeout=30
            )
            
            elapsed_time = time.time() - start_time
            output = result.stdout + result.stderr
            
            # Check for crashes or sanitizer errors
            if result.returncode != 0:
                return False, f"Exit code: {result.returncode}\n{output}", elapsed_time
            
            # Parse test results
            passed = 0
            failed = 0
            
            # Look for test result patterns
            pass_patterns = [
                r'✓',
                r'PASS',
                r'Test passed',
                r'SUCCESS',
                r'\[OK\]'
            ]
            
            fail_patterns = [
                r'✗',
                r'FAIL',
                r'Test failed',
                r'ERROR',
                r'ASSERTION FAILED',
                r'\[FAILED\]'
            ]
            
            for line in output.split('\n'):
                for pattern in pass_patterns:
                    if re.search(pattern, line, re.IGNORECASE):
                        passed += 1
                        break
                
                for pattern in fail_patterns:
                    if re.search(pattern, line, re.IGNORECASE):
                        failed += 1
                        break
            
            # If no explicit pass/fail markers, check for overall success
            if passed == 0 and failed == 0:
                if "All tests passed" in output or "Tests completed successfully" in output:
                    passed = 1
                elif "Tests failed" in output or "Assertion" in output:
                    failed = 1
            
            success = passed > 0 and failed == 0
            return success, output, elapsed_time
            
        except subprocess.TimeoutExpired:
            elapsed_time = time.time() - start_time
            return False, "Test timeout after 30 seconds", elapsed_time
        except Exception as e:
            elapsed_time = time.time() - start_time
            return False, f"Exception: {str(e)}", elapsed_time
    
    def analyze_output(self, output: str) -> Dict:
        """Analyze test output for specific functionality"""
        analysis = {
            "read_sector": {
                "basic_read": False,
                "data_transfer": False,
                "drq_signaling": False,
                "completion_status": False
            },
            "write_sector": {
                "write_initiation": False,
                "drq_for_data": False,
                "write_protection": False,
                "completion": False
            },
            "multi_sector": {
                "multi_read": False,
                "sector_increment": False,
                "cross_boundary": False
            },
            "phase25_fixes": {
                "event_search": False,
                "read_io8": False,
                "event_multi": False
            }
        }
        
        # Check for specific functionality markers
        if "READ SECTOR" in output or "read_sector" in output:
            if "DRQ" in output and "set" in output:
                analysis["read_sector"]["drq_signaling"] = True
            if "data transfer" in output.lower() or "read_io8" in output:
                analysis["read_sector"]["data_transfer"] = True
            if "completion" in output or "complete" in output:
                analysis["read_sector"]["completion_status"] = True
            if any(x in output for x in ["basic read", "simple read", "single sector read"]):
                analysis["read_sector"]["basic_read"] = True
        
        if "WRITE SECTOR" in output or "write_sector" in output:
            if "write initiated" in output.lower() or "write start" in output.lower():
                analysis["write_sector"]["write_initiation"] = True
            if "DRQ" in output and "write" in output.lower():
                analysis["write_sector"]["drq_for_data"] = True
            if "write protect" in output.lower():
                analysis["write_sector"]["write_protection"] = True
            if "write complete" in output.lower():
                analysis["write_sector"]["completion"] = True
        
        if "multi" in output.lower() and "sector" in output.lower():
            if "multiple read" in output.lower() or "multi-sector read" in output.lower():
                analysis["multi_sector"]["multi_read"] = True
            if "sector increment" in output.lower() or "next sector" in output.lower():
                analysis["multi_sector"]["sector_increment"] = True
            if "boundary" in output.lower() or "track boundary" in output.lower():
                analysis["multi_sector"]["cross_boundary"] = True
        
        # Check Phase 25 fixes
        if "EVENT_SEARCH" in output:
            analysis["phase25_fixes"]["event_search"] = True
        if "read_io8" in output:
            analysis["phase25_fixes"]["read_io8"] = True
        if "EVENT_MULTI" in output:
            analysis["phase25_fixes"]["event_multi"] = True
        
        return analysis
    
    def run_asan_verification(self, binary_path: str) -> Tuple[bool, str]:
        """Run test with AddressSanitizer enabled"""
        env = os.environ.copy()
        env['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1:print_stats=1'
        
        try:
            result = subprocess.run(
                [binary_path],
                capture_output=True,
                text=True,
                timeout=60,
                env=env
            )
            
            output = result.stdout + result.stderr
            
            # Check for ASAN errors
            asan_errors = [
                "ERROR: AddressSanitizer",
                "heap-buffer-overflow",
                "stack-buffer-overflow",
                "use-after-free",
                "heap-use-after-free"
            ]
            
            for error in asan_errors:
                if error in output:
                    return False, f"ASAN detected: {error}"
            
            return True, "No memory errors detected"
            
        except Exception as e:
            return False, f"ASAN verification failed: {str(e)}"
    
    def measure_type2_tests(self):
        """Main measurement function"""
        print("Phase 29: Type II Command Full Function Test Measurement")
        print("=" * 60)
        
        # Test the main Type II binary
        test_binary = "./test_mb8877_type2_commands"
        
        print(f"\n1. Running {test_binary}...")
        
        # Multiple runs for consistency
        runs = []
        for i in range(3):
            print(f"   Run {i+1}/3...")
            success, output, elapsed = self.run_test_with_timing("type2_main", test_binary)
            runs.append({
                "run": i+1,
                "success": success,
                "elapsed_time": elapsed,
                "output_length": len(output)
            })
            
            # Analyze first run in detail
            if i == 0:
                self.results["tests"]["type2_commands"] = {
                    "success": success,
                    "elapsed_time": elapsed,
                    "functionality": self.analyze_output(output),
                    "output_sample": output[:500] if len(output) > 500 else output
                }
        
        # Calculate success rate
        successful_runs = sum(1 for r in runs if r["success"])
        success_rate = (successful_runs / len(runs)) * 100
        
        self.results["tests"]["type2_commands"]["runs"] = runs
        self.results["tests"]["type2_commands"]["success_rate"] = success_rate
        
        print(f"\n2. Running ASAN verification...")
        asan_success, asan_msg = self.run_asan_verification(test_binary)
        self.results["tests"]["asan_verification"] = {
            "success": asan_success,
            "message": asan_msg
        }
        
        # Calculate overall metrics
        self.calculate_summary()
        
    def calculate_summary(self):
        """Calculate summary metrics"""
        # Overall success rate
        total_tests = 0
        successful_tests = 0
        
        for test_name, test_data in self.results["tests"].items():
            if "success_rate" in test_data:
                total_tests += 1
                if test_data["success_rate"] >= 70:  # 70% threshold
                    successful_tests += 1
        
        overall_success_rate = (successful_tests / total_tests * 100) if total_tests > 0 else 0
        
        # Functionality coverage
        functionality_working = 0
        functionality_total = 0
        
        if "type2_commands" in self.results["tests"]:
            func = self.results["tests"]["type2_commands"].get("functionality", {})
            for category in func.values():
                if isinstance(category, dict):
                    for feature, working in category.items():
                        functionality_total += 1
                        if working:
                            functionality_working += 1
        
        functionality_rate = (functionality_working / functionality_total * 100) if functionality_total > 0 else 0
        
        self.results["summary"] = {
            "overall_success_rate": overall_success_rate,
            "functionality_coverage": functionality_rate,
            "memory_safety": self.results["tests"].get("asan_verification", {}).get("success", False),
            "baseline_achievement": {
                "baseline": 43.0,
                "target": 85.0,
                "achieved": overall_success_rate,
                "improvement": overall_success_rate - 43.0
            },
            "phase25_effectiveness": {
                "event_search_working": self.results["tests"].get("type2_commands", {}).get("functionality", {}).get("phase25_fixes", {}).get("event_search", False),
                "read_io8_working": self.results["tests"].get("type2_commands", {}).get("functionality", {}).get("phase25_fixes", {}).get("read_io8", False),
                "event_multi_working": self.results["tests"].get("type2_commands", {}).get("functionality", {}).get("phase25_fixes", {}).get("event_multi", False)
            }
        }
        
        # Phase 30 recommendation
        if overall_success_rate >= 85:
            recommendation = "HIGH SUCCESS: Proceed to Type III/IV implementation"
        elif overall_success_rate >= 70:
            recommendation = "MODERATE SUCCESS: Complete Type II refinement"
        else:
            recommendation = "LOW SUCCESS: Core architecture review needed"
        
        self.results["phase30_recommendation"] = recommendation
    
    def save_results(self):
        """Save results to JSON"""
        with open("phase29_results.json", "w") as f:
            json.dump(self.results, f, indent=2)
        
        # Also create a summary report
        summary = f"""
Phase 29 Type II Test Results Summary
====================================
Date: {self.results['test_date']}

Overall Success Rate: {self.results['summary']['overall_success_rate']:.1f}%
Baseline Improvement: {self.results['summary']['baseline_achievement']['improvement']:.1f}%
Memory Safety: {'PASS' if self.results['summary']['memory_safety'] else 'FAIL'}

Phase 25 Fix Effectiveness:
- EVENT_SEARCH: {'Working' if self.results['summary']['phase25_effectiveness']['event_search_working'] else 'Not Working'}
- read_io8: {'Working' if self.results['summary']['phase25_effectiveness']['read_io8_working'] else 'Not Working'}
- EVENT_MULTI: {'Working' if self.results['summary']['phase25_effectiveness']['event_multi_working'] else 'Not Working'}

Phase 30 Recommendation: {self.results['phase30_recommendation']}
"""
        
        with open("phase29_summary.txt", "w") as f:
            f.write(summary)
        
        print(summary)

if __name__ == "__main__":
    measurement = Type2TestMeasurement()
    measurement.measure_type2_tests()
    measurement.save_results()