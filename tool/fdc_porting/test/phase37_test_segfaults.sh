#!/bin/bash

echo "Testing for segfaults in individual tests..."

# Function to run a test and check for segfault
run_test() {
    local test_name=$1
    echo -n "Testing $test_name... "
    
    if ./$test_name > /tmp/${test_name}.log 2>&1; then
        echo "OK"
        return 0
    else
        if grep -q "Segmentation fault" /tmp/${test_name}.log; then
            echo "SEGFAULT!"
            return 1
        else
            echo "FAILED (no segfault)"
            return 2
        fi
    fi
}

# Run each test
tests=(
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
    "test_safe_disk"
    "test_mb8877_safe_disk_integration"
    "test_mb8877_type2_complete"
    "test_mb8877_type3_type4_commands"
)

segfault_count=0
fail_count=0
pass_count=0

for test in "${tests[@]}"; do
    if [ -f "./$test" ]; then
        run_test $test
        result=$?
        if [ $result -eq 0 ]; then
            ((pass_count++))
        elif [ $result -eq 1 ]; then
            ((segfault_count++))
        else
            ((fail_count++))
        fi
    else
        echo "$test not found"
    fi
done

echo ""
echo "Summary:"
echo "  Passed: $pass_count"
echo "  Failed: $fail_count"
echo "  Segfaults: $segfault_count"
echo "  Total: ${#tests[@]}"