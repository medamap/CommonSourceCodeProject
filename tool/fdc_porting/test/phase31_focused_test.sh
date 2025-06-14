#!/bin/bash

echo "=== Phase 31: Focused READ Test ==="
echo "Date: $(date)"
echo

# Clean build
echo "1. Clean building Type II test..."
make clean > /dev/null 2>&1
make test_mb8877_type2_commands 2>&1 | grep -E "(error:|warning:|$)" | head -5

if [ ! -f test_mb8877_type2_commands ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo
echo "2. Running Type II tests..."
./test_mb8877_type2_commands 2>&1 | grep -E "(READ|WRITE|Test Results:|passed:|failed:|Success rate:)" | head -20

echo
echo "3. Detailed READ test results:"
./test_mb8877_type2_commands 2>&1 | grep -A5 -B5 "READ SECTOR" | head -30

echo
echo "4. Check for buffer-related messages:"
./test_mb8877_type2_commands 2>&1 | grep -i "buffer" | head -10

echo
echo "5. Overall success rate:"
./test_mb8877_type2_commands 2>&1 | grep "Success rate:" | tail -1

echo
echo "=== End of Phase 31 Test ==="