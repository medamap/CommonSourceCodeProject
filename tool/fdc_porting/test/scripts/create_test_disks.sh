#!/bin/bash
# Create test disk images using Legacy89DiskKit

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LEGACY89_PATH="$SCRIPT_DIR/../../tools/Legacy89DiskKit/CSharp"
TEST_DISK_DIR="$SCRIPT_DIR/../data/test_disk_images"

# Create test disk directory
mkdir -p "$TEST_DISK_DIR"

echo "Creating test disk images..."

# Check if Legacy89DiskKit CLI is available
if [ ! -f "$LEGACY89_PATH/Legacy89DiskKit.CLI/Legacy89DiskKit.CLI.csproj" ]; then
    echo "Error: Legacy89DiskKit.CLI not found at $LEGACY89_PATH"
    echo "Please ensure Legacy89DiskKit is properly installed."
    exit 1
fi

# 1. Basic 2D disk (Sharp X1 format)
echo "Creating basic 2D disk..."
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    create "$TEST_DISK_DIR/test_basic_2d.d88" 2D "TEST BASIC"

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    format "$TEST_DISK_DIR/test_basic_2d.d88" --filesystem hu-basic

# 2. Data disk with test files
echo "Creating data disk..."
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    create "$TEST_DISK_DIR/test_data_2d.d88" 2D "TEST DATA"

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    format "$TEST_DISK_DIR/test_data_2d.d88" --filesystem hu-basic

# Create test files
echo "Hello, MB8877 FDC Test!" > /tmp/test_hello.txt
echo "This is sector data for READ/WRITE testing." > /tmp/test_sector.txt

# Add files to disk
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    import-text "$TEST_DISK_DIR/test_data_2d.d88" /tmp/test_hello.txt HELLO.TXT \
    --filesystem hu-basic --machine x1

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    import-text "$TEST_DISK_DIR/test_data_2d.d88" /tmp/test_sector.txt SECTOR.TXT \
    --filesystem hu-basic --machine x1

# 3. Write-protected disk
echo "Creating write-protected disk..."
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    create "$TEST_DISK_DIR/test_protected_2d.d88" 2D "PROTECTED"

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    format "$TEST_DISK_DIR/test_protected_2d.d88" --filesystem hu-basic

# Set write protection
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    set-write-protect "$TEST_DISK_DIR/test_protected_2d.d88" true

# 4. Empty disk for write tests
echo "Creating empty disk for write tests..."
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    create "$TEST_DISK_DIR/test_empty_2d.d88" 2D "EMPTY"

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    format "$TEST_DISK_DIR/test_empty_2d.d88" --filesystem hu-basic

# 5. 2DD disk (MSX format)
echo "Creating 2DD disk..."
dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    create "$TEST_DISK_DIR/test_msx_2dd.d88" 2DD "MSX TEST"

dotnet run --project "$LEGACY89_PATH/Legacy89DiskKit.CLI" -- \
    format "$TEST_DISK_DIR/test_msx_2dd.d88" --filesystem msx-dos

echo "Test disk creation completed!"
echo "Created files:"
ls -la "$TEST_DISK_DIR"/*.d88