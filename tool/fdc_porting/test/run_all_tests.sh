#!/bin/bash
# Auto-generated script to run all MB8877 tests

echo "Running MB8877 Compatibility Test Suite"
echo "========================================"

TESTS_PASSED=0
TESTS_FAILED=0

echo "Running test_mb8877_registers..."
if ./test_mb8877_registers; then
    echo "  ✓ test_mb8877_registers PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_registers FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_type1_commands..."
if ./test_mb8877_type1_commands; then
    echo "  ✓ test_mb8877_type1_commands PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_type1_commands FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_type2_commands..."
if ./test_mb8877_type2_commands; then
    echo "  ✓ test_mb8877_type2_commands PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_type2_commands FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_type3_commands..."
if ./test_mb8877_type3_commands; then
    echo "  ✓ test_mb8877_type3_commands PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_type3_commands FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_type4_commands..."
if ./test_mb8877_type4_commands; then
    echo "  ✓ test_mb8877_type4_commands PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_type4_commands FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_error_handling..."
if ./test_mb8877_error_handling; then
    echo "  ✓ test_mb8877_error_handling PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_error_handling FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_timing..."
if ./test_mb8877_timing; then
    echo "  ✓ test_mb8877_timing PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_timing FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_write_track..."
if ./test_mb8877_write_track; then
    echo "  ✓ test_mb8877_write_track PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_write_track FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_drive_mfm..."
if ./test_mb8877_drive_mfm; then
    echo "  ✓ test_mb8877_drive_mfm PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_drive_mfm FAILED"
    ((TESTS_FAILED++))
fi

echo "Running test_mb8877_drive_rpm..."
if ./test_mb8877_drive_rpm; then
    echo "  ✓ test_mb8877_drive_rpm PASSED"
    ((TESTS_PASSED++))
else
    echo "  ✗ test_mb8877_drive_rpm FAILED"
    ((TESTS_FAILED++))
fi

echo "========================================"
echo "Total: $((TESTS_PASSED + TESTS_FAILED)) tests"
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"

if [ $TESTS_FAILED -eq 0 ]; then
    echo "All tests PASSED!"
    exit 0
else
    echo "Some tests FAILED!"
    exit 1
fi
