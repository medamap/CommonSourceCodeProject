#!/bin/bash

# iOS Free (personal team) build script

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
fi

# Use override team ID if provided, otherwise use config
TEAM_ID=${OVERRIDE_TEAM_ID:-$FREE_TEAM_ID}

if [ -z "$TEAM_ID" ] || [ "$TEAM_ID" == "XXXXXXXXXX" ]; then
    echo "Error: Team ID not configured"
    echo "Please set FREE_TEAM_ID in signing/config.sh or use -t option"
    exit 1
fi

# Convert model name to uppercase
MODEL_DISPLAY=$(echo "$MODEL_NAME" | tr '[:lower:]' '[:upper:]')

# Build configuration
BUILD_TYPE="Release"
PLATFORM="iOS"
BUNDLE_ID="${FREE_BUNDLE_PREFIX}.${MODEL_NAME}"

# Move to xcode directory
cd ..

# Create Xcode project if not exists
PROJECT_DIR="ios_${MODEL_NAME}"
if [ ! -d "$PROJECT_DIR" ]; then
    echo "Creating iOS project for $MODEL_DISPLAY..."
    mkdir -p "$PROJECT_DIR"
    
    # TODO: Generate iOS-specific CMakeLists.txt or Xcode project
    echo "Error: iOS project generation not yet implemented"
    echo "Please create an Xcode project manually for now"
    exit 1
fi

cd "$PROJECT_DIR"

# Build using xcodebuild
echo "Building $MODEL_DISPLAY for iOS (personal team)..."
xcodebuild -project "${MODEL_DISPLAY}.xcodeproj" \
    -scheme "$MODEL_DISPLAY" \
    -configuration $BUILD_TYPE \
    -destination "generic/platform=iOS" \
    -archivePath "build/${MODEL_DISPLAY}.xcarchive" \
    CODE_SIGN_STYLE="Automatic" \
    DEVELOPMENT_TEAM="$TEAM_ID" \
    PRODUCT_BUNDLE_IDENTIFIER="$BUNDLE_ID" \
    archive

# Export for development
echo "Exporting for development..."

# Create export options plist
cat > "ExportOptions.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>development</string>
    <key>teamID</key>
    <string>$TEAM_ID</string>
    <key>compileBitcode</key>
    <false/>
    <key>thinning</key>
    <string>&lt;none&gt;</string>
</dict>
</plist>
EOF

xcodebuild -exportArchive \
    -archivePath "build/${MODEL_DISPLAY}.xcarchive" \
    -exportPath "build/" \
    -exportOptionsPlist "ExportOptions.plist"

# Clean up
rm "ExportOptions.plist"

# Return to buildbatch directory
cd ../buildbatch

# Create output directory
DATESTR=$(date +"%Y%m%d")
OUTPUT_DIR="v${DATESTR}_free_ios"
mkdir -p "$OUTPUT_DIR"

# Copy IPA
IPA_FILE="../ios_${MODEL_NAME}/build/${MODEL_DISPLAY}.ipa"
if [ -f "$IPA_FILE" ]; then
    echo "Copying IPA to $OUTPUT_DIR/"
    cp "$IPA_FILE" "$OUTPUT_DIR/"
    
    echo "Note: This build is signed with a personal team (7-day expiration)"
    echo "Install using Xcode or Apple Configurator"
else
    # For development, the .app might be more useful
    APP_DIR="../ios_${MODEL_NAME}/build/${MODEL_DISPLAY}.app"
    if [ -d "$APP_DIR" ]; then
        echo "Copying .app to $OUTPUT_DIR/"
        cp -R "$APP_DIR" "$OUTPUT_DIR/"
        echo "Note: Install this .app using Xcode"
    else
        echo "Error: Build output not found"
        exit 1
    fi
fi

echo "iOS build completed successfully!"