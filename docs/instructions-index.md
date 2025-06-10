# 指示書一覧

## 概要
このファイルは作成された全ての指示書の一覧と概要を管理します。

## 指示書リスト

### 1. investigation-mb8877-structure.md
- **作成日時**: 2025/06/10
- **作成者**: PMエージェント
- **対象エージェント**: InvestigationAgent-MB8877
- **目的**: mb8877.cpp/hの構造を詳細に調査
- **ステータス**: 実行完了
- **主な調査項目**:
  - パブリックメソッドの一覧
  - パブリックフィールドの一覧
  - イベント駆動の仕組み
  - 外部依存関係
  - ディスクI/O処理
- **期待される成果物**: docs/reports/mb8877-structure-analysis.md
- **実際の成果物**: docs/reports/mb8877-structure-analysis.md（作成済み）

### 2. investigation-wd-fdc-structure.md
- **作成日時**: 2025/06/10
- **作成者**: PMエージェント
- **対象エージェント**: InvestigationAgent-WD_FDC
- **目的**: MAMEのwd_fdc.cpp/hの構造を詳細に調査
- **ステータス**: 未実行
- **主な調査項目**:
  - wd_fdc.cppの取得と基本構造
  - パブリックインターフェースの比較
  - ステートマシンの仕組み
  - ディスクI/O実装の差異
  - 特殊機能の対応状況
- **期待される成果物**: docs/reports/wd-fdc-structure-analysis.md
- **実際の成果物**: docs/reports/wd-fdc-structure-analysis.md（作成済み）

### 3. planning-porting-strategy.md
- **作成日時**: 2025/06/10
- **作成者**: PMエージェント
- **対象エージェント**: PlanningAgent-PortingStrategy
- **目的**: MB8877とwd_fdcの調査結果を基に具体的な移植計画を策定
- **ステータス**: 未実行
- **主な検討項目**:
  - 技術的実現可能性の詳細評価
  - 段階的実装計画の詳細化（3フェーズ）
  - リスク分析と対策
  - 実装順序とタスク分割
  - テスト戦略
- **期待される成果物**: docs/reports/porting-strategy-plan.md
- **実際の成果物**: 未作成

## 今後作成予定の指示書

2. **テストスイート作成エージェント用指示書**
   - mb8877.cppの動作を検証するテストコードの作成

3. **実装エージェント用指示書**
   - 実際の移植作業の実施

4. **検証エージェント用指示書**
   - 移植後のコードの動作検証

## 更新履歴
- 2025/06/10: 初版作成（指示書2件を記録）
- 2025/06/10: 指示書3件目（planning-porting-strategy.md）を追加、wd_fdc調査完了を反映