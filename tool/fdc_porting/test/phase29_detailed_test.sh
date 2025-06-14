#!/bin/bash
# Phase 29: Detailed Type II Test Execution with Error Capture

echo "Phase 29: Type II Command Detailed Test Execution"
echo "================================================="
echo "Date: $(date)"
echo ""

# Function to run test with detailed output
run_test() {
    local test_name=$1
    local test_binary=$2
    
    echo "Running $test_name..."
    echo "Binary: $test_binary"
    
    # Check if binary exists
    if [ ! -f "$test_binary" ]; then
        echo "ERROR: Binary not found: $test_binary"
        return 1
    fi
    
    # Run with timeout and capture all output
    timeout 30s $test_binary > ${test_name}_output.txt 2>&1
    local exit_code=$?
    
    echo "Exit code: $exit_code"
    
    if [ $exit_code -eq 139 ]; then
        echo "SEGMENTATION FAULT detected"
        
        # Try to get backtrace with lldb
        echo "Getting backtrace..."
        echo "run" | lldb -b $test_binary 2>&1 | grep -A 20 "stop reason" > ${test_name}_backtrace.txt
        
        # Show first few lines of backtrace
        head -10 ${test_name}_backtrace.txt
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT after 30 seconds"
    else
        # Show output
        echo "Output sample:"
        head -20 ${test_name}_output.txt
    fi
    
    echo ""
    return $exit_code
}

# Test 1: Main Type II test
run_test "type2_main" "./test_mb8877_type2_commands"

# Test 2: Check for other Type II related tests
if [ -f "./test_mb8877_type2_d88" ]; then
    run_test "type2_d88" "./test_mb8877_type2_d88"
fi

# Test 3: Run with AddressSanitizer if available
echo "Checking for ASAN-enabled binary..."
if [ -f "./test_mb8877_type2_commands_asan" ]; then
    ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:print_stats=1 run_test "type2_asan" "./test_mb8877_type2_commands_asan"
else
    echo "No ASAN binary found, building one..."
    # Try to build with ASAN
    make clean
    CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g" LDFLAGS="-fsanitize=address" make test_mb8877_type2_commands 2>&1 | tee asan_build.log
    
    if [ $? -eq 0 ]; then
        echo "ASAN build successful, running test..."
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:print_stats=1 run_test "type2_asan_fresh" "./test_mb8877_type2_commands"
    else
        echo "ASAN build failed"
    fi
fi

# Summary
echo ""
echo "Test Summary"
echo "============"
echo "Type II main test: Exit code $?"

# Check for core dumps
if ls core.* 2>/dev/null; then
    echo "Core dumps found:"
    ls -la core.*
fi