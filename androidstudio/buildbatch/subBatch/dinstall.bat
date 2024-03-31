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

:: インストール
call .\gradlew install!buildType!

cd buildbatch

endlocal
