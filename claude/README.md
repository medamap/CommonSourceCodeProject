# Claude ワークスペース

このフォルダは、CommonSourceCodeProjectのmacOS/iOS/iPadOS移植作業のドキュメントと進捗管理用です。

## ドキュメント一覧

### 📋 調査・分析
- `investigation_summary.md` - プロジェクト調査結果のまとめ
  - 現状分析
  - プラットフォーム間の比較
  - 技術的課題

### 📝 実装計画
- `implementation_tasks.md` - 実装タスクと進捗管理
  - フェーズ別タスクリスト
  - 実装順序と依存関係
  - 進捗状況

- `metal_implementation_plan.md` - Metal描画システムの詳細設計
  - Android OpenGL ES実装の分析
  - Metal実装アーキテクチャ
  - シェーダー設計
  - パフォーマンス最適化

### ⚠️ 重要なガイドライン
- `IMPORTANT_GUIDELINES.md` - **必読：コアソースコード修正時の厳格なルール**
  - 保護対象ファイルの定義
  - 修正時の署名ルール
  - プリプロセッサ使用方法

## 現在の状況

### ✅ 完了
- プロジェクト構造の調査
- 既存実装（Win32/Android）の分析
- 移植計画の策定

### 🚧 作業中
- Metal描画システムの実装準備

### 📅 今後の予定
1. MetalビューとレンダラーのSwift実装
2. C++ブリッジ関数の拡張
3. 基本的な画面バッファ転送機能

## 重要な技術的決定事項

### レンダリング
- **選定**: Metal（OpenGLは非推奨のため）
- **理由**: 最新のApple GPUに最適化、将来性

### アーキテクチャ
- **方針**: 機能ごとにファイル分離
- **理由**: 保守性とテスタビリティの向上

### Swift-C++統合
- **方法**: Objective-C++ブリッジ経由
- **理由**: 最も安定した統合方法

## 参考リンク
- [Metal Programming Guide](https://developer.apple.com/metal/)
- [Swift-C++ Interoperability](https://www.swift.org/documentation/cxx-interop/)

## 更新履歴
- 2024/XX/XX - 初期ドキュメント作成
- 2024/XX/XX - Metal実装計画追加