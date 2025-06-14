# Phase 19: コンパイルエラー修正エージェント指示書

## エージェント名
CompileFixAgent-Phase19

## 作業目的
Phase 18で追加したsearch_sector()メソッドで使用している未定義の定数を修正し、コンパイルエラーを解決する

## 問題の詳細
- エラー: `use of undeclared identifier 'FDC_CMD_WR_SEC'`
- エラー: `use of undeclared identifier 'FDC_CMD_WR_MSEC'`
- 発生箇所: mb8877_compat.cpp 1563行目、1630行目
- 原因: コマンドタイプ定数が定義されていない

## 参照すべきファイル
- `src/vm/mb8877_compat.cpp` - エラー発生箇所
- `src/vm/mb8877_compat.h` - 定数定義の追加場所
- `src/vm/mb8877.cpp` - オリジナルの定数定義参考

## 修正項目

### 1. コマンドタイプ定数の定義
mb8877_compat.cppまたはmb8877_compat.hに以下の定数を追加：
```cpp
// Command types
#define TYPE_I    0
#define TYPE_II   1
#define TYPE_III  2
#define TYPE_IV   3

// Type II Commands
#define FDC_CMD_RD_SEC   0x80
#define FDC_CMD_RD_MSEC  0x90
#define FDC_CMD_WR_SEC   0xA0
#define FDC_CMD_WR_MSEC  0xB0
```

### 2. cmdtypeとの比較ロジック修正
現在のコード：
```cpp
if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC)
```

問題点：
- cmdtypeはコマンドタイプ（TYPE_I〜TYPE_IV）
- FDC_CMD_WR_SECはコマンドコード（0xA0）
- 比較の論理が不適切

修正方法：
1. cmdreg（コマンドレジスタ）との比較に変更
2. またはWRITE_SECTORメインステートとの比較
3. cmdtypeがTYPE_IIで、かつwrite系コマンドかの判定

### 3. 適切な判定方法の実装
オプション1：
```cpp
if(main_state == WRITE_SECTOR)
```

オプション2：
```cpp
if(cmdtype == TYPE_II && (cmdreg & 0x20))
```

オプション3：
```cpp
if((cmdreg & 0xF0) == 0xA0 || (cmdreg & 0xF0) == 0xB0)
```

## 修正手順

1. mb8877_compat.hまたは.cppに必要な定数を定義
2. search_sector()内の判定ロジックを修正
3. 同様のエラーが他にないか確認
4. コンパイルテスト実行

## テスト項目

### 1. コンパイル確認
- エラーが解消されること
- 警告のみの状態になること

### 2. ロジック確認
- 書き込みコマンドの判定が正しく動作
- 読み込みコマンドとの区別が適切

## 成果物

1. 修正ファイル:
   - `src/vm/mb8877_compat.cpp` - 判定ロジック修正
   - `src/vm/mb8877_compat.h` - 必要に応じて定数追加

2. レポートファイル:
   - `tool/fdc_porting/docs/reports/phase19-compile-fix-report.md`

## レポート記載事項
- エラーの原因詳細
- 実施した修正内容
- 定数定義の追加内容
- コンパイル結果
- 今後の予防策

## 注意事項
- オリジナルのロジックとの互換性維持
- 他の場所で同様のエラーがないか確認
- 定数の重複定義を避ける
- コマンド判定の正確性を確保