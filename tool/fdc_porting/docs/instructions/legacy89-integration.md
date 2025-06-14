# Legacy89DiskKit統合ガイド

## 概要
Legacy89DiskKitをMB8877 FDCエミュレーションテストに活用する方法

## ツールパス
`tool/fdc_porting/tools/Legacy89DiskKit/`

## テストディスク作成例

### 1. Legacy89DiskKitのビルド
```bash
cd tool/fdc_porting/tools/Legacy89DiskKit/CSharp
dotnet build
```

### 2. テスト用D88ディスク作成

#### 基本的な2Dディスク（320KB）
```bash
# Hu-BASIC形式でX1用ディスク作成
dotnet run --project Legacy89DiskKit.CLI -- create ../../../test/data/test_disk_images/test_2d.d88 2D "TEST DISK"
dotnet run --project Legacy89DiskKit.CLI -- format ../../../test/data/test_disk_images/test_2d.d88 --filesystem hu-basic

# テストファイル追加
echo "Hello FDC Test!" > /tmp/test.txt
dotnet run --project Legacy89DiskKit.CLI -- import-text ../../../test/data/test_disk_images/test_2d.d88 /tmp/test.txt README.TXT --filesystem hu-basic --machine x1
```

#### 2DDディスク（720KB）
```bash
# MSX-DOS形式でMSX用ディスク作成
dotnet run --project Legacy89DiskKit.CLI -- create ../../../test/data/test_disk_images/test_2dd.d88 2DD "MSX TEST"
dotnet run --project Legacy89DiskKit.CLI -- format ../../../test/data/test_disk_images/test_2dd.d88 --filesystem msx-dos

# バイナリファイル追加（ロード・実行アドレス付き）
dotnet run --project Legacy89DiskKit.CLI -- import-binary ../../../test/data/test_disk_images/test_2dd.d88 /tmp/test.bin TEST.BIN 0x8000 0x8000
```

#### N88-BASIC形式（PC-8801用）
```bash
# PC-8801用2Dディスク作成
dotnet run --project Legacy89DiskKit.CLI -- create ../../../test/data/test_disk_images/test_n88.d88 2D "PC88 DISK"
dotnet run --project Legacy89DiskKit.CLI -- format ../../../test/data/test_disk_images/test_n88.d88 --filesystem n88-basic

# BASICプログラム追加
dotnet run --project Legacy89DiskKit.CLI -- import-text ../../../test/data/test_disk_images/test_n88.d88 /tmp/hello.bas HELLO.BAS --filesystem n88-basic --machine pc8801
```

### 3. FDCテストでの使用

#### C++テストコードでの読み込み
```cpp
// test_mb8877_type2_commands.cpp に追加
void test_read_sector_with_real_disk() {
    // D88ディスクイメージを読み込む
    fdc.open_disk(0, "../test/data/test_disk_images/test_2d.d88", 0);
    
    // モーターON
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // READ SECTORコマンド実行
    fdc.write_io8(FDC_TRACK, 0);    // トラック0
    fdc.write_io8(FDC_SECTOR, 1);   // セクタ1
    fdc.write_io8(FDC_COMMAND, 0x80); // READ SECTOR
    
    // データ読み取り
    uint8_t buffer[256];
    for(int i = 0; i < 256; i++) {
        while(!(fdc.read_io8(FDC_STATUS) & 0x02)); // DRQ待ち
        buffer[i] = fdc.read_io8(FDC_DATA);
    }
    
    // 検証
    // ...
}
```

## 各種ディスクフォーマットの特徴

### Hu-BASIC (Sharp X1)
- セクタサイズ: 256バイト
- トラック数: 40（2D）、80（2DD）
- セクタ/トラック: 16
- ファイル名: 8.3形式

### N88-BASIC (PC-8801)
- セクタサイズ: 256バイト
- トラック数: 40（2D）、80（2DD）
- セクタ/トラック: 16
- ファイル属性: BAS/BIN/ASC

### MSX-DOS
- セクタサイズ: 512バイト
- トラック数: 80
- セクタ/トラック: 9
- FAT12ベース

## 検証スクリプト作成

### test_disk_generator.sh
```bash
#!/bin/bash
# FDCテスト用ディスクイメージ生成スクリプト

LEGACY89_DIR="tool/fdc_porting/tools/Legacy89DiskKit/CSharp"
TEST_IMG_DIR="tool/fdc_porting/test/data/test_disk_images"

# ディレクトリ作成
mkdir -p $TEST_IMG_DIR

# Legacy89DiskKitビルド
echo "Building Legacy89DiskKit..."
(cd $LEGACY89_DIR && dotnet build)

# 各種テストディスク作成
echo "Creating test disks..."

# 1. 空のディスク
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- create $TEST_IMG_DIR/empty_2d.d88 2D "EMPTY"

# 2. フォーマット済みディスク
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- create $TEST_IMG_DIR/formatted_2d.d88 2D "FORMATTED"
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- format $TEST_IMG_DIR/formatted_2d.d88 --filesystem hu-basic

# 3. ファイル入りディスク
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- create $TEST_IMG_DIR/files_2d.d88 2D "FILES"
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- format $TEST_IMG_DIR/files_2d.d88 --filesystem hu-basic

# テストファイル作成
echo "This is a test file for FDC testing." > /tmp/test1.txt
echo "Second test file with different content." > /tmp/test2.txt
dd if=/dev/urandom of=/tmp/test.bin bs=1024 count=10

# ファイル追加
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- import-text $TEST_IMG_DIR/files_2d.d88 /tmp/test1.txt TEST1.TXT --filesystem hu-basic --machine x1
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- import-text $TEST_IMG_DIR/files_2d.d88 /tmp/test2.txt TEST2.TXT --filesystem hu-basic --machine x1
dotnet run --project $LEGACY89_DIR/Legacy89DiskKit.CLI -- import-binary $TEST_IMG_DIR/files_2d.d88 /tmp/test.bin TEST.BIN

echo "Test disk images created in $TEST_IMG_DIR"
```

## インタラクティブテスト

### Legacy89DiskKitシェルでの操作
```bash
# シェル起動
dotnet run --project tool/fdc_porting/tools/Legacy89DiskKit/CSharp/Legacy89DiskKit.CLI -- shell

# シェル内での操作
Legacy89DiskKit [0:Empty]> open test.d88
Legacy89DiskKit [0:test.d88/HuBasic]> list
Legacy89DiskKit [0:test.d88/HuBasic]> info
Legacy89DiskKit [0:test.d88/HuBasic]> export-text README.TXT /tmp/readme.txt
```

## FDCテストへの適用

1. **基本的な読み取りテスト** - 既知のデータを含むディスクで検証
2. **書き込みテスト** - 空のディスクへの書き込みと再読み込み
3. **フォーマットテスト** - WRITE TRACKコマンドでのトラック作成
4. **エラー処理テスト** - 破損ディスクでの動作確認

## 注意事項

- Legacy89DiskKitは.NET 8.0が必要
- D88形式はSharp X1/PC-8801エミュレータ標準
- 文字エンコーディングは機種により異なる（X1は完全実装）
- テスト時はファイルシステム指定を明示的に行う