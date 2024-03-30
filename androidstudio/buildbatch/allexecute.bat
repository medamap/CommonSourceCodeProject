@echo off

setlocal enabledelayedexpansion

set "csvFile=models\\models.csv"
set "buildType=subBatch\\rbuild.bat"
set "hasInputFile=false"
set "hasBuildType=false"

:parseArgs
if "%~1"=="" goto :executeOrShowHelp

if "%~1"=="-i" (
    set "csvFile=%~2"
    set "hasInputFile=true"
    shift & shift
    goto :parseArgs
)

if "%~1"=="-m" (
    set "buildType="
    if "%~2"=="ReleaseBuild" set "buildType=subBatch\\rbuild.bat"
    if "%~2"=="ReleaseInstall" set "buildType=subBatch\\rinstall.bat"
    if "%~2"=="ReleaseBuildExecute" set "buildType=subBatch\\rbuildexec.bat"
    if "%~2"=="DebugBuild" set "buildType=subBatch\\dbuild.bat"
    if "%~2"=="DebugInstall" set "buildType=subBatch\\dinstall.bat"
    if "%~2"=="DebugBuildExecute" set "buildType=subBatch\\dbuildexec.bat"
    if "%~2"=="UnInstall" set "buildType=subBatch\\uninstall.bat"
    if "!buildType!"=="" (
        echo 警告: 指定されたモード「%~2」に対応するバッチファイルがありません。
        exit /b 1
    )
    set "hasBuildType=true"
    shift & shift
    goto :parseArgs
)

shift
goto :parseArgs

:executeOrShowHelp
if %hasInputFile%==true if %hasBuildType%==true goto :execute
goto :showHelp

:showHelp
echo このバッチファイルは以下の引数を取ります:
echo   -i csvFile     モデル情報が記載されたCSVファイルのパス (デフォルト: models\models.csv)
echo   -m buildType   実行するビルドの種類 (デフォルト: ReleaseBuild)
echo                  ReleaseBuild, ReleaseInstall, ReleaseBuildExecute,
echo                  DebugBuild, DebugInstall, DebugBuildExecute, UnInstall
exit /b 0

:execute
if not exist "%csvFile%" (
    echo 指定されたファイルが見つかりません: %csvFile%
    exit /b 1
)

for /f "tokens=1,2 delims=," %%a in (%csvFile%) do (
    echo ***********  %%a [!buildType!] ***********
    call !buildType! %%b
)

echo ビルドプロセスが完了しました。

endlocal