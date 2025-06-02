# 武田さんエミュレータ Mac / iPhone / iPad 移植プロジェクト

## 概要

- 武田さんレトロコンピュータエミュレータプロジェクト Common Source Code Project (CSCP) 通称「武田さんエミュレータ」を Mac / iPhone / iPad に移植します
- 先に Mac に移植し、次に iPad / iPhone の順に移植を進めます
- CSCP Android 移植版レポジトリ https://github.com/medamap/CommonSourceCodeProject.git develop ブランチをクローンし feature/mac-platform-support にブランチしたワーキングフォルダが作業フォルダとなります

## 機種コードについて

- CSCPにおいてはおよそ100を超えるレトロパソコンに対応しており、それぞれ機種を判別するコードをここで便宜上設定する
- vc++2017/ フォルダの vcxproj 拡張子ファイルのファイル名部分を機種コードとし、原則として半角小文字英数字とする
- 以後、機種コードを含めた各種シンボルやファイル名を例えば _{機種コード大文字}.txt という風に表記する (x1turbo なら _X1TURBO.txt となる)

## 参考資料

- 作業フォルダにサブモジュールとしてもうひとつ https://github.com/medamap/CommonSourceCodeProject.git を読み込みます。ブランチは feature/apple-platform-support とし、フォルダ名は CSCP-Reference とします
  - このフォルダは前回 Mac に移植しようとして途中で作業を打ち切ったものですが、参考にできるコードが豊富なのでサブモジュールとしてチェックアウトし、必要なときにここからファイルをコピーしたり参照したりするときに使用し、作業が全て完了すればサブモジュールから削除されるものです
- androidstudio/app/src/main/cpp/CMakeLists.txt で定義されている各種オプションは重要です Android では以下のオプションが指定されています、以後ビルドオプションと便宜上呼称します
  - _{機種コード大文字} ... 例えば _BMJR, _MSX1, _X1, _X1TURBO などです、これがないとビルドが正常にできません
  - _Android ... Android独自の実装をビルドします、今回の Mac/iPhone/iPad では不要です、逆に __APPLE__ 等のプラットフォーム個別のプリプロセッサが必要です
  - _RGB565 ... どこで使われてるんでしょう？ Android 実装時に追加されたシンボル？ちょっとソースコードを解析して必要ならつけてください
  - _EXTEND_MENU ... Android 実装時に追加されたシンボルです、これはメニュー機能を利用する時につけていた気がします、特につけなくてもいいかも
  - _USE_OPENGL_ES30 ... Android 実装時に追加されたシンボルです、OpenGLES3.0 で動作させるためのものでしたが、今回 Mac では不要でしょう
  _ _HAS_TENKEY ... テンキーを持つレトロパソコンの時に実装します、これは通常の0-9を押した時にテンキーの0-9を押す判定をさせるためのもので Mac では不要です

## ルール

- ここの ToDo をまるごとコピーして ToDo.md を作り、進捗管理とします。すでに ToDo.md に項目があれば追記としてください。
  - ToDo にはチェック項目をつけ、どこまで完了したかわかるようにします。
  - ToDo の内容について問い合わせがあったり、ToDo を実行してくださいなどの指示を行います。
  - ToDo 項目が完了すればチェックし、報告してください(完了したかどうかは支持者が判断し承認します)
  - 細かい調整以外は基本指示は ToDo ベースとします。
  - 適宜 ToDo.md を更新保存してください、途中でクラッシュしても続きから作業開始できるようにです
- ToDo を全て完了したら「トップフォルダの CLAUDE.md の ToDo を更新してください」とわたしに伝えてください。
- 最終的なコードビルドはユーザーに任せるようにしてください

## フォルダ構成及び参考情報

```
CommonSourceCodeProject/
|
+- androidstudio/
|  + app/
|  |  + src/
|  |  |  + {機種コード}/res/mipmap-*/ic_launcher.png ... サイズ別のアプリアイコン画像です、Mac/iPhone/iPad のアイコンとして利用するのは mipmap-xxxhdpi/ic_launcher.png が良いでしょう
|  |  |  + {機種コード}/res/values/strings.xml ... アプリケーション名の文字列が例えば <resources><string name="app_name">X1 Emulator</string></resources> という風に格納されます
|  |  |  + main/
|  |  |     + cpp/
|  |  |        + _{機種コード大文字}.txt ... Android版のビルドターゲットはここでわかります
|  |  |        + CMakeLists.txt ... Android版のビルド設定、各機種の _{機種コード大文字}.txt をインクルードしてます
|  |  |        + また、機種毎にビルドオプションで各種プリプロセッサを定義していますが、ここで定義されているシンボルは重要です
|  |  + build.gradle ... Android ビルド設定 gradle です、ビルドバリアント設定などもここに含まれます
|  |
|  + buildbatch/
|  |  |
|  |  + models/*.csv ... 一括ビルドするための機種名、機種コードのリストが各種格納される
|  |  + subBatch/*.bat ... allexecute.bat から呼び出されるバッチファイルサブルーチン
|  |  + allexecute.bat ... 一括ビルドするためのバッチ、このバッチを呼び出してアプリケーションをビルドする
|  |
|  + vYYYYMMDD_debug_apk/ ... allexecute.bat 実行で生成されるDebug版 APK 格納フォルダ(YYYYMMDDには日付が入る)
|  + vYYYYMMDD_release_apk/ ... allexecute.bat 実行で生成されるRelease版 APK 格納フォルダ(YYYYMMDDには日付が入る)
|
+- CSCP-Reference ... 前回 Mac 版移植しようとして失敗した CommonSourceCodeProject レポジトリ、サブモジュールでチェックアウトしている、参考資料として多分使えるかも
|  |
|  + src/Xcode/*.h|*.mm|*.txt|*.cpp|*.md|*.metal ... 前回 Mac 版移植で実装した OSD レイヤー移植コード、多分参考になると思う、参考にする程度にしておいて
|
+- oboe/ ... Android 用オーディオライブラリ
|
+- src/ ... エミュレータソースコード
|  |
|  + Android/ ... Android版OSDレイヤーのソースは大体ここの中
|  |  |
|  |  + android_main.cpp ... Android版のメインループ、OpenGLES 描画及び、メニュー処理はここのソースが参考にできます、基本 WinMain からの移植ですが、全ての機能が網羅できてないので劣化移植ではありますことに注意
|  |  + osd_console.cpp ... コンソール関係、おそらくエミュレータ内デバッガ機能（レトロパソコンのCPUなどをトレースしたりする）で使われていると思いますが、多分劣化移植なので Win版を参考にした方がよいです
|  |  + osd_input.cpp ... キー入力、ジョイパッド、マウス入力などを移植していますが、Win32 の劣化移植
|  |  + osd_midi.cpp ... MIDI機能、確か Android の Java レイヤーとのやりとりで MIDI 出力をしてた気がする、Win 版のも参考にして
|  |  + osd_screen.cpp ... Win版からの劣化移植、中身はほとんど空っぽ、主に多分画面表示関係
|  |  + osd_socket.cpp ... ネットワーク通信機能、確か MZ-2500 からしか使われてなかった気がする、まだ一度も動作確認してないのでちゃんと動くかは未知、参考にするならWin版
|  |  + osd_sound.cpp ... サウンド出力機能、こっちはがんばって移植した、oboe を使用して再生している、Mac/iPhone/iPad での再生はこっちを参考にした方がよいと思う
|  |  + osd.cpp ... Winからの劣化移植
|  |  + osd.h ... キーコード変換表とか、Winから移植したのとかいろいろはいってる、割とごちゃごちゃしてる
|  |  + windows_define.h ... Win32で使われてるシンボルを互換性のために定義している
|  |  + menu/*.cpp|*.h ... Android版メニュー定義、内容的にはピュアC++で機種依存性がないため、現在 src/menu に移行中
|  |
|  + res/
|  |  + {機種コード}/*.png ... レトロパソコン毎に使用される個別の画像リソース
|  |  + {機種コード}.ico ... 機種毎のエミュレータアプリのアイコン
|  |  + {機種コード}rc ... リソース情報、機種毎のエミュレータのメニューバー定義及び、各種画像リソースやアイコン情報も定義されている
|  |
|  + vm/ ... OSD層(OS別依存層) から呼び出されるVM層のソースコードが格納される、この階層はコアソースコードとなり、原則修正してはいけない、やむを得ず修正する場合は必ずプラットフォーム毎のプリプロセッサとコメントを振り、他プラットフォームとの互換性をとらなければいけない
|  |  |
|  |  + {機種コード}/*.cpp|h ... 各機種のレトロパソコンに特化したハードウェアをエミュレートするソースコード群
|  |  + *.cpp|h ... 各機種のレトロパソコンで共有される各種チップをエミュレートするソースコード群（例：Z80, DMA, サウンドチップ等）
|  |
|  + win32/
|  |  + osd_console.cpp
|  |  + osd_input.cpp
|  |  + osd_midi.cpp
|  |  + osd_screen.cpp
|  |  + osd_socket.cpp
|  |  + osd_sound.cpp
|  |  + osd_video.cpp
|  |  + osd.cpp
|  |  + osd.h
|  |  + winmain.cpp
|  |
|  + Xcode/ ... まだ未実装領域、今回の Mac/iOS/iPad 移植のための OSD層の実装、win32とAndroidのOSD層をそれぞれ参考にする、win32が完璧版実装でAndroidは参考実装
|  |  + osd.cpp ... Win版とAndroid版の良いとこ取り
|  |  + osd.h ... プラットフォーム共通インターフェース
|  |  + osd_screen.mm ... Metal/CoreGraphics実装
|  |  + osd_sound.mm ... CoreAudio実装、実装方針は Android の osd_sound.cpp が参考になる
|  |  + osd_input.mm ... GameController framework使用
|  |  + osd_console.cpp ... Win版ベースで完全実装
|  |  + osd_socket.cpp ... BSD socket実装
|  |  + xcode_main.mm ... Swift呼び出し用エントリポイント、winmain.cpp や android_main.cpp を参考にする、ここにはメインループだけおさめたい、次の行の注釈参照
|  |    ※ winmain.cpp も android_main.cpp もものすごく機能が豊富すぎて行数がものすごく多いので、分割したい
|  |      含まれる機能として 画面表示、キー入力、ダイアログ、メニューバー、アイコン描画、アイコンアニメーション、メインループ、ウインドウイベントハンドラ、アプリ初期化、コンフィグ設定、ステータスバー、ファイル操作、ジョイパッド・マウス入力（タッチ入力）、ボタン、仮想記録メディア　これら全てが１つのファイルに収まっているので、機能カテゴリ毎に分割したい
|  |      |
|  |      + xcode_sub_screen.mm ... 例えば画面表示系はこのファイルに分割実装する
|  |      + xcode_sub_keyboard_input.mm ... キー入力はこのあたり、直結キーボード、BLEキーボード、仮想キーボード対応したい
|  |      ...以後それぞれの機能を分割する
|  |
|  | (ここ移行はコアソースコードとなり、原則修正してはいけない、やむを得ず修正する場合は必ずプラットフォーム毎のプリプロセッサとコメントを振り、他プラットフォームとの互換性をとらなければいけない)
|  + common.cpp|h ... 各種共通処理が格納されたソースコード
|  + config.cpp|h ... iniファイルへの保存や読み込み及び、設定情報を取得するための機能を提供するソースコード
|  + debugger.cpp ... エミュレータ上でレトロパソコンのCPUなどをトレースしたりデバッグしたりするための機能を提供したソースコード
|  + emu.cpp|h ... エミュレータ層クラス、ここからOSD層の機能の呼び出しを行う、winmain.cpp や android_main.cpp などメインループから呼び出される
|  + fifo.cpp|h ... 汎用的な循環バッファ（リングバッファ）の実装、エミュレータ内の非同期データ転送で広く使用される
|  |   - シリアル通信の送受信バッファ（i8251, MC6850などのUARTチップ）
|  |   - キーボード入力バッファ（複数の機種で使用）
|  |   - 音声データバッファ（LD700など）
|  |   - オートキー機能（自動入力）のバッファ
|  |   - SCSIデータ転送、プリンタバッファなど
|  |   - ステートセーブ/ロード機能も実装済み
|  |   - Mac版では CoreAudio との連携や、各種非同期処理で活用できる可能性あり
|  + fileio.cpp|h ... ファイル入出力やステート保存/読み込み機能などを提供するソースコード
|
+- tool/
|
+- vc++2017/
|  + {機種コード}.vcxproj ... Win版の機種毎のビルドターゲットはここでわかります
|
+- Xcode/ ... まだ未実装領域、今回の Mac/iOS/iPad 移植のためのビルド設定フォルダ
   |
   + CMakeLists.txt ... Mac/iOS/iPad用CMake設定、各レトロパソコンに対応したビルドオプションが指定され、Machines フォルダの対応機種のビルドターゲットがインクルードされる
   + Machines/
   |  + _{機種コード大文字}.txt ... 各レトロパソコン用のビルドターゲット、CMakeLists.txt からインクルードされる
   |
   + Templates/ ... 自動生成用テンプレート
   |  + Package.swift.in ... SwiftPMパッケージテンプレート
   |  + Bridge.h.in ... Objective-C++ブリッジヘッダー
   |  + Bridge.mm.in ... Objective-C++ブリッジ実装
   |
   + Scripts/ ... ビルド自動化
   |  + build_all.sh ... 全機種ビルドスクリプト、androidstudio/buildbatch/allexecute.bat あたりを参考にする、生成したフォルダをクリアする clean は必須だね、ビルド時には Emuコアビルド、SwiftPMパッケージ化、アプリビルド、AppStoreパッケージング、全体通して実行、リリースビルド、デバッグビルドと色々指定できるとよいね、これも allexecute.bat の実装を参考にしてください
   |  + SubScripts/*.sh ... build_all.sh から呼び出されるスクリプトファイルサブルーチン、androidstudio/buildbatch/subBatch あたりを参考にする
   |  |
   |  + BuildLogs/
   |  |  + 20240602_143022_X1TURBO_build.log
   |  |  + 20240602_143022_X1TURBO_error.log
   |  |  + Latest/ ... 最新ログへのシンボリックリンク
   |  |     + build.log
   |  |     + error.log
   |  + parse_error.py ... エラー解析補助スクリプト
   |  |
   |  + Models/*.csv ... 一括ビルドするための機種名、機種コードのリストが各種格納される、androidstudio/buildbatch/models/*.csv からの丸ごとコピーで流用
   |  + v{YYYYMMDD}_debug_mac/ ... (.gitignore)ビルドしたアプリ形式を格納する、デバッグ版 Mac アプリフォルダ、レトロパソコン100機種分全部ここに入る
   |  + v{YYYYMMDD}_debug_macfree/ ... (.gitignore)同上、署名なし版
   |  + v{YYYYMMDD}_release_mac/ ... (.gitignore)同上、リリース版 Mac アプリ
   |  + v{YYYYMMDD}_release_macfree/ ... (.gitignore)同上、署名なし版
   |  + v{YYYYMMDD}_debug_ios/ ... (.gitignore)同上、デバッグ版 iOS アプリ
   |  + v{YYYYMMDD}_release_ios/ ... (.gitignore)同上、リリース版 iOS アプリ
   |  + v{YYYYMMDD}_release_iosfree/ ... (.gitignore)署名なし版
   |
   + Generated/ ... CMakeで生成される成果物(.gitignore)
      + {機種コード大文字}Core/
      |  + Package.swift
      |  + Sources/
      |  |  + {機種コード大文字}Bridge/ ... C++をラップするObjC++
      |  |  + {機種コード大文字}Swift/ ... Swiftインターフェース
      |  |
      |  + include/
      |     + module.modulemap ... Swiftへの公開設定
      |
      + {機種コード大文字}App/ ... Swiftアプリケーション
         + {機種コード大文字}App.xcodeproj
```

## アーキテクチャ

- CSCP は機能毎にモジュール化されており、階層構造になっている(メインループ -> EMU層 -> OSD層 -> VM層 -> 各種チップ)
- Windows 版は基本全ての機能は C++ で実装されている
  - Windows 以外のプラットフォームでは EMU, OSD, VM, 各種チップ は C++ でビルドし、メインループ層は独自の言語で動作させ、EMU層にリンクしても良い
- EMU層はエミュレータの機能を呼び出すためのエントリとなり、ここからOSD層が呼び出されるが、ここからVM層が呼び出されることもある
- OSD層はOSプラットフォーム依存の機能をエミュレータから呼び出すために実装され、例えば、サウンド機能、キーやマウスにジョイパッド入力、MIDI入力、画面描画等を担当し、この層からVM層がコールされる
- VM層はレトロパソコン独自の機能が実装される
- 各種チップはVM層からデバイスモジュールとして使用される（例：Z80, CTC, DMA, SIO, FM音源チップ、FDC など）

## 実装方針

- CommonSourceCodeProject を CSCP と便宜上呼称します
- 先に Mac 版で X1turbo エミュレータをほぼ完璧に移植し、その後各機種にてビルド及び実行ができるようにしてから iPad 版、iPhone 版の順で移植作業を実施する
- Mac/iPhone/iPad 版はメインループ処理を Swift で実装する
- EMU/OSD/VM/各種チップ 及びエミュレータコアは C++ でビルドします
- OSD層は Mac/iPhone/iPad の各種ハードウェアにアクセスするOS依存部の吸収層でもあります、 EMU->OSD(Appleハードアクセス)->VM->チップ と、めんどくさい箇所に挟まれた部分でもあります、どうしようこれ、ちょっとめんどくさいなぁ
- EMU/OSD/VM/ChipをEmuコア層と便宜上呼称します
- android_main.cpp / winmain.cpp に相当するメインループ層をアプリ層と便宜上呼称します
- Emuコア層は SwiftPM としてパッケージ化します
- アプリ層は Swift で実装し、Emuコア層とリンクします
- CSCPではおよそ100機種以上のレトロパソコンのエミュレートをサポートし、win版では100以上のexeファイルを、Android版では100以上のAPKを出力します
- Mac版CSCPではおよそ100以上のEmuコア層 SwiftPM をパッケージとして提供し、100以上の対応したアプリ層とそれぞれリンクすることで100以上のMacアプリを出力します
- iPad版CSCPではおよそ100以上のEmuコア層 SwiftPM をパッケージとして提供し、100以上の対応したアプリ層とそれぞれリンクすることで100以上のiPadアプリを出力します
- iPhone版CSCPではおよそ100以上のEmuコア層 SwiftPM をパッケージとして提供し、100以上の対応したアプリ層とそれぞれリンクすることで100以上のiPhoneアプリを出力します
- iPad版 と iPhone版が統合できるなら統合します

## プリプロセッサシンボルの定義

### 必須シンボル
- `_{機種コード大文字}` : 機種識別用（例：_X1TURBO）
- `__APPLE__` : Apple プラットフォーム識別（コンパイラで自動設定されないなら明示的に設定するなど）

### プラットフォーム別シンボル
- `TARGET_OS_MAC` : macOS用
- `TARGET_OS_IOS` : iOS/iPadOS用
- `TARGET_OS_SIMULATOR` : シミュレータ用

### 不要なAndroidシンボル
- `_Android` : 使用しない
- `_USE_OPENGL_ES30` : MetalまたはCoreGraphics使用
- `_HAS_TENKEY` : Mac/iOSでは不要

### 実装優先度
1. 必須：screen, sound, input（基本動作に必要）
2. 重要：console（デバッグ用）、ファイルI/O
3. オプション：midi, socket, video録画

## Objective-C++ ブリッジの注意点

- C++オブジェクトは std::unique_ptr/shared_ptr で管理
- Objective-Cオブジェクトは ARC で自動管理
- ブリッジ層では明示的な所有権管理が必要
- 循環参照に注意（weak_ptr使用）

## SwiftPM と Objective-C++ ブリッジの構造

- EmuコアSwiftPMは以下の構造を持つ：
  - C++層：既存のエミュレータコア（純粋なC++）
  - ブリッジ層：Objective-C++（.mm）でC++をラップ
  - Swift層：公開APIを提供（Swiftアプリから利用）

- module.modulemapでObjective-Cヘッダーを公開
- Package.swiftでcxxLanguageStandardを指定

## デバッグ情報

- Xcode の Scheme で各種デバッグオプション設定
- Address Sanitizer, Thread Sanitizer 推奨
- Metal Performance HUD でGPU性能確認
- Instruments でメモリリーク検出

## ビルド確認フロー

1. 小さな変更ごとに実ビルドを要求
   - 「この変更でビルドを試してください」

2. エラー情報の共有
   - コンパイラエラーの完全なメッセージ
   - どのファイルのどの行でエラーか

3. 段階的な確認
   - C++単体でコンパイル可能か
   - Objective-C++ブリッジが通るか
   - SwiftPMパッケージとして認識されるか
   - Swiftからインポートできるか

## メニュー機能

- Windows版はメニューバーにエミュレータを操作する各種機能が実装されており、これはレトロパソコン毎に {機種コード}.rc に定義されており、winmain.cpp 内で処理されており、メニュー選択でイベントハンドラに入り、そこからiniファイル情報を更新し、エミュレータの挙動を変えたり様々なファイル操作などを行なっています
- Android版もWindowsのメニューバー機能を移植しており、src/menu に C++ で実装された仮想メニュー機能が定義されており、同一フォルダ内に {機種コード}.rc を移植した {機種コード}.cpp が実装されており、メニュー階層構造が構築されています、これはOS依存しておらず、C++のクラスにより管理されており、android_main.cpp 内で処理され、イベントハンドラから Java コードが呼び出され、androidstudio/app/src/main/java/jp/matrix/shikarunochi/emulator/EmulatorActivity.java 内でUI表示処理されます
- Windows版もAndroid版も各種メニュー項目にはiniファイルの設定情報に基づいて選択状態が制御され、メニュー表示時にすでに選択されている項目や選択されていない項目がわかるように制御されています、選択状態は、ラジオボタン、チェックボックスのように、グループ化された選択項目のうち１つだけ有効になっているタイプのものと、ON/OFFがトグル化されているタイプのものがわかれています、これらのチェック状態の管理は winmain.cpp や android_main.cpp 内で行われています
- Mac版は、Windowsのようにシステムのメニューバーにメニューを表示するよう実装してください、src/menu/{機種コード}.cpp で定義された情報をメニューバーに表示し、また、メニュー項目によってはグループ選択状態、トグル選択状態がそれぞれわかるように表示を行うようにしてください
- iOS版は、Androidのようにダイアログによるメニューを表示するよう実装してください、src/menu/{機種コード}.cpp で定義された情報を、ダイアログ表示し、また、メニュー項目によってはグループ選択状態、トグル選択状態がそれぞれわかるように表示を行うようにしてください
- なお、メニュー機能は Swift 機能となるため、C++ のインクルードはできないため、src/menu 機能は不本意ながらEmuコアの SwiftPM に取り込むようにし、階層メニュー情報をメインループである xcode_main.mm などから取得するようにしてください

## ソースコード編集ルール

- コアソースコード
  - src/*.cpp|h src/vm/*.cpp.h src/vm/*/*.cpp.h をコアソースコードとする
  - レトロコンピュータのエミュレータソースコードとなる
  - OS依存分は基本含まれない（という前提）であるため、内容の更新は一切必要がない（という前提）
  - ただし、コンパイラやOS環境により修正が必要となる可能性はありえる
    - 止むを得ない理由で修正が必要な場合、__APPLE__ などのプリプロセッサで処理の切り分けを行い、他のプラットフォームに影響がないように十分に注意する
    - プリプロセッサのシンボルが他にふさわしいものがあればそれらのシンボルも活用する、例えば、Xcode 環境で使用するコンパイラで使用されるシンボルなど (MSC とか WIN32とか)
    - コアソースコードに修正をいれた場合はコメントを入れる // Medamap and Claude : (修正理由の記載を英語で)
  - 動作確認のため、一時的にデバッグ表示をいれたい場合は必ず行末に例えば次のようなコメントを入れる // Medamap and Claude : For debug (ToDo : delete this line)
  - 動作確認が完了すればデバッグ表示は必ず即時削除する（コアソースコードへの一時的なデバッグ表示は残さないようにする）

## リソース情報

- src/res/resource.h の中に __ANDROID__ プリプロセッサが設定されていますが、__APPLE__ も OR 条件で追加します、Android 実装時に拡張したシンボルが今回必要となります

## メディア情報

- レトロパソコンエミュレータは、フロッピーディスク、テープドライブ、クイックディスク、カートリッジなど様々な形態の記憶メディアを使用しており、これらのファイルの取り扱いはそれぞれ winmain.cpp 及び android_main.cpp に実装されています、これらのメディアはレトロパソコンの種類によってどの形態のメディアをいくつのドライブ数だけマウントしているかが異なり、それは下記シンボルによって定義されます
  - USE_CART ... ロムカートリッジ、メディアフォルダ CART (androidstudio/app/src/main/res/drawable/cart.png)
  - USE_FLOPPY_DISK ... フロッピーディスク、メディアフォルダ DISK (androidstudio/app/src/main/res/drawable/floppy.png)
  - USE_QUICK_DISK ... クイックディスク、メディアフォルダ QD (androidstudio/app/src/main/res/drawable/qd.png)
  - USE_HARD_DISK ... ハードディスク、メディアフォルダ HDD (androidstudio/app/src/main/res/drawable/hdd.png)
  - USE_TAPE ... テープドライブ、メディアフォルダ TAPE (androidstudio/app/src/main/res/drawable/tape.png)
  - USE_COMPACT_DISC ... CDドライブ、メディアフォルダ CD (androidstudio/app/src/main/res/drawable/cd.png)
  - USE_LASER_DISC ... レーザーディスク (画像アイコンなし、今回は対応なしでOK)
  - USE_BINARY_FILE ... バイナリファイル、メディアフォルダ BIN (androidstudio/app/src/main/res/drawable/binary.png)
  - USE_BUBBLE ... バブル（なにこれ？バブルカセット？）メディアフォルダ BUBBLE (androidstudio/app/src/main/res/drawable/bubble.png)
- メディアの種類によって使用できる仮想メディアのイメージファイルの拡張子が異なります、具体的には winmain.cpp 及び android_main.cpp を参照してださい

## 画面レイアウト

- エミュレータ画面のレイアウト構成は android_main.cpp を参考にしてください
  - エミュレータ画面 ... レトロパソコンの画面を画面上部に寄せて表示します
  - システムアイコン ... 縦画面の時はエミュレータ画面の上に右から左方向に並べます、横画面の時はエミュレータ画面の右に上から下に並べます
  - メディアアイコン ... 縦画面の時はエミュレータ画面の下に左から右方向に並べます、横画面の時はエミュレータ画面の左に上から下に並べます
  - テープドライブプログレスバー ... テープメディアのローディング時のプログレス情報をエミュレータ画面の下に表示します
  - 仮想キーボード ... 縦画面の時はエミュレータ画面とテープドライブプログレスバーの下に、横画面の時は画面下方向になるべく寄せて半透明で表示します（エミュレータ画面とおそらく重なるでしょう）
- 縦画面、横画面の切り替えはフレキシブルに変化します

## アイコン機能

- Android版では、エミュレータの機能にアクセスするためのアイコン機能が実装されています、このアイコン機能を Mac/iPhone/iPad に移植してください
- アイコン機能は android_main.cpp 及び EmulatorActivity.java に実装されています
- アイコン画像は androidstudio/app/src/main/res/drawable/*.png に格納されているのでコピー流用してください、どの画像がどのアイコンになるかは EmulatorActivity.java を参考にしてください
- アイコンは縦画面か横画面かで表示レイアウトが変わります
  - 縦画面 ... 画面上部にシステムアイコンが並び、画面下部にメディアアイコンが並びます
  - 横画面 ... 画面右部にシステムアイコンが並び、画面左部にメディアアイコンが並びます
  - iPhone/iPad では画面縦、横切り替え時にフレキシブルにレイアウトが切り替わります
- システムアイコンはタップするとアイコンに設定された各種機能が実行されます、具体的な実装は android_main.cpp を参考にしてください、また、それぞれの機能が osd.h の enum systemIconType にて定義されています
  - SYSTEM_EXIT ... アプリ終了(androidstudio/app/src/main/res/drawable/exit.png)
  - SYSTEM_RESET ... エミュレータリセット(androidstudio/app/src/main/res/drawable/reset.png)
  - SYSTEM_SOUND ... オーディオ出力ON/OFFトグル(androidstudio/app/src/main/res/drawable/sound.png)
  - SYSTEM_PCG ... PCG機能ON/OFFトグル（PCG機能を実装した機種のみ表示）(androidstudio/app/src/main/res/drawable/pcg.png)
  - SYSTEM_CONFIG ... メニューダイアログ表示、Macではメニューバーが表示されるので不要 (androidstudio/app/src/main/res/drawable/config.png)
  - SYSTEM_KEYBOARD ... 仮想キーボード表示ON/OFFトグル、Macでは不要 (androidstudio/app/src/main/res/drawable/keyboard.png)
  - SYSTEM_MOUSE ... レトロパソコンでのマウス機能ON/OFF (androidstudio/app/src/main/res/drawable/mouse.png)
  - SYSTEM_WALLPAPER ... エミュレータ画面の奥に壁紙を表示するためのダイアログを表示し選択した画像を壁紙表示 (androidstudio/app/src/main/res/drawable/wallpaper.png)
  - SYSTEM_MIDI ... MIDI機能ON/OFFトグル、ON時に接続されたMIDIに接続する (androidstudio/app/src/main/res/drawable/midi.png)
  - SYSTEM_COLORBLIND ... 色覚形に対応した画面表示を順次切り替えます (androidstudio/app/src/main/res/drawable/blindness.png)
  - SYSTEM_JOYSTICK ... ジョイパッド機能ON/OFFトグル、ONで接続されたパッドに接続します(androidstudio/app/src/main/res/drawable/joystick.png)
  - SYSTEM_SCREENSHOT ... スクリーンショット撮影 (androidstudio/app/src/main/res/drawable/screenshot.png)
  - システムアイコンはON/OFFを管理するものはONの時は白、OFFの時はグレー表示されます
- メディアアイコンはタップすると、該当ドライブに対するメディア挿入ダイアログ画面が表示されます
  - 例えばフロッピードライブを4台搭載したレトロパソコンは、フロッピーアイコンが4つ表示され、順に0ドライブ、1ドライブ・・・と設定されます
  - メディアアイコンはアクセスがないときは白、アクセスがあるときは赤く表示されます、例外として、X1turboは2D/2DDドライブはアクセス中は赤、2HDドライブはアクセス中は緑になります
  - メディアアイコンはドライブにメディアが挿入されてない時はグレー表示されます

## ステート機能

- CSCP ではエミュレータの状態を保存したり復帰したりするステート保存・復帰機能がある
- Android 版の実装を参考にする（Android 版はステート保存した時のエミュレータ画面のスクリーンショットが一緒に表示される、ただし、Mac 版の場合は Windows 版の実装を参考にする、メニューバーにエミュレータ画面のスクリーンショット表示できないよね？）

## エミュレータロム・イメージファイル

- レトロコンピュータエミュレータは、それぞれのパソコンで使用されるBIOSやフォントファイルが必要となる
- また、レトロコンピュータで動作させるメディアのイメージファイルが必要となる
- それぞれのパソコンにおけるBIOS及びイメージファイルは下記の場所に設定される
- Mac は Documents は書類フォルダとなる
- iPhone や iPad は Documents はアプリのデータとして認識できるパスを適宜設定する

```
Documents/
  + emulator/
     + {機種コード}ROM/ ... BIOS/FONT/WAV などのエミュレータが読み込む各種ファイルが格納される
        + CART/ ... ROMカートリッジ
        + DISK/ ... フロッピーディスク(2D/2DD/2HD など)
        + QD/ ... クイックディスク
        + HDD/ ... ハードディスク
        + TAPE/ ... テープドライブ
        + CD/ ... CD
        + BIN/ ... バイナリファイル
        + BUBBLE/ ... バブルファイル
```

- 例えば、X1 や X1turbo や MSX は次のようになる

```
Documents/
  + emulator/
     + x1ROM/
        + DISK/
        + HDD/
        + TAPE/
     + x1turboROM/
        + DISK/
        + HDD/
        + TAPE/
     + msx1ROM/
        + CART/
        + TAPE/
```

## Machines/_{機種コード大文字}.txt (ビルド設定)の作り方

- ビルド設定は、各種レトロパソコンを構成するビルドターゲットファイルを指定する
- 今回は Emuコア SwiftPM をビルドする時のターゲットになると思われる
- レトロパソコンを構成するソースファイルは、vc++2017/{機種コード}.vcxproj を参考に移植する
- 上記 vcxproj ファイルから必要な cpp や h ファイルがわかるので、それらのファイルをビルド設定に追加する
- 追加したファイルのうち、OSD層及び winmain.cpp など、OS に依存したソースファイルは除外する
- 代わりに Xcode 用に作成された OSD層のソースコードをビルド設定に追加する
- Android 版のビルド設定を見ると、どのようにビルド設定を作るのかがわかりやすいと思われる

## Emuアプリメインループの実装

- メインループ（xcode_main.mm）は機種毎に実装せず、機種毎に設定された各種プリプロセッサシンボルに応じてビルドされるようにする（え？ *.h で設定されたシンボルでそんなんできるんですか？）
- ビルド設定で各種レトロパソコン用にビルドされたメインループは、各種レトロパソコン用にパッケージ化された EmuコアSwiftPM とリンクしてひとつのエミュレータアプリとなる

## 署名やプロビジョニング

- ビルドバッチは、署名なし版、署名あり版と両方対応できるようにする
- アプリ開発初期は署名なしで開発し、中盤あたりで Apple Developer Program に加入し、プロビジョニングを作成する予定

## 実装中の確認方法

- ビルドスクリプトは、ビルドログをログフォルダに出力するようにしてください
- 実装が一段落するたびにわたしに「ビルドを実行してください」と促し、実行すべき cd コマンドとビルドコマンドとビルドしたアプリの実行コマンドを提示してください
- ビルド完了してエラーが出たら、エラーが出ましたと報告しますので、latest のログ情報を参照して分析および対応してください

## もし実装に困ったら

- CSCP-Reference に前回 Mac に移植したときの CSCP のソースコード群があります、OSD 層もあらかた実装が終わっています、いろいろ問題はあったけど

## ToDo

- xcode ビルド環境の構築
  - フォルダ構成で指定された Xcode 及び src/XCode フォルダ及び関連ファイルの作成（最初の時点で作れるものだけでよい、あとは適宜作成）
- レトロコンピュータ(機種コード:x1turbo)のMac環境への移植
  - レトロコンピュータビルド設定ファイル _X1TURBO.txt の作成
  - CMakeLists.txt の作成、androidstudio/app/src/main/cpp/CMakeLists.txt を参考に作成する、各機種レトロパソコンのインクルード処理及び、ビルド時のオプション指定などもここでわかる範囲で全機種設定しておく（x1turbo 以外はエラーになるが X1turbo の実装を終わらせてから残りの分を作成する）
  - OS依存レイヤー(osd)の Mac 環境への移植
  - EmuコアSwiftPM X1turbo 版の作成
  - Emuアプリメインループの実装、winmain.cpp 及び android_main.cpp を参考に実装する、EmuコアSwiftPM X1turbo にリンクする
  - Mac版エミュレータとして完成させる

