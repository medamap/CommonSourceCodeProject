#!/bin/bash
# MB8877 Implementation Comparison Test Script
# This script runs the test suite against both original and compat implementations

echo "=========================================="
echo "MB8877 Implementation Comparison Test"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Create results directory
mkdir -p test_results
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Function to run tests
run_tests() {
    local impl_name=$1
    local cpp_file=$2
    local result_file=$3
    
    echo "----------------------------------------"
    echo "Testing: $impl_name"
    echo "----------------------------------------"
    
    # Create a test wrapper for the implementation
    cat > mb8877_${impl_name}_wrapper.cpp << EOF
// Auto-generated test wrapper for $impl_name
#define STANDALONE_TEST

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment first to provide all required types
#include "mock_environment.h"

// Include appropriate header based on implementation
EOF

    if [ "$impl_name" = "compat" ]; then
        echo '#include "../src/vm/mb8877_compat.h"' >> mb8877_${impl_name}_wrapper.cpp
        echo '#include "../src/vm/mb8877_compat.cpp"' >> mb8877_${impl_name}_wrapper.cpp
    else
        echo '#include "../src/vm/mb8877.h"' >> mb8877_${impl_name}_wrapper.cpp
        echo '#include "../src/vm/mb8877.cpp"' >> mb8877_${impl_name}_wrapper.cpp
    fi
    
    # Update Makefile to use the specific wrapper
    cp mb8877_${impl_name}_wrapper.cpp mb8877_test_wrapper.cpp
    
    # Clean previous builds
    make clean > /dev/null 2>&1
    
    # Build test suite
    echo -n "Building test suite... "
    if make all > build_${impl_name}.log 2>&1; then
        echo -e "${GREEN}OK${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        echo "Build errors saved to build_${impl_name}.log"
        return 1
    fi
    
    # Run all tests and capture results
    echo "Running tests..."
    echo "Test Results for $impl_name" > $result_file
    echo "=========================" >> $result_file
    echo "" >> $result_file
    
    local total_tests=0
    local passed_tests=0
    
    # Run each test executable
    for test_exe in test_mb8877_*; do
        if [[ -x "$test_exe" && "$test_exe" != *.cpp && "$test_exe" != *.o ]]; then
            echo -n "  Running $test_exe... "
            
            # Run test and capture output
            ./$test_exe > temp_output.txt 2>&1
            exit_code=$?
            
            # Count passed/failed tests from output
            local test_passed=$(grep -c "PASSED" temp_output.txt || echo 0)
            local test_failed=$(grep -c "FAILED" temp_output.txt || echo 0)
            local test_total=$((test_passed + test_failed))
            
            if [ $test_total -eq 0 ]; then
                # If no PASSED/FAILED markers, use exit code
                if [ $exit_code -eq 0 ]; then
                    test_passed=1
                    test_total=1
                else
                    test_failed=1
                    test_total=1
                fi
            fi
            
            total_tests=$((total_tests + test_total))
            passed_tests=$((passed_tests + test_passed))
            
            # Display result
            if [ $test_failed -eq 0 ] && [ $exit_code -eq 0 ]; then
                echo -e "${GREEN}PASSED ($test_passed/$test_total)${NC}"
            else
                echo -e "${RED}FAILED ($test_passed/$test_total)${NC}"
            fi
            
            # Save to result file
            echo "### $test_exe" >> $result_file
            echo "Passed: $test_passed/$test_total" >> $result_file
            echo "Exit code: $exit_code" >> $result_file
            echo "" >> $result_file
            cat temp_output.txt >> $result_file
            echo "" >> $result_file
            echo "----------------------------------------" >> $result_file
            echo "" >> $result_file
        fi
    done
    
    rm -f temp_output.txt
    
    # Summary
    echo ""
    echo "Summary for $impl_name:"
    echo "  Total tests: $total_tests"
    echo "  Passed: $passed_tests"
    echo "  Failed: $((total_tests - passed_tests))"
    if [ $total_tests -gt 0 ]; then
        local pass_rate=$((passed_tests * 100 / total_tests))
        echo "  Pass rate: ${pass_rate}%"
        
        echo "" >> $result_file
        echo "SUMMARY" >> $result_file
        echo "=======" >> $result_file
        echo "Total tests: $total_tests" >> $result_file
        echo "Passed: $passed_tests" >> $result_file
        echo "Failed: $((total_tests - passed_tests))" >> $result_file
        echo "Pass rate: ${pass_rate}%" >> $result_file
    fi
    echo ""
    
    # Cleanup temporary wrapper
    rm -f mb8877_${impl_name}_wrapper.cpp
    
    return 0
}

# Change to test directory
cd "$(dirname "$0")"

# Test mb8877_compat.cpp
echo "Phase 1: Testing MB8877 Compatibility Layer"
run_tests "compat" "../src/vm/mb8877_compat.cpp" "test_results/compat_results_${TIMESTAMP}.txt"
compat_result=$?

# Test original mb8877.cpp
echo "Phase 2: Testing Original MB8877"
run_tests "original" "../src/vm/mb8877.cpp" "test_results/original_results_${TIMESTAMP}.txt"
original_result=$?

# Restore original test wrapper
cat > mb8877_test_wrapper.cpp << 'EOF'
/*
	MB8877 Test Wrapper
	
	This file provides the proper include order for testing MB8877 implementation
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment first to provide all required types
#include "mock_environment.h"

// Include the header (not the .cpp) to get class definition
#include "../src/vm/mb8877_compat.h"

// Now include the implementation
#include "../src/vm/mb8877_compat.cpp"
EOF

# Generate comparison report
echo ""
echo "=========================================="
echo "Generating Comparison Report"
echo "=========================================="

cat > test_results/comparison_report_${TIMESTAMP}.txt << EOF
MB8877 Implementation Comparison Report
Generated: $(date)

Test Suite Comparison Results
=============================

Implementation Files:
- Original: src/vm/mb8877.cpp
- Compat:   src/vm/mb8877_compat.cpp

Individual Test Results:
- Compat results:   test_results/compat_results_${TIMESTAMP}.txt
- Original results: test_results/original_results_${TIMESTAMP}.txt

Build Logs:
- Compat build:   build_compat.log  
- Original build: build_original.log

Notes:
- Tests are designed to verify API compatibility
- Implementation differences may cause behavioral variations
- Both implementations should provide similar functionality
EOF

echo ""
echo "Test comparison complete!"
echo "Results saved to: test_results/"
echo "  - Comparison report: comparison_report_${TIMESTAMP}.txt"
echo "  - Compat results:    compat_results_${TIMESTAMP}.txt"
echo "  - Original results:  original_results_${TIMESTAMP}.txt"
echo ""

# Exit with error if either test failed
if [ $compat_result -ne 0 ] || [ $original_result -ne 0 ]; then
    exit 1
fi

exit 0