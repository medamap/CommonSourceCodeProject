# Xcodeバッチビルドシステム

macOS/iOS/iPadOS向けのバッチビルドシステムです。
無料版（個人チーム）とDeveloper Program版の両方に対応しています。

## 初期設定

### 1. 設定ファイルの作成

```bash
cd signing
cp config.sh.template config.sh
# config.shを編集してあなたの情報を入力
```

### 2. 実行権限の付与

```bash
chmod +x allexecute.sh
chmod +x subBatch/*.sh
```

## 使用方法

### macOS版

#### 無料版（署名なし）
```bash
# 全モデルをビルド
./allexecute.sh -i models/models.csv -m MacFree

# 特定モデルをビルド
./allexecute.sh -d x1turbo -m MacFree
```

#### Developer版（署名付き）
```bash
# Developer ID署名付きビルド
./allexecute.sh -i models/x1.csv -m MacDeveloper
```

### iOS/iPadOS版

#### 無料版（個人チーム、7日間）
```bash
# Team IDを指定してビルド
./allexecute.sh -d x1turbo -m iOSFree -t YOUR_TEAM_ID
```

#### Developer版（配布用）
```bash
# TestFlight/Ad Hoc配布用
./allexecute.sh -i models/x1.csv -m iOSDeveloper
```

## ビルドモード

| モード | 説明 | 署名 | 配布方法 |
|--------|--------|------|----------|
| MacFree | macOS無署名版 | なし | 直接実行（警告あり） |
| MacDeveloper | macOS署名版 | Developer ID | 公証済み配布 |
| iOSFree | iOS個人版 | 個人チーム | Xcode経由（7日間） |
| iOSDeveloper | iOS配布版 | 配布証明書 | TestFlight/Ad Hoc |

## 出力ディレクトリ

ビルド成果物は日付付きディレクトリに保存されます：

- `v20250531_free_mac/` - macOS無署名版
- `v20250531_dev_mac/` - macOS署名版（DMG含む）
- `v20250531_free_ios/` - iOS個人版
- `v20250531_dev_ios/` - iOS配布版（IPA）

## 署名設定

### 無料版の設定

1. Xcodeを開き、Preferences > Accountsへ
2. Apple IDを追加
3. Team IDを確認（例: `AB12CD34EF`）
4. `signing/config.sh`に設定

### Developer版の設定

1. [Apple Developer](https://developer.apple.com)にログイン
2. Certificates, IDs & Profilesへ
3. 証明書とプロビジョニングプロファイルを作成
4. ダウンロードした`.mobileprovision`を`signing/developer/`に配置
5. `signing/config.sh`に設定

## トラブルシューティング

### 「開発元を検証できない」エラー（macOS）

無署名版の場合：
1. アプリを右クリックして「開く」を選択
2. またはシステム環境設定 > セキュリティとプライバシーで許可

### Team IDが見つからない

```bash
# Xcodeから確認
security find-identity -v -p codesigning | grep "Apple Development"
```

### iOSビルドが失敗する

1. Xcodeでプロジェクトを開き、Signing & Capabilitiesを確認
2. Automatically manage signingをオン
3. Teamを選択

## 注意事項

- `signing/config.sh`は機密情報を含むため、Gitにコミットしないでください
- iOS無料版は7日ごとに再署名が必要です
- macOS Developer版は公証（Notarization）を推奨します