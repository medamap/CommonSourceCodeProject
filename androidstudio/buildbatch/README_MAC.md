# Android Studio ビルドバッチスクリプト

複数のプラットフォームを効率的にビルドするためのスクリプト集です。
Windows版（バッチファイル）とmacOS版（シェルスクリプト）の両方が用意されています。

## 必要な環境

### Windows版
- Windows 10/11
- Android Studio または Android SDK
- adb.exe (Android Debug Bridge)
- Gradle (gradlew.bat)

### macOS版
- macOS
- Android Studio または Android SDK
- adb コマンド (Android Debug Bridge)
- Gradle (gradlew)

## 使い方

### 基本的な使い方

#### Windows版
```batch
REM 全モデルをリリースビルド
allexecute.bat -i models\models.csv -m ReleaseBuild

REM 特定のモデルをデバッグビルド
allexecute.bat -d x1turbo -m DebugBuild

REM CSVファイルを指定してビルド
allexecute.bat -i models\pc88.csv -m ReleaseBuild
```

#### macOS版
```bash
# 全モデルをリリースビルド
./allexecute.sh -i models/models.csv -m ReleaseBuild

# 特定のモデルをデバッグビルド
./allexecute.sh -d x1turbo -m DebugBuild

# CSVファイルを指定してビルド
./allexecute.sh -i models/pc88.csv -m ReleaseBuild
```

### コマンドオプション

- `-i csvFile` : モデル情報が記載されたCSVファイルのパス（デフォルト: models/models.csv）
- `-m buildType` : 実行するビルドの種類
  - `ReleaseBuild` : リリースビルド（APK作成）
  - `ReleaseInstall` : リリース版インストールのみ
  - `ReleaseBuildExecute` : リリースビルド＋インストール＋実行
  - `DebugBuild` : デバッグビルド（APK作成）
  - `DebugInstall` : デバッグ版インストールのみ
  - `DebugBuildExecute` : デバッグビルド＋インストール＋実行
  - `UnInstall` : アンインストール
- `-d modelName` : 直接指定するモデル名

### CSVファイルフォーマット

CSVファイルは以下の形式で記述します：

```
表示名,モデル名
X1turbo,x1turbo
PC-8801,pc8801
```

### 個別スクリプト

`subBatch`ディレクトリには個別のビルドスクリプトがあります：

#### Windows版
```batch
cd subBatch
rbuild.bat x1turbo     REM X1turboのリリースビルド
dbuild.bat pc8801      REM PC-8801のデバッグビルド
uninstall.bat msx1     REM MSX1のアンインストール
```

#### macOS版
```bash
cd subBatch
./rbuild.sh x1turbo    # X1turboのリリースビルド
./dbuild.sh pc8801     # PC-8801のデバッグビルド
./uninstall.sh msx1    # MSX1のアンインストール
```

## トラブルシューティング

### Windows版

#### gradlewが見つからない
```batch
REM プロジェクトルートに移動
cd ..
.\gradlew.bat clean
```

#### adb.exeが見つからない
環境変数PATHにAndroid SDKのplatform-toolsを追加：
```batch
set PATH=%PATH%;C:\Users\%USERNAME%\AppData\Local\Android\Sdk\platform-tools
```

#### 文字化け
コマンドプロンプトで文字コードをUTF-8に設定：
```batch
chcp 65001
```

### macOS版

#### 実行権限エラー
```bash
chmod +x allexecute.sh
chmod +x subBatch/*.sh
chmod +x ../gradlew
```

#### adbが見つからない
Android SDKのパスを通すか、フルパスで指定してください：
```bash
export PATH=$PATH:~/Library/Android/sdk/platform-tools
```

#### Gradleエラー
プロジェクトルートで以下を実行：
```bash
./gradlew clean
```

## プラットフォーム間の違い

| 項目 | Windows版 | macOS版 |
|------|-----------|----------|
| ファイル拡張子 | .bat | .sh |
| 実行方法 | `allexecute.bat` | `./allexecute.sh` |
| パス区切り | `\` (バックスラッシュ) | `/` (スラッシュ) |
| 変数参照 | `%変数名%` | `$変数名` または `${変数名}` |
| 文字コード | Shift_JIS または UTF-8 | UTF-8 |
| 大文字変換 | 複雑なバッチ処理 | `tr '[:lower:]' '[:upper:]'` |
| 日付取得 | `%date%` | `date +"%Y%m%d"` |
| エラーチェック | `if %ERRORLEVEL% neq 0` | `if [ $? -ne 0 ]` |

## ビルド成果物

ビルドが成功すると、以下のディレクトリにAPKファイルが作成されます：

- リリースビルド: `buildbatch/v[日付]_release_apk/`
- デバッグビルド: `buildbatch/v[日付]_debug_apk/`

例：
- `v20250531_release_apk/app-x1turbo-release.apk`
- `v20250531_debug_apk/app-x1turbo-debug.apk`

## 利用可能なCSVファイル

`models/`ディレクトリには以下のCSVファイルが用意されています：

- `models.csv` - 全モデル
- `x1.csv` - X1シリーズ（X1, X1turbo, X1turboZ, X1twin）
- `pc88.csv` - PC-8801シリーズ
- `pc98.csv` - PC-9801シリーズ
- `msx.csv` - MSXシリーズ
- `mz.csv` - MZシリーズ
- その他多数