#!/bin/bash

# Windows batch file converted to macOS shell script
# Original: allexecute.bat

csvFile="models/models.csv"
buildType="subBatch/rbuild.sh"
hasInputFile=false
hasBuildType=false
directModelName=""

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
                ReleaseBuild)
                    buildType="subBatch/rbuild.sh"
                    ;;
                ReleaseInstall)
                    buildType="subBatch/rinstall.sh"
                    ;;
                ReleaseBuildExecute)
                    buildType="subBatch/rbuildexec.sh"
                    ;;
                DebugBuild)
                    buildType="subBatch/dbuild.sh"
                    ;;
                DebugInstall)
                    buildType="subBatch/dinstall.sh"
                    ;;
                DebugBuildExecute)
                    buildType="subBatch/dbuildexec.sh"
                    ;;
                UnInstall)
                    buildType="subBatch/uninstall.sh"
                    ;;
                *)
                    echo "エラー: 指定されたモード「$2」に対応するバッチファイルがありません。"
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
        *)
            shift
            ;;
    esac
done

# Show help if required arguments are missing
if [[ "$hasInputFile" != "true" ]] || [[ "$hasBuildType" != "true" ]]; then
    echo "このバッチファイルは以下の引数を取ります:"
    echo "  -i csvFile     モデル情報が記載されたCSVファイルのパス (デフォルト: models/models.csv)"
    echo "  -m buildType   実行するビルドの種類 (デフォルト: ReleaseBuild)"
    echo "                 ReleaseBuild, ReleaseInstall, ReleaseBuildExecute,"
    echo "                 DebugBuild, DebugInstall, DebugBuildExecute, UnInstall"
    echo "  -d modelName   ダイレクトにビルドバッチに渡す機種名"
    exit 0
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