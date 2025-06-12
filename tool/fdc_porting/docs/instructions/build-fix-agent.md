# BuildFixAgent 指示書

## エージェント情報
- エージェント名: BuildFixAgent-TestEnv
- 役割: テスト環境のビルドエラー修正
- 作成日時: 2025/06/11
- 作成者: PMエージェント
- 優先度: 緊急

## 背景と問題

ユーザーがテスト環境のビルドを実行した際、以下のコンパイルエラーが発生しました：

```bash
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject/test
make clean
make
```

### 発生したエラー
```
test_mb8877_registers.cpp:10:1: error: unknown type name 'VM_TEMPLATE'
test_mb8877_registers.cpp:11:1: error: unknown type name 'VM_TEMPLATE'
src/vm/mb8877_compat.cpp:449:26: error: no member named 'emu' in 'EMU'
        seekend_clock = emu->emu->get_current_clock();
                        ~~~~  ^
src/vm/mb8877_compat.cpp:522:31: error: no member named 'emu' in 'EMU'
        seekend_clock = emu->emu->get_current_clock();
                        ~~~~  ^
```

## 実行する作業

### 1. エラー分析と原因特定
- VM_TEMPLATE型定義の不在
- EMUクラスの構造問題（emu->emu アクセス）
- テスト環境とメインコードの型定義不整合

### 2. 修正作業
#### A. VM_TEMPLATE型定義の修正
- `test/mock_environment.h`または`test/test_framework.h`でVM_TEMPLATE型を定義
- 必要に応じて`typedef`を追加

#### B. EMUクラスアクセスの修正
- `src/vm/mb8877_compat.cpp`内の`emu->emu->get_current_clock()`を`emu->get_current_clock()`に修正
- 他の同様のアクセスパターンをチェックして修正

#### C. テスト環境の型定義整備
- テスト用ヘッダーファイルで不足している型定義を追加
- STANDALONE_TESTマクロを適切に活用

### 3. ビルドテストの実行
- 修正後にmakeコマンドでビルドテスト
- エラーが解消されることを確認
- 可能であれば基本的なテスト実行も確認

### 4. 追加の問題への対応
- 他の潜在的なビルドエラーの発見と修正
- プラットフォーム固有の問題があれば対応
- インクルードパスやライブラリの問題があれば修正

## 期待される成果物

### 主要成果物
1. **修正されたソースファイル**
   - `src/vm/mb8877_compat.cpp` (EMUアクセス修正)
   - `test/mock_environment.h` (型定義追加)
   - `test/test_framework.h` (必要に応じて)
   - その他修正が必要なファイル

2. **ビルド成功の確認**
   - `make clean && make`が成功すること
   - コンパイルエラーの完全解消

3. **テスト実行可能性の確認**
   - `./run_all_tests`が実行可能であること（可能であれば）

### レポート
4. **build-fix-results.md**
   - 発見したエラーの詳細分析
   - 実施した修正内容
   - ビルド結果とテスト結果
   - 残存する問題（あれば）
   - 今後の推奨事項

## 技術的な注意事項

### 1. 型定義の扱い
- VM_TEMPLATEが何を表すかを特定して適切な型を定義
- テスト環境では簡略化された定義でも可

### 2. EMUクラスの構造
- オリジナルのmb8877.cppでのEMUアクセスパターンを参考
- テスト環境のMockEMUとの整合性を確保

### 3. STANDALONE_TESTマクロ
- テスト専用コードと実際のエミュレータコードを適切に分離
- 条件付きコンパイルを活用

### 4. プラットフォーム互換性
- macOS/Unix環境での動作を前提
- 必要に応じてWindows互換コードも維持

## 完了条件

1. すべてのコンパイルエラーが解消されている
2. `make clean && make`が成功する
3. 生成された実行ファイルが正常に起動する
4. レポートファイルが適切に作成されている

## 緊急度と影響

- **緊急度**: 最高（プロジェクト進行の阻害要因）
- **影響度**: 高（テスト環境全体の動作不能）
- **推定作業時間**: 1-2時間
- **依存関係**: Phase 2完了とPhase 3開始の間の重要なマイルストーン

## 注意事項

- 既存のテスト実装を破壊しないよう注意
- 修正は最小限に留めて影響範囲を限定
- バックアップやコメントで修正履歴を明確に
- ユーザーの環境（macOS）での動作を最優先

この指示書に基づいて、迅速かつ確実にビルドエラーを解消し、テスト環境を正常に動作させてください。