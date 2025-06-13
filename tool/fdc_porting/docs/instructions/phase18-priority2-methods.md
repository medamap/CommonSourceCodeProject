# Phase 18: 優先度2メソッド実装エージェント指示書

## エージェント名
Priority2Agent-Phase18

## 作業目的
Phase 16の監査で発見された優先度2（重要）のモック実装メソッドを、完全な実装に置き換える

## 前提情報
- Phase 17で致命的メソッドは実装済み
- search_track()、search_sector()、search_addr()が現在モック実装
- これらはディスクアクセスの精度に直接影響する重要メソッド

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 実装対象
- `src/vm/mb8877.cpp` - オリジナル実装の参考
- `src/vm/disk.h` - DISKクラスインターフェース
- `tool/fdc_porting/docs/reports/phase16-implementation-audit-report.md` - 監査結果

## 実装項目

### 1. search_track()の実装（1405-1414行目）
現在の状態：
```cpp
// Mock implementation
if (!fdc[drvreg].head_load) {
    return S_RNF;
}
// Simplified logic...
```

実装すべき内容：
- 実際のトラック検索処理
- トラック番号の検証
- CRCチェック
- 適切なエラーステータス返却

### 2. search_sector()の実装（1495-1505行目）
現在の状態：
```cpp
// Mock implementation using simple logic
bool sector_found = false;
// Simplified search...
```

実装すべき内容：
- 現在のディスク位置からセクタ検索
- セクタIDの照合
- タイミング計算
- 次の転送位置の正確な計算

### 3. search_addr()の実装（1554-1569行目）
現在の状態：
```cpp
// Mock implementation
// Find first available sector...
```

実装すべき内容：
- 次のIDフィールドの検索
- 現在位置からの正確な検索
- IDフィールド読み取り位置の計算
- CRCエラーチェック

## 実装詳細

### search_track()
- disk[drvreg]->get_track()を使用した実際のトラック読み込み
- トラックレジスタとの照合
- シーク動作の正確性確認

### search_sector()
- 現在位置から目標セクタまでの検索
- セクタ間ギャップの考慮
- 複数回転の可能性考慮
- 正確な転送開始位置計算

### search_addr()
- READ ADDRESSコマンド用の実装
- 次のIDフィールドまでの位置計算
- 任意のセクタIDの読み取り

## テスト項目

### 1. 機能テスト
- トラック検索の正確性
- セクタ検索のタイミング
- アドレス読み取りの動作

### 2. エラーケーステスト
- 存在しないトラック/セクタ
- CRCエラー処理
- タイムアウト処理

## 作業手順

1. オリジナルmb8877.cppの該当メソッドを詳細分析
2. ディスクアクセスパターンの理解
3. 各メソッドを順次実装
4. 既存のテストでの動作確認
5. エラーケースの適切な処理

## 成果物

1. 更新ファイル:
   - `src/vm/mb8877_compat.cpp` - 3メソッドの完全実装

2. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase18-priority2-methods-report.md`

## レポート記載事項
- 実装した3メソッドの詳細
- モック実装からの改善点
- ディスクアクセス精度の向上
- 残存する実装課題
- 最終的な実装完成度評価

## 注意事項
- ディスクフォーマットの正確な理解
- タイミング計算の精度維持
- 既存の動作との互換性確保
- CRCエラー処理の適切な実装
- パフォーマンスへの配慮