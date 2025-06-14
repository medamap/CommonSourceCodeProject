#\!/bin/bash

# Phase 20 Validation Test Script
echo "=== Phase 20: MB8877 Final Validation Testing ==="
echo "Date: $(date)"
echo "Working Directory: $(pwd)"
echo

# Initialize counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SEGFAULT_TESTS=0

# Test list
TESTS=(
    "test_mb8877_registers"
    "test_mb8877_type1_commands"
    "test_mb8877_type2_commands"
    "test_mb8877_type3_commands"
    "test_mb8877_type4_commands"
    "test_mb8877_error_handling"
    "test_mb8877_timing"
    "test_mb8877_write_track"
    "test_mb8877_drive_mfm"
    "test_mb8877_drive_rpm"
    "test_mb8877_registers_compat"
)

# Additional simple tests
SIMPLE_TESTS=(
    "simple_test"
    "simple_mb8877_test"
    "simple_test_track_size"
)

# All tests combined
ALL_TESTS=("${TESTS[@]}" "${SIMPLE_TESTS[@]}")

# Function to run a single test
run_test() {
    local test_name=$1
    local test_num=$2
    local total=$3
    
    echo "=== Test $test_num/$total: $test_name ==="
    echo "Starting at: $(date +%H:%M:%S)"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ -x "./$test_name" ]; then
        # Run test with timeout protection
        ( ./$test_name 2>&1 ) &
        TEST_PID=$\!
        
        # Wait up to 10 seconds
        COUNTER=0
        while [ $COUNTER -lt 10 ]; do
            if \! kill -0 $TEST_PID 2>/dev/null; then
                # Process finished
                wait $TEST_PID
                EXIT_CODE=$?
                
                if [ $EXIT_CODE -eq 0 ]; then
                    echo "✓ PASSED"
                    PASSED_TESTS=$((PASSED_TESTS + 1))
                elif [ $EXIT_CODE -eq 139 ]; then
                    echo "✗ SEGMENTATION FAULT"
                    SEGFAULT_TESTS=$((SEGFAULT_TESTS + 1))
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                else
                    echo "✗ FAILED (exit code: $EXIT_CODE)"
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                fi
                break
            fi
            sleep 1
            COUNTER=$((COUNTER + 1))
        done
        
        # If still running after 10 seconds, kill it
        if kill -0 $TEST_PID 2>/dev/null; then
            echo "✗ TIMEOUT (killed after 10s)"
            kill -9 $TEST_PID 2>/dev/null
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        echo "✗ NOT FOUND"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    echo "Completed at: $(date +%H:%M:%S)"
    echo
}

# Run all tests
TEST_NUM=1
for test in "${ALL_TESTS[@]}"; do
    run_test "$test" "$TEST_NUM" "${#ALL_TESTS[@]}"
    TEST_NUM=$((TEST_NUM + 1))
done

# Summary
echo "=== PHASE 20 TEST SUMMARY ==="
echo "Total Tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"
echo "Segfaults: $SEGFAULT_TESTS"
echo "Success Rate: $(( PASSED_TESTS * 100 / TOTAL_TESTS ))%"
echo

# Generate JSON report
cat > phase20_results.json << EOJSON
{
  "phase": 20,
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "summary": {
    "total_tests": $TOTAL_TESTS,
    "passed": $PASSED_TESTS,
    "failed": $FAILED_TESTS,
    "segfaults": $SEGFAULT_TESTS,
    "success_rate": $(( PASSED_TESTS * 100 / TOTAL_TESTS ))
  },
  "implementation_completeness": {
    "estimate_percent": 75,
    "core_functionality": "mostly_complete",
    "missing_features": [
      "Some timing edge cases",
      "Advanced error handling scenarios",
      "Full MB89311 compatibility"
    ]
  },
  "recommendations": {
    "phase_21": [
      "Fix segmentation faults in failing tests",
      "Implement missing disk insertion detection",
      "Improve event timing accuracy",
      "Add comprehensive error recovery"
    ]
  }
}
EOJSON

echo "Results saved to phase20_results.json"
