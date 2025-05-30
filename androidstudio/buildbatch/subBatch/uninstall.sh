#!/bin/bash

# Windows batch file converted to macOS shell script
# Original: uninstall.bat

# Get variant parameter
variant=$1

# Uninstall the app
# Default package name pattern: com.github.takeda.cscp.${variant}
adb uninstall "com.github.takeda.cscp.${variant}"

if [ $? -eq 0 ]; then
    echo "Uninstall completed successfully"
else
    echo "Uninstall failed or app not found"
fi