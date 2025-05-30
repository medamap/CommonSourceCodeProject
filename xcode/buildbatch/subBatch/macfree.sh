#!/bin/bash

# macOS Free (unsigned) build script

set -e  # Exit on error

# Get model name
MODEL_NAME=$1
if [ -z "$MODEL_NAME" ]; then
    echo "Error: Model name required"
    exit 1
fi

# Convert model name to uppercase for display
MODEL_DISPLAY=$(echo "$MODEL_NAME" | tr '[:lower:]' '[:upper:]')

# Build configuration
BUILD_TYPE="Release"
PLATFORM="macOS"

# Save current directory
BUILDBATCH_DIR=$(pwd)

# Move to xcode directory (parent of buildbatch)
cd ..

# Use existing build script
echo "Building $MODEL_DISPLAY using existing build system..."
./build_machine.sh "$MODEL_NAME"

# Return to buildbatch directory  
cd "$BUILDBATCH_DIR"

# Create output directory with date
DATESTR=$(date +"%Y%m%d")
OUTPUT_DIR="v${DATESTR}_free_mac"
mkdir -p "$OUTPUT_DIR"

# Find and copy the executable
EXEC_NAME="$MODEL_NAME"
BUILD_DIR="../build_${MODEL_NAME}_debug"

if [ -f "$BUILD_DIR/bin/$EXEC_NAME" ]; then
    echo "Copying $EXEC_NAME to $OUTPUT_DIR/"
    cp "$BUILD_DIR/bin/$EXEC_NAME" "$OUTPUT_DIR/"
    chmod +x "$OUTPUT_DIR/$EXEC_NAME"
    
    # Add a note about unsigned app
    echo "Note: This executable is unsigned. Users may see a security warning."
    echo "To run: cd $OUTPUT_DIR && ./$EXEC_NAME"
    echo "Or: Right-click and select 'Open' in Finder"
else
    echo "Error: Executable not found at $BUILD_DIR/bin/$EXEC_NAME"
    ls -la "$BUILD_DIR/bin/" 2>/dev/null || echo "Build directory not found"
    exit 1
fi

echo "Build completed successfully!"