#!/bin/bash

# Windows batch file converted to macOS shell script
# Original: rbuild.bat

# Get variant parameter and convert to uppercase
variant=$1
variant_uppercase=$(echo "$variant" | tr '[:lower:]' '[:upper:]')

# Create complete build type with Release suffix
buildType="${variant_uppercase}Release"

# Move to parent directory
cd ..

# Assemble and install
./gradlew assemble${buildType}
if [ $? -ne 0 ]; then
    exit 1
fi

./gradlew install${buildType}

# Return to buildbatch directory and create date folder
cd buildbatch
DATESTR=$(date +"%Y%m%d")
FOLDERNAME="v${DATESTR}_release_apk"

if [ ! -d "$FOLDERNAME" ]; then
    mkdir "$FOLDERNAME"
fi

# Copy APK files to new folder
# Path may need adjustment based on project structure
cp ../app/build/outputs/apk/${variant}/release/*.apk "$FOLDERNAME/" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "APK copy failed"
    exit 1
fi

echo "Build completed successfully"