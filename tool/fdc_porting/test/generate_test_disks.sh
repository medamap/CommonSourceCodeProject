#!/bin/bash

# Test disk generation script for MB8877 FDC testing
# Using Legacy89DiskKit to create various D88 format test disks

LEGACY89_CLI="../tools/Legacy89DiskKit/CSharp/Legacy89DiskKit.CLI/bin/Release/net8.0/Legacy89DiskKit.CLI.dll"
TEST_DISK_DIR="test_disks"

# Create test disk directory if it doesn't exist
mkdir -p "$TEST_DISK_DIR"

echo "=== Generating Test Disk Images ==="

# Function to create and format a disk
create_disk() {
    local filename=$1
    local format=$2
    local filesystem=$3
    local label=$4
    
    echo "Creating $filename ($format, $filesystem)..."
    dotnet "$LEGACY89_CLI" create "$TEST_DISK_DIR/$filename" "$format" "$label"
    dotnet "$LEGACY89_CLI" format "$TEST_DISK_DIR/$filename" "$filesystem"
}

# 1. Basic 2D empty formatted disks
create_disk "test_2d_hubasic_empty.d88" "2D" "hu-basic" "HU-BASIC 2D Empty"
create_disk "test_2d_n88basic_empty.d88" "2D" "n88-basic" "N88-BASIC 2D Empty"

# 2. 2DD disks with different filesystems
create_disk "test_2dd_hubasic_empty.d88" "2DD" "hu-basic" "HU-BASIC 2DD Empty"
create_disk "test_2dd_msxdos_empty.d88" "2DD" "msx-dos" "MSX-DOS 2DD Empty"
create_disk "test_2dd_fat12_empty.d88" "2DD" "fat12" "FAT12 2DD Empty"

# 3. 2HD disk
create_disk "test_2hd_hubasic_empty.d88" "2HD" "hu-basic" "HU-BASIC 2HD Empty"

# 4. Create test files to import
echo "Creating test files..."
echo "This is a test text file for MB8877 FDC testing." > "$TEST_DISK_DIR/test_content.txt"
echo "Line 1: Test data" >> "$TEST_DISK_DIR/test_content.txt"
echo "Line 2: More test data" >> "$TEST_DISK_DIR/test_content.txt"
echo "Line 3: Final test data" >> "$TEST_DISK_DIR/test_content.txt"

# Create a binary test file (256 bytes of pattern data)
python3 -c "
import struct
with open('$TEST_DISK_DIR/test_binary.bin', 'wb') as f:
    for i in range(256):
        f.write(struct.pack('B', i))
"

# 5. Import files to some disks
echo "Importing test files to disks..."

# Import text file to 2D HU-BASIC disk
dotnet "$LEGACY89_CLI" import-text "$TEST_DISK_DIR/test_2d_hubasic_empty.d88" \
    "$TEST_DISK_DIR/test_content.txt" "TEST.TXT" --filesystem hu-basic --machine x1

# Import binary file to 2DD HU-BASIC disk
dotnet "$LEGACY89_CLI" import-binary "$TEST_DISK_DIR/test_2dd_hubasic_empty.d88" \
    "$TEST_DISK_DIR/test_binary.bin" "TEST.BIN" 8000 8000

# Create a disk with multiple files
create_disk "test_2d_hubasic_multi.d88" "2D" "hu-basic" "HU-BASIC Multi Files"

# Create multiple test files
for i in {1..5}; do
    echo "Test file $i content" > "$TEST_DISK_DIR/file$i.txt"
    dotnet "$LEGACY89_CLI" import-text "$TEST_DISK_DIR/test_2d_hubasic_multi.d88" \
        "$TEST_DISK_DIR/file$i.txt" "FILE$i.TXT" --filesystem hu-basic --machine x1
done

# 6. Create a disk with specific sector patterns for testing
echo "Creating disk with test patterns..."
create_disk "test_2d_patterns.d88" "2D" "hu-basic" "Test Patterns"

# Create pattern files
# Pattern 1: All 0xFF
python3 -c "with open('$TEST_DISK_DIR/pattern_ff.bin', 'wb') as f: f.write(b'\\xFF' * 512)"
# Pattern 2: All 0x00
python3 -c "with open('$TEST_DISK_DIR/pattern_00.bin', 'wb') as f: f.write(b'\\x00' * 512)"
# Pattern 3: Alternating 0x55 0xAA
python3 -c "with open('$TEST_DISK_DIR/pattern_55aa.bin', 'wb') as f: f.write(b'\\x55\\xAA' * 256)"

dotnet "$LEGACY89_CLI" import-binary "$TEST_DISK_DIR/test_2d_patterns.d88" \
    "$TEST_DISK_DIR/pattern_ff.bin" "PATFF.BIN" 8000 8000
dotnet "$LEGACY89_CLI" import-binary "$TEST_DISK_DIR/test_2d_patterns.d88" \
    "$TEST_DISK_DIR/pattern_00.bin" "PAT00.BIN" 8000 8000
dotnet "$LEGACY89_CLI" import-binary "$TEST_DISK_DIR/test_2d_patterns.d88" \
    "$TEST_DISK_DIR/pattern_55aa.bin" "PAT55AA.BIN" 8000 8000

# 7. List contents of generated disks
echo -e "\\n=== Disk Contents ==="
for disk in "$TEST_DISK_DIR"/*.d88; do
    echo -e "\\nContents of $(basename "$disk"):"
    dotnet "$LEGACY89_CLI" list "$disk"
done

# 8. Show disk information
echo -e "\\n=== Disk Information ==="
for disk in "$TEST_DISK_DIR"/*.d88; do
    echo -e "\\nInfo for $(basename "$disk"):"
    dotnet "$LEGACY89_CLI" info "$disk"
done

# Clean up temporary files
rm -f "$TEST_DISK_DIR"/*.txt "$TEST_DISK_DIR"/*.bin

echo -e "\\n=== Test disk generation complete ==="
echo "Generated disks in: $TEST_DISK_DIR/"
ls -la "$TEST_DISK_DIR"/*.d88