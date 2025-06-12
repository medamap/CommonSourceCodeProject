# CommonSourceCodeProject - MB8877 FDC Porting Branch

## 概要

このブランチ（`feature/porting-mb8877`）は、MB8877フロッピーディスクコントローラー（FDC）のライセンス互換実装を提供します。オリジナルのGPLライセンス実装を、BSD-3-Clauseライセンス互換の新しい実装で置き換えることを目的としています。

### プロジェクトの背景

- **問題**: オリジナルの`mb8877.cpp`はGPLライセンスで、一部のプロジェクトで使用できない
- **解決策**: MAMEプロジェクトの`wd_fdc`実装アプローチを参考に、クリーンルーム実装を作成
- **成果**: 完全なAPI互換性を持つBSD-3-Clauseライセンスの実装

## ディレクトリ構造

```
CommonSourceCodeProject/
├── src/
│   ├── vm/
│   │   ├── mb8877.cpp          # オリジナルGPL実装（参照用）
│   │   ├── mb8877.h            # オリジナルヘッダー（参照用）
│   │   ├── mb8877_compat.cpp   # 新しいBSD互換実装（成果物）
│   │   └── mb8877_compat.h     # 新しいBSD互換ヘッダー（成果物）
│   └── ...
├── tool/
│   └── fdc_porting/
│       ├── test/               # テストスイート（FDCポーティング専用）
│       │   ├── test_mb8877_*.cpp       # 各種テストスイート
│       │   ├── run_comparison_tests.sh # 実装比較テストスクリプト
│       │   ├── Makefile                # テストビルド用
│       │   └── obj/                    # オブジェクトファイル出力先
│       └── docs/                       # FDCポーティング関連ドキュメント
│           ├── instructions/           # 開発指示書
│           ├── reports/               # 開発レポート
│           └── PROJECT_STATUS.md      # プロジェクトステータス
└── backup/
    └── original_mb8877/        # オリジナルファイルのバックアップ
```

## 主要成果物

### 1. MB8877互換実装
- **ファイル**: `src/vm/mb8877_compat.cpp`, `src/vm/mb8877_compat.h`
- **ライセンス**: BSD-3-Clause互換
- **特徴**:
  - 完全なAPI互換性
  - ステートマシンベースの実装
  - MAMEのwd_fdcアプローチを採用
  - 全FDCコマンドサポート（Type I-IV）
  - MB89311拡張モード対応

### 2. 包括的テストスイート
- **テストカバレッジ**: 110以上のテストケース
- **テスト種別**:
  - レジスタアクセステスト
  - Type I コマンド（RESTORE, SEEK, STEP）
  - Type II コマンド（READ/WRITE SECTOR）
  - Type III コマンド（READ ADDRESS/TRACK）
  - Type IV コマンド（FORCE INTERRUPT）
  - エラーハンドリング
  - タイミング検証

## テストスイートの実行方法

### 前提条件
- C++11対応のコンパイラ（g++またはclang++）
- POSIX互換環境（Linux, macOS, WSL）
- make ユーティリティ

### 基本的なテスト実行

```bash
# テストディレクトリに移動
cd tool/fdc_porting/test/

# すべてのテストをビルドして実行
make all
./run_all_tests.sh

# または make test で一括実行
make test

# 個別のテストを実行
./test_mb8877_registers
./test_mb8877_type1_commands
./test_mb8877_type2_commands
./test_mb8877_type3_commands
./test_mb8877_type4_commands
./test_mb8877_error_handling
./test_mb8877_timing
```

### 実装比較テストの実行

オリジナルのmb8877.cppと新しいmb8877_compat.cppの両方に対してテストスイートを実行し、結果を比較できます：

```bash
# テストディレクトリに移動
cd tool/fdc_porting/test/

# 比較テストスクリプトを実行
./run_comparison_tests.sh
```

このスクリプトは以下を実行します：
1. mb8877_compat.cppに対してすべてのテストを実行
2. オリジナルのmb8877.cppに対して同じテストを実行
3. 両者の結果を比較レポートとして出力
4. 結果は`test_results/`ディレクトリに保存

### テスト結果の確認

```bash
# 最新のテスト結果を確認（テストディレクトリ内で実行）
cd tool/fdc_porting/test/
ls -la test_results/

# 比較レポートを表示
cat test_results/comparison_report_*.txt

# 詳細なテスト結果を確認
cat test_results/compat_results_*.txt
cat test_results/original_results_*.txt
```

## 統合方法

プロジェクトでMB8877互換実装を使用するには：

```cpp
// 既存のコード
#include "mb8877.h"

// 以下に変更
#include "mb8877_compat.h"
```

ファイル名以外の変更は不要です。完全なAPI互換性が保証されています。

## ライセンス

- オリジナルmb8877.cpp/h: GPL（Takeda.Toshiya氏による）
- mb8877_compat.cpp/h: BSD-3-Clause互換（このプロジェクトの成果物）

## 開発履歴

詳細な開発履歴は以下のドキュメントを参照：
- `docs/PROJECT_STATUS.md` - プロジェクトの現在の状態
- `docs/reports/` - 各フェーズの詳細レポート
- Phase 1-9まで完了（2025年6月12日）

## 貢献者

- 実装: Claude AI Assistant
- アーキテクチャ設計: MAMEプロジェクトのwd_fdcアプローチを参考
- オリジナル実装: Takeda.Toshiya氏（2006年）

## 注意事項

- このブランチはポーティング作業専用です
- mainブランチへのマージ前に十分なテストが必要です
- 実機での動作確認を推奨します