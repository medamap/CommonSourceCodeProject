#!/bin/bash

# Windows batch file converted to macOS shell script
# Original: dbuildexec.bat

# Get variant parameter and convert to uppercase
variant=$1
variant_uppercase=$(echo "$variant" | tr '[:lower:]' '[:upper:]')

# Create complete build type with Debug suffix
buildType="${variant_uppercase}Debug"

# Move to parent directory
cd ..

# Assemble, install and execute
./gradlew assemble${buildType}
if [ $? -ne 0 ]; then
    exit 1
fi

./gradlew install${buildType}
if [ $? -ne 0 ]; then
    exit 1
fi

# Execute the app (package name may need adjustment)
# Default package name pattern: com.github.takeda.cscp.${variant}
adb shell am start -n "com.github.takeda.cscp.${variant}/.MainActivity"

cd buildbatch