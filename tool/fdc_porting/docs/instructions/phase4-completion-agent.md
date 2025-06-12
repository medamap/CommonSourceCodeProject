# Phase 4 最終完成エージェント指示書

## エージェント情報
- **エージェント名**: CompletionAgent-Phase4-Final
- **役割**: MB8877互換レイヤーの最終完成と95%目標達成
- **作成日時**: 2025/06/12
- **作成者**: PMエージェント
- **優先度**: 最高
- **推定作業時間**: 5-7日

## 背景・現状分析

### 📊 Phase 3完了時の状況
- **Type I Commands**: 45.8% (目標95%まで49.2%不足)
- **Type II Commands**: 38.9% (目標95%まで56.1%不足)
- **Type III Commands**: 実行エラー発生中 (緊急修正必要)
- **Type IV Commands**: 64.7% (目標95%まで30.3%不足、最有望)

### 🎯 Phase 4の使命
**MB8877互換レイヤーの実用レベル完成**
- 全コマンドタイプで95%以上の通過率達成
- 実環境での動作確認
- プロダクションレディな品質確保

## 実行する作業

### 🚨 緊急対応 (Day 1: 最優先)

#### A. Type III Commands実行エラー修正
**現状**: テスト実行時にエラーで停止
**対応**:
1. エラー原因の特定（セグメンテーション違反、nullptrアクセス等）
2. デバッグ情報の追加とスタックトレース分析
3. 根本原因の修正
4. テスト実行可能状態への復旧

**確認方法**:
```bash
./test_type3_commands  # エラーなく実行完了すること
```

#### B. Type II Commands通過率回復
**現状**: 38.9% (前回より低下)
**対応**:
1. DRQ処理の完全実装
2. セクタ読み書き機能の詳細実装
3. エラーフラグ処理の修正

### 🎯 段階的95%達成戦略 (Day 2-6)

#### Stage 1: Type IV Commands → 95%達成 (Day 2)
**現状**: 64.7% (最も有望)
**重点項目**:
1. **IRQ処理の完全実装**
   - Force Interrupt条件フラグの詳細処理
   - IRQタイミングの精密制御
   - 割り込み条件の正確な実装

2. **BUSY状態管理の完成**
   - 各種操作中のBUSY制御
   - Force Interrupt時の状態リセット
   - コマンド完了時の状態更新

**目標**: Type IV Commandsで95%以上達成

#### Stage 2: Type I Commands → 95%達成 (Day 3)
**現状**: 45.8%
**重点項目**:
1. **シーク動作の完全実装**
   - RESTOREコマンドの詳細動作
   - SEEKコマンドの精密制御
   - STEPコマンド群の完全実装

2. **フラグ処理の完成**
   - TRACK00フラグの正確な制御
   - SEEKERRフラグの適切な設定
   - HEADLOADフラグの管理

3. **タイミング制御**
   - ステップレート制御 (6ms/12ms/20ms/30ms)
   - シーク完了タイミング
   - ベリファイ処理

**目標**: Type I Commandsで95%以上達成

#### Stage 3: Type III Commands → 95%達成 (Day 4)
**前提**: Day 1でエラー修正完了
**重点項目**:
1. **Read Address実装**
   - ID フィールド読み取り
   - CRCエラー処理
   - セクタ情報の正確な返却

2. **Read/Write Track実装**
   - トラック全体の読み取り/書き込み
   - フォーマット機能
   - データ整合性確保

**目標**: Type III Commandsで95%以上達成

#### Stage 4: Type II Commands → 95%達成 (Day 5-6)
**現状**: 38.9% (最も複雑)
**重点項目**:
1. **DRQ処理の完全実装**
   - データ転送タイミング
   - DRQフラグの精密制御
   - データロスト検出

2. **セクタアクセスの完成**
   - Read Sector詳細実装
   - Write Sector詳細実装
   - マルチセクタ処理

3. **エラーハンドリング**
   - CRCエラー処理
   - Record Not Found処理
   - Write Protect処理

**目標**: Type II Commandsで95%以上達成

### 🔧 技術的実装ガイドライン

#### 1. デバッグとトラブルシューティング
```bash
# デバッグビルド
make clean
CXXFLAGS="-g -O0 -DDEBUG" make

# デバッガー実行
gdb ./test_type3_commands
(gdb) run
(gdb) bt  # エラー時のスタックトレース
```

#### 2. 段階的テスト実行
```bash
# 各段階での確認
./test_type4_commands | grep "Success rate"  # 95%以上を確認
./test_type1_commands | grep "Success rate"  # 95%以上を確認
./test_type3_commands | grep "Success rate"  # 95%以上を確認
./test_type2_commands | grep "Success rate"  # 95%以上を確認
```

#### 3. 詳細実装のポイント

**a. タイミング制御の精密化**
```cpp
// 正確なタイミング計算
double get_usec_per_bytes(int bytes) {
    return (double)bytes * 8.0 / (drive_rpm == 300 ? 500000.0 : 250000.0);
}
```

**b. エラーフラグの正確な設定**
```cpp
// CRCエラーの適切な処理
if (crc_error) {
    status |= S_CRC;
    set_irq(true);
}
```

**c. DRQ制御の完成**
```cpp
// DRQタイミングの精密制御
void register_drq_event(double usec) {
    register_my_event(EVENT_DRQ, usec);
    prev_drq_clock = get_current_clock();
}
```

## 期待される成果物

### 1. **完成した実装ファイル**
- `src/vm/mb8877_compat.cpp` (95%品質の完全実装)
- `src/vm/mb8877_compat.h` (必要な更新)

### 2. **テスト結果証明**
- 各コマンドタイプで95%以上の通過率
- 全テストスイートの統合実行結果
- エラーゼロの動作確認

### 3. **完成レポート**
- `docs/reports/phase4-final-completion-report.md`
- 各段階の達成状況詳細
- 実装完了の技術的証明
- Phase 5への準備状況

### 4. **品質証明書類**
- コンパイル警告ゼロの確認
- メモリリーク検査結果
- 実行時エラーゼロの証明

## 作業進行管理

### 📅 Daily Milestone

#### Day 1: 緊急対応日
- [ ] Type III Commands実行エラー修正
- [ ] Type II Commands通過率回復確認
- [ ] 全テスト実行可能状態確保

#### Day 2: Type IV Commands完成
- [ ] IRQ処理完全実装
- [ ] BUSY状態管理完成
- [ ] **95%達成確認**

#### Day 3: Type I Commands完成
- [ ] シーク動作完全実装
- [ ] フラグ処理完成
- [ ] **95%達成確認**

#### Day 4: Type III Commands完成
- [ ] Read Address実装
- [ ] Read/Write Track実装
- [ ] **95%達成確認**

#### Day 5-6: Type II Commands完成
- [ ] DRQ処理完全実装
- [ ] セクタアクセス完成
- [ ] **95%達成確認**

#### Day 7: 統合確認・品質保証
- [ ] 全テスト95%以上確認
- [ ] 品質メトリクス確認
- [ ] 完成レポート作成

### 🎯 成功基準

#### 必須達成項目
1. **Type I Commands**: 95%以上
2. **Type II Commands**: 95%以上
3. **Type III Commands**: 95%以上
4. **Type IV Commands**: 95%以上

#### 品質基準
1. **コンパイル警告**: ゼロ
2. **実行時エラー**: ゼロ
3. **メモリリーク**: ゼロ
4. **セグメンテーション違反**: ゼロ

#### 統合基準
1. **平均通過率**: 95%以上
2. **全テスト実行**: エラーなし
3. **一貫性**: 複数回実行で同じ結果

## 完了後の移行準備

### Phase 5への準備
1. **実環境テスト準備**: FM7/X1エミュレータ対応
2. **特殊ディスク対応**: RIGLAS、Batten Tanuki等
3. **パフォーマンス最適化**: プロファイリング準備

この指示書に従い、MB8877互換レイヤーの実用レベル完成を達成してください。95%目標の達成により、プロジェクトは次の実用段階に進むことができます。