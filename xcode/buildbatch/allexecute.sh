#!/bin/bash

# Xcode batch build script for macOS/iOS/iPadOS
# Supports both free (personal team) and developer program signing

set -e  # Exit on error

# Default values
csvFile="models/models.csv"
buildType="subBatch/macfree.sh"
hasInputFile=false
hasBuildType=false
directModelName=""
teamId=""
configFile="signing/config.sh"

# Load configuration if exists
if [ -f "$configFile" ]; then
    source "$configFile"
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -i)
            csvFile="$2"
            hasInputFile=true
            shift 2
            ;;
        -m)
            buildType=""
            case $2 in
                # macOS builds
                MacFree)
                    buildType="subBatch/macfree.sh"
                    ;;
                MacDeveloper)
                    buildType="subBatch/macdeveloper.sh"
                    ;;
                # iOS builds
                iOSFree)
                    buildType="subBatch/iosfree.sh"
                    ;;
                iOSDeveloper)
                    buildType="subBatch/iosdeveloper.sh"
                    ;;
                # iPad builds (same as iOS)
                iPadFree)
                    buildType="subBatch/iosfree.sh"
                    ;;
                iPadDeveloper)
                    buildType="subBatch/iosdeveloper.sh"
                    ;;
                # Clean
                Clean)
                    buildType="subBatch/clean.sh"
                    ;;
                *)
                    echo "エラー: 指定されたモード「$2」に対応するビルドスクリプトがありません。"
                    exit 1
                    ;;
            esac
            hasBuildType=true
            shift 2
            ;;
        -d)
            directModelName="$2"
            hasInputFile=true
            shift 2
            ;;
        -t)
            teamId="$2"
            shift 2
            ;;
        *)
            shift
            ;;
    esac
done

# Export team ID for sub scripts
export OVERRIDE_TEAM_ID="$teamId"

# Show help if required arguments are missing
if [[ "$hasInputFile" != "true" ]] || [[ "$hasBuildType" != "true" ]]; then
    echo "Xcode バッチビルドスクリプト"
    echo ""
    echo "使用方法:"
    echo "  $0 -i csvFile -m buildType [-t teamId]"
    echo ""
    echo "オプション:"
    echo "  -i csvFile     モデル情報が記載されたCSVファイルのパス (デフォルト: models/models.csv)"
    echo "  -m buildType   実行するビルドの種類:"
    echo "                 [macOS]"
    echo "                   MacFree       - 署名なしビルド"
    echo "                   MacDeveloper  - Developer ID署名付きビルド"
    echo "                 [iOS/iPadOS]"
    echo "                   iOSFree       - 個人署名（7日間）"
    echo "                   iOSDeveloper  - 配布用署名"
    echo "                   iPadFree      - 個人署名（7日間）"
    echo "                   iPadDeveloper - 配布用署名"
    echo "                 [その他]"
    echo "                   Clean         - ビルドクリーン"
    echo "  -d modelName   直接指定する機種名"
    echo "  -t teamId      Team IDを上書き指定（オプション）"
    echo ""
    echo "例:"
    echo "  $0 -i models/x1.csv -m MacFree"
    echo "  $0 -d x1turbo -m iOSDeveloper"
    exit 0
fi

# Check if signing configuration exists for developer builds
if [[ "$buildType" == *"developer"* ]]; then
    if [ ! -f "$configFile" ]; then
        echo "エラー: Developer版ビルドには signing/config.sh が必要です。"
        echo "signing/config.sh.template をコピーして設定してください。"
        exit 1
    fi
fi

# Execute build
if [[ -n "$directModelName" ]]; then
    echo "*********** Direct Model [$directModelName] [$buildType] ***********"
    bash "$buildType" "$directModelName"
else
    if [[ ! -f "$csvFile" ]]; then
        echo "指定されたファイルが見つかりません: $csvFile"
        exit 1
    fi
    
    while IFS=',' read -r displayName modelName; do
        # Skip empty lines and comments
        [[ -z "$displayName" ]] && continue
        [[ "$displayName" =~ ^# ]] && continue
        
        echo "***********  $displayName [$buildType] ***********"
        bash "$buildType" "$modelName"
    done < "$csvFile"
fi

echo "ビルドプロセスが完了しました。"