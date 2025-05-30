#!/bin/bash

# 最新のログファイルを表示するスクリプト

# xcodeディレクトリに移動
cd /Volumes/PoppoSSD2T/Projects/EmulatorProjects/CommonSourceCodeProject/xcode

# 最新のログファイルを探す
LATEST_LOG=$(ls -t build_test_*.log 2>/dev/null | head -1)

if [ -z "$LATEST_LOG" ]; then
    echo "ログファイルが見つかりません"
    exit 1
fi

echo "=== 最新のログファイル: $LATEST_LOG ==="
echo ""

# エラーがあるかチェック
if grep -q "\[ERROR\]" "$LATEST_LOG"; then
    echo "=== エラーが検出されました ==="
    grep -A 5 -B 5 "\[ERROR\]" "$LATEST_LOG"
    echo ""
fi

# ビルドエラーを抽出
if grep -q "error:" "$LATEST_LOG"; then
    echo "=== ビルドエラー ==="
    grep -A 3 -B 3 "error:" "$LATEST_LOG" | head -50
    echo ""
fi

# 最後の50行を表示
echo "=== ログの最後の50行 ==="
tail -n 50 "$LATEST_LOG"