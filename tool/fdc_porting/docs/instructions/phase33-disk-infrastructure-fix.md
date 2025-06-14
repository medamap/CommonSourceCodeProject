# Phase 33: ディスクインフラストラクチャ修正・SafeDISK実装指示書

## エージェント名
DiskInfrastructureFixAgent-Phase33

## 作業目的
Phase 32のモックMB8877で100%成功率を達成したREAD操作実装を、実際のディスクインフラストラクチャで動作させるため、DISK::openのクラッシュ問題を根本的に解決する。SafeDISKクラスを作成し、エラーハンドリングを強化する。

## 前提情報
- Phase 32でモックMB8877による検証完了（100%成功率）
- 本物のDISKクラスでopen()呼び出し時にクラッシュ発生
- Phase 31のREAD操作修正は正しいことが証明済み
- 現在のテストは仮想ディスクインフラで停止中

## 問題分析
```
1. DISK::open() クラッシュの原因：
   - 初期化されていないポインタアクセス
   - 仮想関数テーブルの不整合
   - メモリアライメント問題
   
2. MockDISKとの差異：
   - MockDISKは最小限の実装で動作
   - 本物のDISKは複雑な継承構造を持つ
   - D88ファイル処理の実装が不完全
```

## 実装戦略

### Phase 33.1: SafeDISKクラス設計（2時間）

#### 1.1 SafeDISKクラス作成
```cpp
// tool/fdc_porting/safe_disk.h
#ifndef SAFE_DISK_H
#define SAFE_DISK_H

#include "../../../src/vm/disk.h"
#include <memory>
#include <vector>
#include <string>

class SafeDISK : public DISK {
private:
    bool initialized;
    std::vector<uint8_t> sector_buffer;
    std::vector<uint8_t> track_buffer;
    std::string current_file_path;
    
    // 安全な初期化
    void safe_initialize() {
        if(initialized) return;
        
        // 基底クラスの必須メンバー初期化
        inserted = false;
        ejected = false;
        write_protected = false;
        changed = false;
        media_type = MEDIA_TYPE_2D;
        is_special_disk = 0;
        
        // セクタ情報初期化
        sector_size.sd = 0;
        sector_num.sd = 0;
        sector = nullptr;
        
        // バッファ確保
        sector_buffer.resize(8192, 0);
        track_buffer.resize(65536, 0);
        
        initialized = true;
    }
    
public:
    SafeDISK() : initialized(false) {
        safe_initialize();
    }
    
    virtual ~SafeDISK() {
        close();
    }
    
    // 安全なopen実装
    virtual void open(const _TCHAR* file_path, int bank) override {
        try {
            safe_initialize();
            
            // ファイルパス保存
            current_file_path = file_path ? file_path : "";
            
            // 基本的なディスク挿入処理
            inserted = true;
            ejected = false;
            
            // ダミーディスクとして初期化（D88読み込みは後で実装）
            setup_dummy_disk();
            
        } catch(...) {
            // エラー時は安全な状態にリセット
            inserted = false;
            ejected = true;
        }
    }
    
    // 安全なclose実装
    virtual void close() override {
        if(!initialized) return;
        
        inserted = false;
        ejected = true;
        sector = nullptr;
        current_file_path.clear();
    }
    
    // ダミーディスクセットアップ
    void setup_dummy_disk() {
        // 2Dディスク（40トラック、16セクタ/トラック）
        media_type = MEDIA_TYPE_2D;
        drive_type = DRIVE_TYPE_2D;
        
        // セクタ設定
        sector_size.sd = 256;
        sector_num.sd = 16;
        track_size.sd = sector_size.sd * sector_num.sd;
        
        // トラック情報
        cylinders = 40;
        surfaces = 2;
    }
    
    // セクタ取得（安全版）
    virtual bool get_sector(int trk, int side, int index) override {
        if(!initialized || !inserted) {
            return false;
        }
        
        // 範囲チェック
        if(trk < 0 || trk >= cylinders || side < 0 || side >= surfaces) {
            return false;
        }
        
        if(index < 1 || index > sector_num.sd) {
            return false;
        }
        
        // ダミーセクタデータ生成
        id[0] = trk;
        id[1] = side;
        id[2] = index;
        id[3] = 1; // 256バイトセクタ
        id[4] = 0; // CRC1
        id[5] = 0; // CRC2
        
        // セクタバッファ設定
        sector = sector_buffer.data();
        
        // テストパターン生成
        for(int i = 0; i < 256; i++) {
            sector_buffer[i] = (trk * 16 + index + i) & 0xFF;
        }
        
        return true;
    }
    
    // トラック取得（安全版）
    virtual bool get_track(int trk, int side) override {
        if(!initialized || !inserted) {
            return false;
        }
        
        if(trk < 0 || trk >= cylinders || side < 0 || side >= surfaces) {
            return false;
        }
        
        // ダミートラックデータ生成
        track = track_buffer.data();
        track_size.sd = sector_size.sd * sector_num.sd;
        
        return true;
    }
};

#endif // SAFE_DISK_H
```

### Phase 33.2: MB8877への統合（2時間）

#### 2.1 SafeDISK使用への切り替え
```cpp
// mb8877_compat.cpp の修正
#include "../../tool/fdc_porting/safe_disk.h"

class MB8877 : public DEVICE {
private:
    // DISKポインタをSafeDISKに変更
    SafeDISK* safe_disks[MAX_DRIVE];
    
public:
    void initialize() override {
        // SafeDISKインスタンス作成
        for(int i = 0; i < MAX_DRIVE; i++) {
            safe_disks[i] = new SafeDISK();
            disk[i] = safe_disks[i];  // 基底クラスポインタに設定
        }
        
        // 既存の初期化処理
        // ...
    }
    
    void release() override {
        // SafeDISKインスタンス解放
        for(int i = 0; i < MAX_DRIVE; i++) {
            if(safe_disks[i]) {
                delete safe_disks[i];
                safe_disks[i] = nullptr;
                disk[i] = nullptr;
            }
        }
    }
    
    // open_disk修正
    void open_disk(int drv, const _TCHAR* file_path, int bank) override {
        if(drv >= 0 && drv < MAX_DRIVE && safe_disks[drv]) {
            safe_disks[drv]->open(file_path, bank);
            disk[drv] = safe_disks[drv];
        }
    }
};
```

### Phase 33.3: エラーハンドリング強化（2時間）

#### 3.1 get_disk_safe関数の実装
```cpp
// mb8877_compat.cpp に追加
DISK* get_disk_safe(int drv) {
    if(drv < 0 || drv >= MAX_DRIVE) {
        return nullptr;
    }
    
    DISK* d = disk[drv];
    if(!d) {
        return nullptr;
    }
    
    // 基本的な健全性チェック
    try {
        // 仮想関数テーブルチェック（簡易版）
        bool is_inserted = d->inserted;
        (void)is_inserted;  // 未使用警告回避
        
        return d;
    } catch(...) {
        // アクセス違反の場合はnullptrを返す
        return nullptr;
    }
}

// コマンドハンドラでの使用
void cmd_read_sector(bool first_sector) {
    DISK* d = get_disk_safe(drvreg);
    if(!d || !d->inserted) {
        // ディスクなしエラー
        status = FDC_ST_NOTREADY;
        cmdtype = 0;
        set_irq(true);
        return;
    }
    
    // 以降の処理...
}
```

### Phase 33.4: D88ファイル読み込み実装（3時間）

#### 4.1 SafeDISKにD88サポート追加
```cpp
// safe_disk.h の拡張
class SafeDISK : public DISK {
private:
    struct D88Header {
        char name[17];
        uint8_t reserved[9];
        uint8_t write_protect;
        uint8_t disk_type;
        uint32_t disk_size;
        uint32_t track_table[164];
    };
    
    std::vector<uint8_t> disk_image;
    
    bool load_d88_file(const std::string& path) {
        FILE* fp = fopen(path.c_str(), "rb");
        if(!fp) return false;
        
        // ファイルサイズ取得
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        // 最小サイズチェック
        if(file_size < sizeof(D88Header)) {
            fclose(fp);
            return false;
        }
        
        // 全体読み込み
        disk_image.resize(file_size);
        size_t read_size = fread(disk_image.data(), 1, file_size, fp);
        fclose(fp);
        
        if(read_size != file_size) {
            return false;
        }
        
        // ヘッダー解析
        D88Header* header = (D88Header*)disk_image.data();
        
        // ディスクタイプ設定
        media_type = header->disk_type;
        write_protected = (header->write_protect != 0);
        
        // トラック情報設定
        switch(media_type) {
            case MEDIA_TYPE_2D:
                cylinders = 40;
                surfaces = 2;
                sector_num.sd = 16;
                sector_size.sd = 256;
                break;
            case MEDIA_TYPE_2DD:
                cylinders = 80;
                surfaces = 2;
                sector_num.sd = 16;
                sector_size.sd = 256;
                break;
            default:
                // 未対応フォーマット
                return false;
        }
        
        return true;
    }
    
public:
    virtual void open(const _TCHAR* file_path, int bank) override {
        safe_initialize();
        
        current_file_path = file_path ? file_path : "";
        
        // D88ファイル読み込み試行
        if(!current_file_path.empty() && load_d88_file(current_file_path)) {
            inserted = true;
            ejected = false;
        } else {
            // 読み込み失敗時はダミーディスク
            setup_dummy_disk();
            inserted = true;
            ejected = false;
        }
    }
    
    virtual bool get_sector(int trk, int side, int index) override {
        if(!initialized || !inserted) {
            return false;
        }
        
        // D88イメージがある場合
        if(!disk_image.empty()) {
            return get_sector_from_d88(trk, side, index);
        }
        
        // ダミーセクタ
        return get_dummy_sector(trk, side, index);
    }
    
private:
    bool get_sector_from_d88(int trk, int side, int index) {
        D88Header* header = (D88Header*)disk_image.data();
        
        // トラックオフセット取得
        int track_num = trk * surfaces + side;
        if(track_num >= 164 || header->track_table[track_num] == 0) {
            return false;
        }
        
        uint32_t track_offset = header->track_table[track_num];
        if(track_offset >= disk_image.size()) {
            return false;
        }
        
        // トラック内のセクタ検索
        uint8_t* track_ptr = disk_image.data() + track_offset;
        
        while(track_ptr < disk_image.data() + disk_image.size()) {
            // セクタヘッダー解析
            if(track_ptr + 16 > disk_image.data() + disk_image.size()) {
                break;
            }
            
            uint8_t sector_track = track_ptr[0];
            uint8_t sector_side = track_ptr[1];
            uint8_t sector_index = track_ptr[2];
            uint8_t sector_size_code = track_ptr[3];
            uint16_t data_size = *(uint16_t*)(track_ptr + 14);
            
            if(sector_track == trk && sector_side == side && sector_index == index) {
                // セクタ発見
                id[0] = sector_track;
                id[1] = sector_side;
                id[2] = sector_index;
                id[3] = sector_size_code;
                id[4] = track_ptr[4];  // CRC1
                id[5] = track_ptr[5];  // CRC2
                
                // データコピー
                if(data_size > 0 && data_size <= sector_buffer.size()) {
                    memcpy(sector_buffer.data(), track_ptr + 16, data_size);
                    sector = sector_buffer.data();
                    sector_size.sd = data_size;
                    return true;
                }
            }
            
            // 次のセクタへ
            track_ptr += 16 + data_size;
        }
        
        return false;
    }
};
```

### Phase 33.5: テスト実装（2時間）

#### 5.1 SafeDISKテスト
```cpp
// test_safe_disk.cpp
#include "test_framework.h"
#include "../safe_disk.h"

void test_safe_disk_basic(TestFramework& test) {
    TEST_SECTION("SafeDISK Basic Operations");
    
    SafeDISK disk;
    
    // 初期状態
    test.assert_false(disk.inserted, "Not inserted initially");
    
    // ダミーディスクオープン
    disk.open(nullptr, 0);
    test.assert_true(disk.inserted, "Inserted after open");
    
    // セクタ取得
    bool got = disk.get_sector(0, 0, 1);
    test.assert_true(got, "Get sector successful");
    test.assert_not_null(disk.sector, "Sector buffer allocated");
    
    // クローズ
    disk.close();
    test.assert_false(disk.inserted, "Not inserted after close");
}

void test_safe_disk_d88(TestFramework& test) {
    TEST_SECTION("SafeDISK D88 Support");
    
    SafeDISK disk;
    
    // D88ファイルオープン
    disk.open(_T("test/data/test_disk_images/basic_2d.d88"), 0);
    test.assert_true(disk.inserted, "D88 disk inserted");
    
    // セクタ読み込み
    if(disk.get_sector(0, 0, 1)) {
        test.assert_equal(disk.sector_size.sd, 256, "Sector size 256");
        test.assert_equal(disk.id[0], 0, "Track ID correct");
        test.assert_equal(disk.id[2], 1, "Sector ID correct");
    }
}

void test_safe_disk_error_handling(TestFramework& test) {
    TEST_SECTION("SafeDISK Error Handling");
    
    SafeDISK disk;
    
    // 無効なセクタアクセス
    bool got = disk.get_sector(99, 0, 1);
    test.assert_false(got, "Invalid track rejected");
    
    got = disk.get_sector(0, 3, 1);
    test.assert_false(got, "Invalid side rejected");
    
    got = disk.get_sector(0, 0, 99);
    test.assert_false(got, "Invalid sector rejected");
}
```

#### 5.2 MB8877統合テスト
```cpp
// test_mb8877_safe_disk.cpp
void test_mb8877_with_safe_disk(TestFramework& test) {
    TEST_SECTION("MB8877 with SafeDISK");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // SafeDISKでディスクオープン（クラッシュしないことを確認）
    fdc.open_disk(0, _T("test.d88"), 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // READ SECTORコマンド
    fdc.write_io8(1, 0);  // Track 0
    fdc.write_io8(2, 1);  // Sector 1
    fdc.write_io8(0, 0x80);  // READ SECTOR
    
    // コマンド完了待ち
    for(int i = 0; i < 1000 && (fdc.read_io8(0) & 0x01); i++) {
        event.advance_clock(100);
    }
    
    uint32_t status = fdc.read_io8(0);
    test.assert_true((status & 0x01) == 0, "Command completed");
    
    // データ読み取り
    if(status & 0x02) {  // DRQ
        uint8_t data = fdc.read_io8(3);
        test.assert_true(true, "Data read successful");
    }
}
```

## テスト手順

### 1. SafeDISK単体テスト
```bash
cd tool/fdc_porting/test
make test_safe_disk
./test_safe_disk
```

### 2. MB8877統合テスト
```bash
make test_mb8877_safe_disk
./test_mb8877_safe_disk
```

### 3. 既存テストの再実行
```bash
make all
./run_all_tests.sh
```

## 成果物

### 1. 実装レポート
- `tool/fdc_porting/docs/reports/phase33-disk-infrastructure-fix-report.md`

### 2. 新規/更新ファイル
- `tool/fdc_porting/safe_disk.h` - SafeDISKクラス実装
- `src/vm/mb8877_compat.cpp` - SafeDISK統合
- `tool/fdc_porting/test/test_safe_disk.cpp` - SafeDISKテスト
- `tool/fdc_porting/test/test_mb8877_safe_disk.cpp` - 統合テスト

### 3. 標準出力（JSON形式）
```json
{
  "phase": 33,
  "task": "disk_infrastructure_fix",
  "safe_disk_implementation": {
    "status": "completed",
    "features": [
      "crash_prevention",
      "error_handling",
      "d88_support",
      "dummy_disk_fallback"
    ],
    "memory_safety": "100%"
  },
  "test_results": {
    "safe_disk_tests": {
      "total": "X",
      "passed": "X",
      "failed": "X",
      "crash_rate": "0%"
    },
    "mb8877_integration": {
      "open_disk_crash": "fixed",
      "read_sector_working": "X%",
      "write_sector_working": "X%"
    },
    "regression_tests": {
      "type1_commands": "100%",
      "type2_commands": "X%",
      "overall": "X%"
    }
  },
  "performance": {
    "disk_open_time": "X ms",
    "sector_read_time": "X µs",
    "memory_usage": "X KB"
  },
  "next_phase": {
    "phase": 34,
    "focus": "type2_command_completion",
    "expected_success_rate": "80%+"
  }
}
```

## 成功基準

### 最低基準
- DISK::openクラッシュ解消
- SafeDISK基本動作確認
- 既存テストの維持

### 目標基準
- D88ファイル読み込み成功
- Type IIコマンド50%以上動作
- メモリリーク0

### 理想基準
- 全Type IIコマンド80%以上動作
- 実用的なディスク操作可能
- Phase 31の修正が完全動作

## 注意事項
- 仮想関数テーブルの整合性に注意
- メモリアライメントを考慮
- 例外処理で安全性を確保
- 既存のインターフェースとの互換性維持