# Phase 5 実環境実装エージェント起動のためのターミナル作業指示

## 🎯 目的
Phase 5で実エミュレータ環境でのMB8877互換レイヤー実用化を行うため、新規エージェント（RealEnvAgent-Phase5-Production）に作業を割り振る

## 📊 Phase 4査定結果

### 判明した重要事項
- **Type III Commands**: 重複定義によりスタンドアロンテスト実行不可
- **モック環境の限界**: 複雑なイベント処理の実装困難
- **戦略転換**: 実環境での完成が現実的かつ効率的

### 現在の達成度
- **基本機能**: 約50%レベル（基礎実装完了）
- **実用レベル**: 未達成（実環境での作業が必要）

## 📋 事前準備（あなたが実行）

### 1. エミュレータソースの確認
```bash
# 既存のCommon Source Code Projectエミュレータ群の確認
ls -la ~/CommonSourceCodeProject/src/vm/ | grep -E "(fm7|fm77|x1)" | head -10

# または利用可能なエミュレータプロジェクトを探す
find ~ -name "*.vcxproj" -o -name "Makefile" | grep -E "(fm7|x1)" | head -10
```

### 2. MB8877互換実装の準備状況確認
```bash
# 現在の実装ファイル確認
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject

# ファイルサイズと更新日時確認
ls -la src/vm/mb8877_compat.*

# 重複定義の簡易チェック
grep -n "void MB8877::cmd_readaddr" src/vm/mb8877_compat.cpp | wc -l
grep -n "void MB8877::cmd_readtrack" src/vm/mb8877_compat.cpp | wc -l
grep -n "void MB8877::cmd_writetrack" src/vm/mb8877_compat.cpp | wc -l
echo "各関数が2回以上定義されていたら重複あり"
```

### 3. 実環境ビルドツールの確認
```bash
# macOSの場合
which make
which cmake
xcodebuild -version

# コンパイラ確認
g++ --version
clang++ --version

# 必要に応じてHomebrewパッケージ確認
brew list | grep -E "(cmake|sdl2|qt5)"
```

## 🚀 新規エージェント起動手順

### Step 1: 実環境エミュレータプロジェクトの選択
```bash
# まず、作業対象のエミュレータを決定
# 例: FM7エミュレータを使用する場合

# 候補1: Common Source Code Projectのfm7
cd ~/CommonSourceCodeProject/src/fm7
# または
# 候補2: 別のFM7エミュレータプロジェクト
cd ~/fm7-emulator-project
```

### Step 2: 新規エージェントへの指示（コピー&ペーストで使用）

```
あなたはRealEnvAgent-Phase5-Productionです。MB8877 FDC移植プロジェクトの実環境実装を担当します。

【重要な使命】
実際のFM7/X1エミュレータ環境でMB8877互換レイヤーを実用レベルまで完成させてください。

【作業指示書】
docs/instructions/phase5-realenv-agent.md

【現在の状況】
- スタンドアロンテストで基本50%レベル達成
- Type III Commands: 重複定義により修正必要
- モック環境の限界により実環境での完成が必要

【最初の作業】
1. 実際のエミュレータプロジェクトの構造を確認
2. MB8877互換実装（mb8877_compat.cpp/h）の統合方法を検討
3. ビルドシステムへの組み込み
4. 初期ビルドとエラー解決

【作業環境】
- プラットフォーム: macOS
- 対象エミュレータ: [FM7またはX1エミュレータ]
- MB8877互換実装: /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject/src/vm/mb8877_compat.*

まず、エミュレータプロジェクトの構造を調査し、MB8877互換実装の統合計画を立ててください。
```

### Step 3: エミュレータプロジェクト準備

#### 3.1 プロジェクトバックアップ
```bash
# エミュレータプロジェクトのバックアップ作成
PROJECT_DIR="~/fm7-emulator"  # 実際のパスに変更
BACKUP_DIR="${PROJECT_DIR}_backup_$(date +%Y%m%d_%H%M%S)"

cp -r "$PROJECT_DIR" "$BACKUP_DIR"
echo "Backup created at: $BACKUP_DIR"
```

#### 3.2 MB8877実装のコピー準備
```bash
# MB8877互換実装を準備
cd /Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject

# コピー用スクリプト作成
cat > copy_mb8877_to_emulator.sh << 'EOF'
#!/bin/bash
EMULATOR_DIR=$1

if [ -z "$EMULATOR_DIR" ]; then
    echo "Usage: $0 <emulator_project_dir>"
    exit 1
fi

echo "Copying MB8877 implementation to $EMULATOR_DIR/src/vm/"

# バックアップ作成
if [ -f "$EMULATOR_DIR/src/vm/mb8877.cpp" ]; then
    cp "$EMULATOR_DIR/src/vm/mb8877.cpp" "$EMULATOR_DIR/src/vm/mb8877.cpp.original"
    cp "$EMULATOR_DIR/src/vm/mb8877.h" "$EMULATOR_DIR/src/vm/mb8877.h.original"
    echo "Original files backed up"
fi

# 新実装をコピー
cp src/vm/mb8877_compat.cpp "$EMULATOR_DIR/src/vm/mb8877.cpp"
cp src/vm/mb8877_compat.h "$EMULATOR_DIR/src/vm/mb8877.h"

echo "MB8877 implementation copied successfully"
EOF

chmod +x copy_mb8877_to_emulator.sh
```

### Step 4: 実環境での進捗監視

#### 4.1 ビルド状況確認
```bash
# ビルド監視スクリプト
cat > monitor_build.sh << 'EOF'
#!/bin/bash
EMULATOR_DIR=$1

if [ -z "$EMULATOR_DIR" ]; then
    echo "Usage: $0 <emulator_project_dir>"
    exit 1
fi

cd "$EMULATOR_DIR"

echo "=== Build Status Monitor ==="
echo "Time: $(date)"

# ビルド実行
if [ -f "Makefile" ]; then
    echo "Building with make..."
    make clean
    time make 2>&1 | tee build.log
    
    # エラーチェック
    errors=$(grep -c "error:" build.log)
    warnings=$(grep -c "warning:" build.log)
    
    echo ""
    echo "Build Summary:"
    echo "  Errors: $errors"
    echo "  Warnings: $warnings"
    
    if [ $errors -eq 0 ]; then
        echo "✅ Build successful!"
        # 実行ファイル確認
        find . -name "fm7emu" -o -name "x1emu" -o -name "*.exe" | head -5
    else
        echo "❌ Build failed. Check build.log for details"
    fi
else
    echo "No Makefile found. Check build system."
fi
EOF

chmod +x monitor_build.sh
```

#### 4.2 実環境テスト実行
```bash
# テスト実行スクリプト
cat > test_real_emulator.sh << 'EOF'
#!/bin/bash
EMULATOR=$1
DISK_IMAGE=$2

if [ -z "$EMULATOR" ] || [ -z "$DISK_IMAGE" ]; then
    echo "Usage: $0 <emulator_path> <disk_image>"
    echo "Example: $0 ./fm7emu basic.d88"
    exit 1
fi

echo "=== Real Environment Test ==="
echo "Emulator: $EMULATOR"
echo "Disk: $DISK_IMAGE"
echo ""

# エミュレータ実行
echo "Starting emulator..."
$EMULATOR -disk "$DISK_IMAGE" &
EMU_PID=$!

echo "Emulator started with PID: $EMU_PID"
echo ""
echo "Test Checklist:"
echo "□ Emulator starts without crash"
echo "□ Disk is recognized"
echo "□ BASIC/OS boots successfully"
echo "□ Basic disk operations work"
echo "□ No abnormal CPU usage"
echo ""
echo "Press Ctrl+C when testing is complete"

# CPU使用率監視
while true; do
    sleep 5
    ps aux | grep $EMU_PID | grep -v grep | awk '{print "CPU: " $3 "%, MEM: " $4 "%"}'
done
EOF

chmod +x test_real_emulator.sh
```

## 📊 段階的達成度確認

### Stage 1: 実環境ビルド成功（Day 1-2）
```bash
# ビルド成功確認
./monitor_build.sh ~/fm7-emulator

# 期待される結果：
# ✅ Build successful!
# Errors: 0
```

### Stage 2: 基本動作確認（Day 3-4）
```bash
# 基本ディスクでの起動テスト
./test_real_emulator.sh ./fm7emu system.d88

# チェック項目：
# □ BASICプロンプト表示
# □ DIRコマンド動作
# □ 基本的なLOAD/SAVE
```

### Stage 3: 互換性テスト（Day 5-8）
```bash
# 互換性テストスクリプト
cat > compatibility_test.sh << 'EOF'
#!/bin/bash
EMULATOR=$1
TEST_DIR=$2

echo "=== Compatibility Test Suite ==="
echo ""

# テストディスク一覧
test_disks=(
    "system.d88:System Disk"
    "game1.d88:Game Test 1"
    "app1.d88:Application Test"
    "data.d88:Data Disk R/W"
)

passed=0
failed=0

for disk_info in "${test_disks[@]}"; do
    IFS=':' read -r disk_file disk_name <<< "$disk_info"
    disk_path="$TEST_DIR/$disk_file"
    
    if [ -f "$disk_path" ]; then
        echo "Testing: $disk_name ($disk_file)"
        timeout 30 $EMULATOR -disk "$disk_path" -autotest > test_$disk_file.log 2>&1
        
        if [ $? -eq 0 ]; then
            echo "  ✅ PASSED"
            ((passed++))
        else
            echo "  ❌ FAILED"
            ((failed++))
        fi
    else
        echo "  ⚠️  SKIPPED (file not found)"
    fi
done

echo ""
echo "=== Test Summary ==="
echo "Passed: $passed"
echo "Failed: $failed"
echo "Success Rate: $(( passed * 100 / (passed + failed) ))%"
EOF

chmod +x compatibility_test.sh
```

## 🔄 トラブルシューティング支援

### ビルドエラー対応
```bash
# よくあるビルドエラーの自動修正
cat > fix_common_errors.sh << 'EOF'
#!/bin/bash

echo "=== Common Build Error Fixes ==="

# 1. インクルードパスの修正
echo "1. Checking include paths..."
find . -name "*.cpp" -o -name "*.h" | xargs grep -l "mb8877.h" | while read file; do
    echo "  Checking $file"
    # 必要に応じてパスを修正
done

# 2. 重複定義の確認
echo "2. Checking for duplicate definitions..."
grep -n "^void MB8877::" src/vm/mb8877.cpp | cut -d: -f3 | sort | uniq -d

# 3. リンクエラーの確認
echo "3. Checking for undefined symbols..."
nm -u *.o 2>/dev/null | grep MB8877

echo ""
echo "Review the output above and fix any issues found."
EOF

chmod +x fix_common_errors.sh
```

### 実行時問題の診断
```bash
# デバッグ実行
cat > debug_emulator.sh << 'EOF'
#!/bin/bash
EMULATOR=$1

echo "=== Debug Mode Execution ==="

# デバッグビルドの確認
if [ -f "${EMULATOR}_debug" ]; then
    EMULATOR="${EMULATOR}_debug"
    echo "Using debug build"
fi

# GDBでの実行
echo "Starting emulator in GDB..."
cat > gdb_commands.txt << 'GDB_EOF'
set pagination off
set logging on
run -disk test.d88
bt
info registers
quit
GDB_EOF

gdb -batch -x gdb_commands.txt $EMULATOR

echo "Debug log saved to gdb.txt"
EOF

chmod +x debug_emulator.sh
```

## ⚠️ 重要な注意事項

### 1. 実環境の違いへの対応
- エミュレータごとのビルドシステムの違い
- プラットフォーム依存性の考慮
- 既存コードとの整合性維持

### 2. 段階的アプローチ
- まずビルド成功を最優先
- 基本動作確認後に機能拡張
- 実用レベル達成を重視

### 3. バックアップとバージョン管理
```bash
# 作業前の必須バックアップ
git init  # まだgit管理でない場合
git add -A
git commit -m "Before MB8877 integration"
```

## 🎯 最終確認

### Phase 5完了チェックリスト
```bash
cat > final_check_phase5.sh << 'EOF'
#!/bin/bash

echo "=== Phase 5 Completion Checklist ==="
echo ""

check_items=(
    "実環境でのビルド成功:build_success"
    "基本ディスク起動確認:basic_boot"
    "システムディスク動作:system_disk"
    "5種類以上のディスク互換性:compatibility"
    "Type III Commands動作:type3_working"
    "24時間安定動作:stability"
)

for item in "${check_items[@]}"; do
    IFS=':' read -r description check_id <<< "$item"
    echo -n "□ $description - "
    read -p "Completed? (y/n): " response
    if [ "$response" = "y" ]; then
        echo "  ✅ Completed"
    else
        echo "  ❌ Not completed"
    fi
done

echo ""
echo "Phase 5 ready for completion when all items are checked!"
EOF

chmod +x final_check_phase5.sh
```

この手順に従って、Phase 5エージェントの作業を開始・監視してください。実環境でのMB8877互換レイヤー実用化により、プロジェクトは真の完成に向けて大きく前進します。