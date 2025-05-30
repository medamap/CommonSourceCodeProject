# Developer署名用ファイル

このディレクトリには、Apple Developer Program用の署名ファイルを配置します。

## 必要なファイル

### プロビジョニングプロファイル

1. [Apple Developer Portal](https://developer.apple.com/account/resources/profiles/list)にログイン
2. 「Profiles」で新規作成または既存のものを選択
3. ダウンロードした`.mobileprovision`ファイルをこのディレクトリにコピー

例：
- `CSCP_iOS_Distribution.mobileprovision` - iOS配布用
- `CSCP_Mac_Distribution.mobileprovision` - macOS配布用

### 証明書（オプション）

通常、証明書はキーチェーンにインストールされているため、
このディレクトリにコピーする必要はありません。

チームで共有する場合のみ：
- `Certificates.p12` - 証明書と秘密鍵

## ファイル名の設定

`signing/config.sh`で以下のように設定：

```bash
# プロビジョニングプロファイル名（拡張子なし）
DEV_PROVISION_IOS="CSCP_iOS_Distribution"
DEV_PROVISION_MAC="CSCP_Mac_Distribution"
```

## セキュリティ

**重要**: これらのファイルはGitにコミットしないでください！
`.gitignore`で除外設定済みです。