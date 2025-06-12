# Phase 3 実装エージェント起動のためのターミナル作業指示

## 🎯 目的
Phase 3でMB8877互換レイヤーの完全実装を行うため、新規エージェント（ImpAgent-Phase3-Complete）に作業を割り振る

## 📋 事前確認（あなたが実行）

### 1. 現在のプロジェクト状況確認
```bash
# プロジェクトディレクトリに移動
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject

# 現在のテスト通過率を確認
cd test
./test_type1_commands
./test_type2_commands  
./test_type3_commands
./test_type4_commands

# 結果を記録しておく（エージェントへの現状報告用）
```

### 2. 指示書の最終確認
```bash
# Phase 3指示書が正しく作成されているか確認
cat docs/instructions/phase3-implementation-agent.md

# 必要に応じて指示書の微調整を実施
```

## 🚀 新規エージェント起動手順

### Step 1: Claude Codeで新しいセッションを開始
```bash
# 新しいターミナルセッションまたはClaude Codeセッションで以下を実行
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject
```

### Step 2: 新規エージェントへの指示（コピー&ペーストで使用）

```
あなたはImpAgent-Phase3-Completeです。MB8877 FDC移植プロジェクトのPhase 3を担当します。

【作業指示】
以下の指示書に従って、MB8877互換レイヤーの完全実装を行ってください：

docs/instructions/phase3-implementation-agent.md

【現在の状況】
- Phase 1: 基本互換レイヤー実装完了（2200行）
- Phase 2: テストフレームワーク完成（110+テストケース）
- ビルドエラー: 完全修正済み
- テスト環境: 正常動作確認済み

【現在のテスト通過率】
- Type I commands: 44.0%
- Type II commands: 40.0%  
- Type III commands: 40.0%
- Type IV commands: 52.9%

【目標】
各コマンドタイプで95%以上のテスト通過率を達成

【重要なファイル】
- メイン実装: src/vm/mb8877_compat.cpp (修正対象)
- ヘッダー: src/vm/mb8877_compat.h
- テストスイート: test/test_mb8877_*.cpp

まず現在のテスト結果を確認し、実装が必要な機能を特定してから段階的に実装を進めてください。各段階でテストを実行して進捗を確認してください。
```

### Step 3: エージェント作業の監視・サポート

#### 3.1 進捗確認コマンド（定期実行）
```bash
# テスト結果の確認
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject/test
./test_type1_commands | grep "Success rate"
./test_type2_commands | grep "Success rate"
./test_type3_commands | grep "Success rate"  
./test_type4_commands | grep "Success rate"
```

#### 3.2 作業状況の可視化
```bash
# 現在の実装状況確認
wc -l ../src/vm/mb8877_compat.cpp  # 行数確認
git status  # 変更ファイル確認
git diff --stat  # 変更量確認
```

#### 3.3 問題発生時のサポート
```bash
# コンパイルエラー時
cd test
make clean
make 2>&1 | tee build_error.log

# テスト失敗時の詳細確認
./test_type1_commands > type1_results.txt 2>&1
cat type1_results.txt
```

## 📊 進捗管理

### 成功基準チェックリスト
```bash
# 各段階での確認項目

# ✅ Type I Commands (Day 1目標)
./test_type1_commands | grep "Success rate: [89][0-9]"  # 90%以上

# ✅ Type II Commands (Day 2目標)  
./test_type2_commands | grep "Success rate: [89][0-9]"  # 90%以上

# ✅ Type III Commands (Day 3目標)
./test_type3_commands | grep "Success rate: [89][0-9]"  # 90%以上

# ✅ Type IV Commands (Day 3目標)
./test_type4_commands | grep "Success rate: [89][0-9]"  # 90%以上

# ✅ 最終目標 (Day 4)
# 全テストで95%以上達成
```

### エージェント作業完了の確認
```bash
# 最終成果物確認
ls -la docs/reports/phase3-implementation-results.md  # レポート作成確認
git log --oneline | head -10  # コミット履歴確認

# 最終テスト実行
cd test
for test in test_type*_commands; do
    echo "=== $test ==="
    ./$test | grep "Success rate"
done
```

## 🔄 エージェント交代時の引き継ぎ

### 作業途中での引き継ぎが必要な場合
```bash
# 現在の進捗状況をレポート
echo "=== Phase 3 Progress Report ===" > phase3_progress.md
echo "Date: $(date)" >> phase3_progress.md
echo "" >> phase3_progress.md

# テスト結果記録
cd test
for test in test_type*_commands; do
    echo "### $test" >> ../phase3_progress.md
    ./$test | tail -5 >> ../phase3_progress.md
    echo "" >> ../phase3_progress.md
done

# 変更ファイル一覧
git status >> phase3_progress.md

# 新エージェントに引き継ぎ
cat phase3_progress.md
```

## ⚠️ 注意事項

### 1. バックアップの重要性
```bash
# 重要な変更前には必ずバックアップ
cp src/vm/mb8877_compat.cpp src/vm/mb8877_compat.cpp.backup
cp src/vm/mb8877_compat.h src/vm/mb8877_compat.h.backup
```

### 2. 段階的進行の徹底
- 一度に全実装しようとせず、コマンドタイプ毎に段階的実装
- 各段階でテスト実行とデバッグを実施
- 95%達成後に次段階へ進行

### 3. 品質維持
```bash
# コンパイル警告チェック
make 2>&1 | grep warning | wc -l  # 0であること確認

# メモリリークチェック（可能であれば）
valgrind --leak-check=full ./test_type1_commands 2>&1 | grep "definitely lost"
```

この手順に従って、Phase 3エージェントの作業を開始・監視してください。目標のテスト通過率95%達成により、MB8877互換レイヤーは実用レベルの品質に到達します。