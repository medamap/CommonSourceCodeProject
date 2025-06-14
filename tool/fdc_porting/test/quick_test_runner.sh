#\!/bin/bash

# Quick test runner with timeout protection
TEST_LIST=(
    "simple_test"
    "simple_mb8877_test"
    "test_mb8877_type3_commands"
    "test_mb8877_type4_commands"
    "test_mb8877_error_handling"
)

echo "=== Phase 20 Quick Validation ==="
echo "Testing subset of ${#TEST_LIST[@]} tests"
echo

PASSED=0
FAILED=0

for test in "${TEST_LIST[@]}"; do
    echo -n "Testing $test... "
    if [ -x "./$test" ]; then
        # Run with timeout and capture exit code
        timeout 5 ./$test > /tmp/test_output.txt 2>&1
        EXIT_CODE=$?
        
        if [ $EXIT_CODE -eq 0 ]; then
            echo "PASS"
            PASSED=$((PASSED + 1))
        elif [ $EXIT_CODE -eq 124 ]; then
            echo "TIMEOUT"
            FAILED=$((FAILED + 1))
        elif [ $EXIT_CODE -eq 139 ]; then
            echo "SEGFAULT"
            FAILED=$((FAILED + 1))
        else
            echo "FAIL (exit $EXIT_CODE)"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "NOT FOUND"
        FAILED=$((FAILED + 1))
    fi
done

echo
echo "=== Summary ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "Success Rate: $(( PASSED * 100 / (PASSED + FAILED) ))%"
