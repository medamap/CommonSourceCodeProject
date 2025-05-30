#!/bin/bash

# macOS Developer (signed) build script

set -e  # Exit on error

# Get model name
MODEL_NAME=$1
if [ -z "$MODEL_NAME" ]; then
    echo "Error: Model name required"
    exit 1
fi

# Load configuration
CONFIG_FILE="signing/config.sh"
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
else
    echo "Error: $CONFIG_FILE not found"
    exit 1
fi

# Check configuration
if ! check_config "developer"; then
    exit 1
fi

# Convert model name to uppercase for display
MODEL_DISPLAY=$(echo "$MODEL_NAME" | tr '[:lower:]' '[:upper:]')

# Build configuration
BUILD_TYPE="Release"
PLATFORM="macOS"
BUNDLE_ID="${DEV_BUNDLE_PREFIX}.${MODEL_NAME}"

# Move to xcode directory
cd ..

# Create build directory
BUILD_DIR="build_${MODEL_NAME}_mac_signed"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring $MODEL_DISPLAY for macOS (signed)..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DMACHINE_NAME="$MODEL_NAME" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15" \
    -DCODE_SIGN_IDENTITY="$DEV_CERT_MAC" \
    -DDEVELOPMENT_TEAM="$DEV_TEAM_ID" \
    -DBUNDLE_IDENTIFIER="$BUNDLE_ID"

# Build
echo "Building $MODEL_DISPLAY..."
cmake --build . --config $BUILD_TYPE -j$(sysctl -n hw.ncpu)

# Sign the app
APP_NAME="${MODEL_DISPLAY}.app"
if [ -d "$APP_NAME" ]; then
    echo "Signing $APP_NAME with Developer ID..."
    codesign --force --deep --sign "$DEV_CERT_MAC" \
        --options runtime \
        --entitlements ../buildbatch/signing/developer/entitlements.plist \
        "$APP_NAME"
    
    # Verify signature
    echo "Verifying signature..."
    codesign --verify --deep --strict "$APP_NAME"
    spctl -a -t exec -vv "$APP_NAME"
else
    echo "Error: App bundle not found"
    exit 1
fi

# Return to buildbatch directory
cd ../buildbatch

# Create output directory
DATESTR=$(date +"%Y%m%d")
OUTPUT_DIR="v${DATESTR}_dev_mac"
mkdir -p "$OUTPUT_DIR"

# Copy signed app
echo "Copying signed app to $OUTPUT_DIR/"
cp -R "../build_${MODEL_NAME}_mac_signed/$APP_NAME" "$OUTPUT_DIR/"

# Create DMG (optional)
echo "Creating DMG..."
DMG_NAME="${MODEL_DISPLAY}-${DATESTR}.dmg"
hdiutil create -volname "$MODEL_DISPLAY" \
    -srcfolder "$OUTPUT_DIR/$APP_NAME" \
    -ov -format UDZO \
    "$OUTPUT_DIR/$DMG_NAME"

# Sign DMG
codesign --force --sign "$DEV_CERT_MAC" "$OUTPUT_DIR/$DMG_NAME"

# Notarization (optional, requires additional setup)
if [ -n "$NOTARIZATION_PASSWORD" ]; then
    echo "Submitting for notarization..."
    xcrun altool --notarize-app \
        --primary-bundle-id "$BUNDLE_ID" \
        --username "$DEV_APPLE_ID" \
        --password "$NOTARIZATION_PASSWORD" \
        --file "$OUTPUT_DIR/$DMG_NAME"
    echo "Note: Check notarization status with: xcrun altool --notarization-history 0 -u $DEV_APPLE_ID"
fi

echo "Build and signing completed successfully!"