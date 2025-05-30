#!/bin/bash

# CSCP macOS GUI版ビルドスクリプト
# Usage: ./build_machine_gui.sh -m <machine_name>

MACHINE="x1turbo"  # デフォルト機種

while getopts "m:" opt; do
  case $opt in
    m)
      MACHINE="$OPTARG"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

echo "Building GUI version for machine: $MACHINE"

# ビルドディレクトリ作成
BUILD_DIR="build_${MACHINE}_gui"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake実行（GUI版用の定義を追加）
cmake .. -DMACHINE=$MACHINE -DCMAKE_BUILD_TYPE=Release -DUSE_GUI=ON

# ビルド実行
cmake --build . --config Release

# 実行ファイルの確認
if [ -f "cscp_exec" ]; then
    echo "Build successful!"
    echo "Run with: ./$BUILD_DIR/cscp_exec -machine $MACHINE -fdd0 \"path/to/disk.d88\""
else
    echo "Build failed!"
    exit 1
fi