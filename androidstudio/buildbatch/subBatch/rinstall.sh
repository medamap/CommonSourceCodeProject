#!/bin/bash

# Windows batch file converted to macOS shell script
# Original: rinstall.bat

# Get variant parameter and convert to uppercase
variant=$1
variant_uppercase=$(echo "$variant" | tr '[:lower:]' '[:upper:]')

# Create complete build type with Release suffix
buildType="${variant_uppercase}Release"

# Move to parent directory
cd ..

# Install only
./gradlew install${buildType}

cd buildbatch