# Phase 24: ディスクファイルサポート実装・Type IIコマンド完全対応指示書

## エージェント名
DiskFileSupportAgent-Phase24

## 作業目的
Phase 23で60%まで向上した機能成功率を、Legacy89DiskKitを活用したD88ディスクイメージサポートにより90%以上に引き上げる。Type IIコマンド（READ/WRITE SECTOR）の完全動作を実現する。

## 前提情報
- Phase 23で全体成功率60%達成（レジスタ100%、Type I 100%、Type II 43%）
- Legacy89DiskKitがサブモジュールとして統合済み
- Type IIコマンドのブロッキング要因：ディスクファイルサポート不足
- セグフォルト0、安定性100%維持

## 参照すべきファイル
- `tool/fdc_porting/tools/Legacy89DiskKit/` - D88ディスク操作ツール
- `src/vm/disk.cpp` - DISKクラス実装（D88形式対応）
- `src/vm/disk.h` - DISKクラスヘッダー
- `tool/fdc_porting/test/mock_environment.h` - モック環境
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - Type IIテスト
- `tool/fdc_porting/test/data/test_disk_images/` - テストディスク格納場所

## 実装戦略

### Phase 24.1: Legacy89DiskKit統合準備（2時間）

#### 1.1 ビルド環境セットアップ
```bash
# Legacy89DiskKitのビルド
cd tool/fdc_porting/tools/Legacy89DiskKit/CSharp
dotnet build -c Release

# テストディスク生成スクリプト作成
cd ../../..
chmod +x test/scripts/generate_test_disks.sh
```

#### 1.2 テストディスクイメージ生成
```bash
#!/bin/bash
# generate_test_disks.sh

LEGACY89="dotnet run --project tools/Legacy89DiskKit/CSharp/Legacy89DiskKit.CLI --"
DISK_DIR="test/data/test_disk_images"

mkdir -p $DISK_DIR

# 1. 基本的な2Dディスク（Hu-BASIC、Sharp X1形式）
$LEGACY89 create $DISK_DIR/basic_2d.d88 2D "BASIC 2D"
$LEGACY89 format $DISK_DIR/basic_2d.d88 --filesystem hu-basic

# 2. データ入り2Dディスク
$LEGACY89 create $DISK_DIR/data_2d.d88 2D "DATA DISK"
$LEGACY89 format $DISK_DIR/data_2d.d88 --filesystem hu-basic
echo "Test data for FDC" > /tmp/test.txt
$LEGACY89 import-text $DISK_DIR/data_2d.d88 /tmp/test.txt TEST.TXT --filesystem hu-basic --machine x1

# 3. 2DDディスク（MSX-DOS形式）
$LEGACY89 create $DISK_DIR/msx_2dd.d88 2DD "MSX DISK"
$LEGACY89 format $DISK_DIR/msx_2dd.d88 --filesystem msx-dos

# 4. バイナリファイル入りディスク
$LEGACY89 create $DISK_DIR/binary_2d.d88 2D "BINARY"
$LEGACY89 format $DISK_DIR/binary_2d.d88 --filesystem hu-basic
dd if=/dev/urandom of=/tmp/test.bin bs=256 count=10
$LEGACY89 import-binary $DISK_DIR/binary_2d.d88 /tmp/test.bin TEST.BIN 0x8000 0x8000
```

#### 1.3 ディスクイメージ検証
```cpp
// verify_disk_images.cpp
void verify_disk_image(const char* path) {
    DISK disk;
    disk.open(path, 0);
    
    printf("Disk: %s\n", path);
    printf("  Inserted: %s\n", disk.inserted ? "Yes" : "No");
    printf("  Tracks: %d\n", disk.is_1dd_image ? 80 : 40);
    printf("  Sectors: %d\n", disk.sector_num.sd);
    printf("  Write Protected: %s\n", disk.write_protected ? "Yes" : "No");
    
    disk.close();
}
```

### Phase 24.2: MockDISK実装強化（3時間）

#### 2.1 D88ファイルサポート追加
```cpp
// mock_environment.h の拡張
class DISK {
private:
    std::string disk_file_path;
    bool disk_file_loaded;
    std::vector<uint8_t> disk_image_data;
    
public:
    // D88ファイル読み込み対応
    void open(const _TCHAR* file_path, int bank) {
        disk_file_path = file_path;
        disk_file_loaded = false;
        
        // D88ファイル読み込み実装
        if(load_d88_file(file_path)) {
            disk_file_loaded = true;
            inserted = true;
            parse_d88_header();
            setup_track_sector_info();
        }
    }
    
    bool load_d88_file(const char* path) {
        FILE* fp = fopen(path, "rb");
        if(!fp) return false;
        
        // D88ヘッダー読み込み
        struct D88Header {
            char name[17];
            uint8_t reserved[9];
            uint8_t write_protect;
            uint8_t disk_type;
            uint32_t disk_size;
            uint32_t track_table[164];
        } header;
        
        fread(&header, sizeof(header), 1, fp);
        
        // ディスクイメージ全体読み込み
        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        disk_image_data.resize(size);
        fread(disk_image_data.data(), 1, size, fp);
        fclose(fp);
        
        // ディスク情報設定
        write_protected = header.write_protect != 0;
        media_type = header.disk_type;
        
        return true;
    }
    
    // セクタデータ読み込み
    bool get_sector(int trk, int side, int sect) {
        if(!disk_file_loaded) return false;
        
        // D88フォーマットからセクタ取得
        int track_offset = get_track_offset(trk, side);
        if(track_offset == 0) return false;
        
        // トラック内のセクタ検索
        uint8_t* track_data = disk_image_data.data() + track_offset;
        return find_sector_in_track(track_data, sect);
    }
};
```

#### 2.2 セクタ読み書き実装
```cpp
// DISKクラスの読み書きメソッド強化
class DISK {
public:
    // セクタ読み込み（D88対応）
    bool read_sector_from_file(int track, int side, int sector, uint8_t* buffer) {
        if(!get_sector(track, side, sector)) {
            return false;
        }
        
        // セクタデータをバッファにコピー
        memcpy(buffer, sector, sector_size.sd);
        return true;
    }
    
    // セクタ書き込み（D88対応）
    bool write_sector_to_file(int track, int side, int sector_num, uint8_t* buffer) {
        if(write_protected) {
            return false;
        }
        
        if(!get_sector(track, side, sector_num)) {
            return false;
        }
        
        // セクタデータを更新
        memcpy(sector, buffer, sector_size.sd);
        sector_changed = true;
        
        // D88ファイルに書き戻し
        return sync_to_file();
    }
    
    // ファイル同期
    bool sync_to_file() {
        if(!disk_file_loaded || disk_file_path.empty()) {
            return false;
        }
        
        FILE* fp = fopen(disk_file_path.c_str(), "r+b");
        if(!fp) return false;
        
        fwrite(disk_image_data.data(), 1, disk_image_data.size(), fp);
        fclose(fp);
        
        return true;
    }
};
```

### Phase 24.3: Type IIコマンドテスト強化（4時間）

#### 3.1 READ SECTORテスト改良
```cpp
// test_mb8877_type2_commands.cpp
TEST_CASE("Type II - READ SECTOR with D88 disk", "[type2][d88]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    // テストディスク読み込み
    fdc.open_disk(0, "test/data/test_disk_images/data_2d.d88", 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    SECTION("Single sector read") {
        // トラック0、セクタ1を読む
        fdc.write_io8(FDC_TRACK, 0);
        fdc.write_io8(FDC_SECTOR, 1);
        fdc.write_io8(FDC_COMMAND, 0x80); // READ SECTOR
        
        // コマンド実行待ち
        wait_for_command_completion(&fdc);
        
        // データ読み取り
        std::vector<uint8_t> buffer;
        while(fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY) {
            if(fdc.read_io8(FDC_STATUS) & FDC_ST_DRQ) {
                buffer.push_back(fdc.read_io8(FDC_DATA));
            }
            env.advance_time(10);
        }
        
        CHECK(buffer.size() == 256);
        CHECK((fdc.read_io8(FDC_STATUS) & FDC_ST_CRCERR) == 0);
    }
    
    SECTION("Multi-sector read") {
        fdc.write_io8(FDC_TRACK, 0);
        fdc.write_io8(FDC_SECTOR, 1);
        fdc.write_io8(FDC_COMMAND, 0x90); // READ MULTIPLE
        
        std::vector<uint8_t> buffer;
        int sectors_read = 0;
        
        while(fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY) {
            if(fdc.read_io8(FDC_STATUS) & FDC_ST_DRQ) {
                buffer.push_back(fdc.read_io8(FDC_DATA));
                if(buffer.size() % 256 == 0) {
                    sectors_read++;
                }
            }
            env.advance_time(10);
        }
        
        CHECK(sectors_read >= 2);
    }
}
```

#### 3.2 WRITE SECTORテスト実装
```cpp
TEST_CASE("Type II - WRITE SECTOR with D88 disk", "[type2][d88]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    // 書き込み可能なディスク
    fdc.open_disk(0, "test/data/test_disk_images/empty_2d.d88", 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    SECTION("Single sector write") {
        // テストデータ準備
        std::vector<uint8_t> test_data(256);
        for(int i = 0; i < 256; i++) {
            test_data[i] = i & 0xFF;
        }
        
        // セクタ書き込み
        fdc.write_io8(FDC_TRACK, 0);
        fdc.write_io8(FDC_SECTOR, 2);
        fdc.write_io8(FDC_COMMAND, 0xA0); // WRITE SECTOR
        
        // DRQ待ちしてデータ書き込み
        int written = 0;
        while(written < 256 && (fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY)) {
            if(fdc.read_io8(FDC_STATUS) & FDC_ST_DRQ) {
                fdc.write_io8(FDC_DATA, test_data[written++]);
            }
            env.advance_time(10);
        }
        
        CHECK(written == 256);
        CHECK((fdc.read_io8(FDC_STATUS) & FDC_ST_WRITEFAULT) == 0);
        
        // 読み戻して検証
        fdc.write_io8(FDC_COMMAND, 0x80); // READ SECTOR
        std::vector<uint8_t> read_back = read_sector(&fdc, &env);
        
        CHECK(read_back == test_data);
    }
    
    SECTION("Write protected disk") {
        // 書き込み保護ディスク
        fdc.open_disk(0, "test/data/test_disk_images/protected_2d.d88", 0);
        
        fdc.write_io8(FDC_COMMAND, 0xA0); // WRITE SECTOR
        wait_for_command_completion(&fdc, &env);
        
        CHECK(fdc.read_io8(FDC_STATUS) & FDC_ST_WRITEP);
    }
}
```

### Phase 24.4: 統合テスト実装（3時間）

#### 4.1 実際のディスク操作シナリオ
```cpp
TEST_CASE("Real disk operation scenarios", "[integration][d88]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    SECTION("Format and verify track") {
        fdc.open_disk(0, "test/data/test_disk_images/format_test.d88", 0);
        fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
        
        // WRITE TRACK (format)
        fdc.write_io8(FDC_TRACK, 5);
        fdc.write_io8(FDC_COMMAND, 0xF0); // WRITE TRACK
        
        // フォーマットデータ送信
        std::vector<uint8_t> format_data = generate_format_data();
        send_format_data(&fdc, &env, format_data);
        
        // 検証読み込み
        bool verify_ok = verify_formatted_track(&fdc, &env, 5);
        CHECK(verify_ok);
    }
    
    SECTION("Cross-track operations") {
        // トラックをまたぐ操作
        fdc.open_disk(0, "test/data/test_disk_images/multi_track.d88", 0);
        
        // トラック0の最終セクタから開始
        fdc.write_io8(FDC_TRACK, 0);
        fdc.write_io8(FDC_SECTOR, 16);
        fdc.write_io8(FDC_COMMAND, 0x90); // READ MULTIPLE
        
        // 自動的にトラック1に移行することを確認
        int sectors_read = read_multiple_sectors(&fdc, &env);
        CHECK(sectors_read > 16); // トラック境界を超えた
        CHECK(fdc.read_io8(FDC_TRACK) == 1);
    }
}
```

#### 4.2 エラー処理テスト
```cpp
TEST_CASE("Error handling with real disks", "[error][d88]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    SECTION("CRC error simulation") {
        // CRCエラーを含むディスク
        fdc.open_disk(0, "test/data/test_disk_images/crc_error.d88", 0);
        
        fdc.write_io8(FDC_TRACK, 1);
        fdc.write_io8(FDC_SECTOR, 5); // CRCエラーセクタ
        fdc.write_io8(FDC_COMMAND, 0x80);
        
        wait_for_command_completion(&fdc, &env);
        CHECK(fdc.read_io8(FDC_STATUS) & FDC_ST_CRCERR);
    }
    
    SECTION("Sector not found") {
        fdc.open_disk(0, "test/data/test_disk_images/basic_2d.d88", 0);
        
        fdc.write_io8(FDC_SECTOR, 99); // 存在しないセクタ
        fdc.write_io8(FDC_COMMAND, 0x80);
        
        wait_for_command_completion(&fdc, &env);
        CHECK(fdc.read_io8(FDC_STATUS) & FDC_ST_RECNFND);
    }
}
```

### Phase 24.5: Type III/IVコマンド基礎実装（2時間）

#### 5.1 READ ADDRESSコマンド
```cpp
TEST_CASE("Type III - READ ADDRESS", "[type3][d88]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    fdc.open_disk(0, "test/data/test_disk_images/data_2d.d88", 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // 現在位置のIDフィールド読み取り
    fdc.write_io8(FDC_COMMAND, 0xC0); // READ ADDRESS
    
    std::vector<uint8_t> id_data;
    while(fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY) {
        if(fdc.read_io8(FDC_STATUS) & FDC_ST_DRQ) {
            id_data.push_back(fdc.read_io8(FDC_DATA));
        }
        env.advance_time(10);
    }
    
    CHECK(id_data.size() == 6); // Track, Side, Sector, Size, CRC1, CRC2
    CHECK(id_data[0] == 0);     // Track 0
    CHECK(id_data[2] >= 1);     // Valid sector number
}
```

#### 5.2 FORCE INTERRUPTコマンド
```cpp
TEST_CASE("Type IV - FORCE INTERRUPT", "[type4]") {
    MockEnvironment env;
    MB8877 fdc(&env);
    
    // 長時間コマンド開始
    fdc.write_io8(FDC_COMMAND, 0xE0); // READ TRACK
    
    // 実行中に割り込み
    env.advance_time(1000);
    CHECK(fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY);
    
    fdc.write_io8(FDC_COMMAND, 0xD0); // FORCE INTERRUPT
    
    // 即座に停止
    env.advance_time(10);
    CHECK(!(fdc.read_io8(FDC_STATUS) & FDC_ST_BUSY));
}
```

## テスト戦略

### Phase 24.6: 包括的検証（2時間）

#### 6.1 全テスト実行
```bash
# Legacy89DiskKitでテストディスク生成
cd tool/fdc_porting
./test/scripts/generate_test_disks.sh

# テストビルドと実行
cd test
make clean && make all

# 個別テスト実行
./test_mb8877_type2_commands -v
./test_mb8877_type3_commands -v
```

#### 6.2 パフォーマンステスト
```cpp
TEST_CASE("Performance with real disk operations", "[performance]") {
    // 1トラック読み込み時間測定
    auto start = std::chrono::high_resolution_clock::now();
    read_entire_track(&fdc, &env, 0);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    CHECK(duration.count() < 200000); // 200ms以内
}
```

## 成果物

### 1. 実装レポート（Markdownファイル）
- `tool/fdc_porting/docs/reports/phase24-disk-file-support-report.md`

### 2. 更新ファイル
- `tool/fdc_porting/test/mock_environment.h` - D88対応MockDISK
- `tool/fdc_porting/test/test_mb8877_type2_commands.cpp` - 完全なType IIテスト
- `tool/fdc_porting/test/test_mb8877_type3_commands.cpp` - Type IIIテスト追加
- `tool/fdc_porting/test/scripts/generate_test_disks.sh` - ディスク生成スクリプト

### 3. 標準出力（JSON形式）
```json
{
  "phase": 24,
  "task": "disk_file_support_implementation",
  "legacy89_integration": {
    "status": "completed",
    "test_disks_created": [
      "basic_2d.d88",
      "data_2d.d88",
      "msx_2dd.d88",
      "binary_2d.d88"
    ],
    "mock_disk_enhanced": true
  },
  "test_improvements": {
    "type2_commands": {
      "before": "6/14 PASS (43%)",
      "after": "X/14 PASS (X%)",
      "new_tests_added": [
        "d88_single_sector_read",
        "d88_multi_sector_read",
        "d88_sector_write",
        "d88_write_protected"
      ]
    },
    "type3_commands": {
      "implemented": ["READ_ADDRESS"],
      "success_rate": "X%"
    },
    "type4_commands": {
      "implemented": ["FORCE_INTERRUPT"],
      "success_rate": "X%"
    }
  },
  "overall_metrics": {
    "functional_success_rate": {
      "before": "60%",
      "after": "X%",
      "target_achieved": "90%+"
    },
    "test_coverage": {
      "commands_covered": "X/16",
      "scenarios_tested": "X"
    }
  },
  "stability_maintained": {
    "segfaults": 0,
    "memory_leaks": 0
  },
  "phase25_readiness": {
    "status": "ready",
    "remaining_items": [
      "advanced_error_scenarios",
      "performance_optimization",
      "android_compatibility"
    ]
  }
}
```

## 成功基準

### 最低基準
- Type IIコマンド成功率 43% → 70%以上
- 全体成功率 60% → 80%以上
- D88ディスク読み込み成功

### 目標基準
- Type IIコマンド成功率 43% → 90%以上
- 全体成功率 60% → 90%以上
- Type III/IVコマンド基本動作

### 理想基準
- 全コマンドタイプ動作確認
- 全体成功率 95%以上
- 実用レベルの完成度

## 品質保証

### 1. ディスク互換性
- 標準的なD88形式サポート
- 各種ファイルシステム対応
- エラー処理の適切性

### 2. テスト網羅性
- 正常系・異常系の両方
- 境界値テスト
- ストレステスト

### 3. パフォーマンス
- リアルタイム性の確保
- メモリ効率
- ファイルI/O最適化

## 注意事項
- Legacy89DiskKitは.NET 8.0必須
- D88ファイルの互換性確認
- ファイルパスのクロスプラットフォーム対応
- テストディスクのバックアップ推奨
- 実機動作との互換性重視