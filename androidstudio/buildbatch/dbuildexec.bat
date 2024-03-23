@echo off
setlocal enabledelayedexpansion

:: 引数を取得し、大文字に変換
set variant=%1
set variant_uppercase=%variant%
for %%i in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
    call set variant_uppercase=%%variant_uppercase:%%i=%%i%%
)

:: Debugを付けて完全なビルドタイプを作成
set buildType=!variant_uppercase!Debug

cd ..

:: アセンブルとインストール
call .\gradlew assemble!buildType!
call .\gradlew install!buildType!

:: アプリケーションの起動
adb shell am start -n com.shikarunochi.emulator.!variant!/com.shikarunochi.emulator.EmulatorActivity

cd buildbatch

:: buildbatchに移動して日付フォルダを作成
cd buildbatch
set DATESTR=%date:~-10,4%%date:~-5,2%%date:~-2,2%
set FOLDERNAME=v%DATESTR%_debug_apk

if not exist "%FOLDERNAME%" (
    mkdir "%FOLDERNAME%"
)

:: APKファイルを新しいフォルダにコピー
:: 以下のパスはプロジェクトの構成により適宜調整
copy "..\app\build\outputs\apk\%VARIANT%\debug\*.apk" "%FOLDERNAME%"

endlocal
