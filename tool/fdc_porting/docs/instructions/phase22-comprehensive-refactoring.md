# Phase 22: 包括的ディスクアクセス安全化リファクタリング指示書

## エージェント名
ComprehensiveRefactoringAgent-Phase22

## 作業目的
Phase 21で部分修正されたセグフォルト問題を根本的に解決するため、ディスクアクセス全体を安全化し、NULLチェック・境界チェックを体系的に実装する

## 前提情報
- Phase 21で3テストが部分修正（read_io8()まで到達）
- 根本原因: 初期化されていないdisk配列、NULL参照、境界チェック欠如
- 現在の実装完成度: 75%（安定性15%）
- mb8877_compat.cppの安全性向上が急務

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - 主要リファクタリング対象
- `src/vm/mb8877_compat.h` - ヘッダー拡張対象
- `tool/fdc_porting/test/mock_environment.h` - モック環境改善
- `src/vm/mb8877.cpp` - オリジナル実装（安全性参考）

## リファクタリング方針

### 1. 防御的プログラミングの徹底
すべてのディスクアクセスを安全化：
- 配列範囲チェック（drvreg < MAX_DRIVE）
- NULLポインタチェック（disk[drvreg] != NULL）
- 初期化状態チェック（disk配列の初期化完了）
- エラー状態での早期リターン

### 2. 安全なアクセサメソッドの導入
```cpp
// 安全なディスクアクセス用メソッド
bool is_disk_available(int drv);
DISK* get_safe_disk(int drv);
bool validate_drive_index(int drv);
void ensure_disk_initialization();
```

### 3. エラーハンドリングの統一
- 共通エラー処理関数の導入
- 適切なステータス返却
- ログ出力の統一
- graceful degradation（段階的機能低下）

## 実装項目

### Phase 22.1: 基盤安全化（12時間中4時間）

#### 1.1 安全なアクセサメソッド実装
```cpp
class MB8877 {
private:
    bool disk_initialized;
    
    // 安全性メソッド
    bool is_disk_available(int drv) const {
        return (drv >= 0 && drv < MAX_DRIVE && 
                disk_initialized && 
                disk[drv] != NULL && 
                disk[drv]->inserted);
    }
    
    DISK* get_safe_disk(int drv) {
        if (!is_disk_available(drv)) {
            return NULL;
        }
        return disk[drv];
    }
    
    bool validate_drive_index(int drv) const {
        return (drv >= 0 && drv < MAX_DRIVE);
    }
};
```

#### 1.2 初期化プロセスの強化
- disk_initializedフラグの追加
- 段階的初期化の実装
- 初期化失敗時の安全な状態維持

#### 1.3 エラー処理の統一
```cpp
enum SafetyError {
    SAFETY_OK = 0,
    SAFETY_INVALID_DRIVE,
    SAFETY_NULL_DISK,
    SAFETY_NOT_INITIALIZED,
    SAFETY_NOT_READY
};

SafetyError check_disk_safety(int drv);
void handle_safety_error(SafetyError error, const char* operation);
```

### Phase 22.2: I/Oメソッド安全化（12時間中4時間）

#### 2.1 read_io8()の包括的修正
```cpp
uint32_t MB8877::read_io8(uint32_t addr) {
    // 1. 初期化チェック
    if (!disk_initialized) {
        handle_safety_error(SAFETY_NOT_INITIALIZED, "read_io8");
        return 0xFF;  // 安全なデフォルト値
    }
    
    // 2. ドライブインデックス検証
    if (!validate_drive_index(drvreg)) {
        handle_safety_error(SAFETY_INVALID_DRIVE, "read_io8");
        return 0xFF;
    }
    
    // 3. 安全なディスクアクセス
    DISK* safe_disk = get_safe_disk(drvreg);
    if (!safe_disk) {
        // ディスクなしでも動作可能な処理
        return handle_no_disk_read(addr);
    }
    
    // 4. 通常処理
    return perform_safe_read(addr, safe_disk);
}
```

#### 2.2 write_io8()の包括的修正
- 同様の安全性チェック実装
- 書き込み保護チェックの強化
- エラー時のステータス適切な設定

#### 2.3 DMAアクセスの安全化
- read_dma_io8()の修正
- write_dma_io8()の修正

### Phase 22.3: コマンド処理安全化（12時間中4時間）

#### 3.1 Type I コマンドの安全化
```cpp
void MB8877::cmd_restore() {
    // 安全性事前チェック
    if (!ensure_command_safety("cmd_restore")) {
        return;
    }
    
    DISK* safe_disk = get_safe_disk(drvreg);
    if (!safe_disk) {
        handle_no_disk_command();
        return;
    }
    
    // 通常処理
    perform_safe_restore(safe_disk);
}
```

#### 3.2 Type II コマンドの安全化
- cmd_readdata()の包括的修正
- cmd_writedata()の包括的修正
- セクタアクセスの安全性向上

#### 3.3 Type III コマンドの安全化
- cmd_readaddr()の修正
- cmd_readtrack()の修正
- cmd_writetrack()の修正

## テスト戦略

### Phase 22.4: 段階的検証（12時間中残り時間）

#### 4.1 基盤テスト
```bash
# 初期化テスト
./test_mb8877_registers  # 基本I/Oテスト
# 期待結果: セグフォルトなし、適切なエラーハンドリング
```

#### 4.2 コマンドテスト
```bash
# Type I コマンドテスト
./test_mb8877_type1_commands
# 期待結果: 安全な実行、適切なエラー返却

# Type II コマンドテスト  
./test_mb8877_type2_commands
# 期待結果: データ転送の安全な処理
```

#### 4.3 統合テスト
```bash
# 全テスト実行
make clean && make all && ./run_all_tests.sh
# 期待結果: セグフォルト0、成功率50%以上
```

## 実装ガイドライン

### 1. 安全性優先の原則
- パフォーマンスより安定性
- 失敗時のgraceful degradation
- 詳細なエラーログ

### 2. 段階的実装
- 小さな修正の積み重ね
- 各段階でのテスト実行
- 回帰の早期発見

### 3. 互換性維持
- オリジナルAPIの保持
- 既存動作の継承
- 設定可能な安全性レベル

## 成果物

### 1. リファクタリングレポート（Markdownファイル）
- `tool/fdc_porting/docs/reports/phase22-comprehensive-refactoring-report.md`

### 2. 修正ファイル
- `src/vm/mb8877_compat.cpp` - 包括的安全化
- `src/vm/mb8877_compat.h` - 安全性メソッド追加
- `tool/fdc_porting/test/mock_environment.h` - モック改善

### 3. 標準出力（JSON形式）
```json
{
  "phase": 22,
  "task": "comprehensive_disk_access_refactoring",
  "safety_improvements": {
    "accessor_methods_added": 4,
    "null_checks_added": "count",
    "bounds_checks_added": "count",
    "error_handlers_unified": true
  },
  "test_results": {
    "segfaults_eliminated": "X/3",
    "test_success_rate": "X%",
    "regression_tests": "passed/failed"
  },
  "stability_metrics": {
    "before_completion": "15%",
    "after_completion": "X%",
    "improvement": "+X%"
  },
  "phase23_readiness": "ready/needs_work"
}
```

## 成功基準

### 最低基準
- 3つのセグフォルトテスト中2つ以上が正常実行
- 新しいセグフォルトなし
- 安全性メソッドの基盤実装

### 理想基準
- 全セグフォルトの解消
- テスト成功率50%以上
- 包括的な安全性インフラ完成

## 品質保証

### 1. メモリ安全性
- Valgrindでのメモリリーク検出なし
- AddressSanitizerでの警告なし
- NULL参照の完全排除

### 2. 機能安全性
- 予期しない動作の排除
- エラー状態での適切な処理
- リソースの確実な解放

### 3. 保守性
- コードの可読性向上
- 統一されたエラーハンドリング
- 適切なコメントとドキュメント

## 注意事項
- 大規模なリファクタリングのため、バックアップ必須
- 段階的な修正とテストの繰り返し
- パフォーマンス低下は許容（安定性重視）
- オリジナル動作の完全な理解を前提
- 修正範囲が広いため、優先順位を明確に