#!/bin/bash
# Phase 22.2: Comprehensive disk access refactoring script

echo "Starting comprehensive disk access refactoring..."

# Backup original files
cp ../../../src/vm/mb8877_compat.cpp ../../../src/vm/mb8877_compat.cpp.backup
cp ../../../src/vm/mb8877_compat.h ../../../src/vm/mb8877_compat.h.backup

# Create temporary working file
cp ../../../src/vm/mb8877_compat.cpp mb8877_compat_temp.cpp

# Count original disk accesses
echo "Original disk access count:"
grep -c "disk\[drvreg\]->" mb8877_compat_temp.cpp

# Step 1: Replace simple disk[drvreg]-> accesses with safety checks
echo "Step 1: Replacing simple disk[drvreg]-> accesses..."

# Create a sed script for replacements
cat > disk_refactor.sed << 'EOF'
# Pattern 1: disk[drvreg]->method() at start of line
s/^(\s*)disk\[drvreg\]->(\w+)\(/\1DISK* disk_safe = get_disk_safe(drvreg);\n\1if(disk_safe) disk_safe->\2(/g

# Pattern 2: if(disk[drvreg]->
s/if\(disk\[drvreg\]->/DISK* disk_safe = get_disk_safe(drvreg);\n\tif(disk_safe \&\& disk_safe->/g

# Pattern 3: disk[drvreg]->property in expressions
s/disk\[drvreg\]->(\w+)([^(])/get_disk_safe(drvreg) ? get_disk_safe(drvreg)->\1\2 : 0/g
EOF

# Apply sed script (macOS compatible)
# Note: This is a simplified approach - we'll need manual fixes

echo "Due to complexity, manual refactoring is required..."
echo "Creating refactored version with safety checks..."

# Let's create a proper refactored version manually