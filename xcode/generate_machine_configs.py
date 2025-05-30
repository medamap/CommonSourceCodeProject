#!/usr/bin/env python3
"""
CMake機種別設定ファイル自動生成スクリプト
CommonSourceCodeProject Xcode版

Author: Medamap and Claude
Date: 2025.01.29
"""

import os
import sys
import re
from pathlib import Path

# 基本パス設定
SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
SRC_DIR = PROJECT_ROOT / "src"
VM_DIR = SRC_DIR / "vm"
MENU_DIR = SRC_DIR / "menu"
MACHINES_DIR = SCRIPT_DIR / "Machines"

# 共通ファイルリスト
COMMON_HEADERS = [
    "${SRC_DIR}/common.h",
    "${SRC_DIR}/config.h",
    "${SRC_DIR}/emu.h",
    "${SRC_DIR}/fifo.h",
    "${SRC_DIR}/fileio.h",
    "${SRC_DIR}/res/resource.h",
]

COMMON_SOURCES = [
    "${SRC_DIR}/Xcode/xcode_mainloop.cpp",
    "${SRC_DIR}/Xcode/xcode_main.mm",
    "${SRC_DIR}/Xcode/Bridge.mm",
    "${HEADER}",
    "${SRC_DIR}/common.cpp",
    "${SRC_DIR}/config.cpp",
    "${SRC_DIR}/debugger.cpp",
    "${SRC_DIR}/emu.cpp",
    "${SRC_DIR}/fifo.cpp",
    "${SRC_DIR}/fileio.cpp",
]

XCODE_HEADERS = [
    "${SRC_DIR}/menu/BaseMenu.h",
    "${SRC_DIR}/menu/menu.h",
    "${SRC_DIR}/Xcode/xcode_menu_wrapper.h",
    "${SRC_DIR}/Xcode/osd.h",
    "${SRC_DIR}/Xcode/windows_define.h",
    "${SRC_DIR}/Xcode/xcode_mainloop.h",
    "${SRC_DIR}/Xcode/Bridge.h",
    "${SRC_DIR}/Xcode/MetalView.h",
    "${SRC_DIR}/Xcode/file_dialog.h",
    "${SRC_DIR}/Xcode/config_dialog.h",
    "${SRC_DIR}/Xcode/alert_dialog.h",
    "${SRC_DIR}/Xcode/CSCPViewController.h",
    "${SRC_DIR}/Xcode/CSCPAppDelegate.h",
]

XCODE_SOURCES = [
    "${SRC_DIR}/Xcode/osd.cpp",
    "${SRC_DIR}/Xcode/osd_screen.mm",
    "${SRC_DIR}/Xcode/osd_sound.cpp",
    "${SRC_DIR}/Xcode/osd_input.cpp",
    "${SRC_DIR}/Xcode/osd_console.cpp",
    "${SRC_DIR}/Xcode/osd_midi.cpp",
    "${SRC_DIR}/Xcode/osd_socket.cpp",
    "${SRC_DIR}/menu/BaseMenu.cpp",
    "${SRC_DIR}/Xcode/xcode_menu_wrapper.cpp",
    "${SRC_DIR}/Xcode/MetalView.mm",
    "${SRC_DIR}/Xcode/file_dialog.mm",
    "${SRC_DIR}/Xcode/config_dialog.mm",
    "${SRC_DIR}/Xcode/alert_dialog.mm",
    "${SRC_DIR}/Xcode/CSCPViewController.mm",
    "${SRC_DIR}/Xcode/CSCPAppDelegate.mm",
]

def get_machine_files():
    """利用可能な機種リストを取得"""
    machines = []
    
    # メニューファイルから機種を検出
    if MENU_DIR.exists():
        for menu_file in MENU_DIR.glob("*.cpp"):
            if menu_file.name not in ["BaseMenu.cpp", "menu.cpp"]:
                machine_name = menu_file.stem
                machines.append(machine_name)
    
    # VM ディレクトリから機種を検出
    if VM_DIR.exists():
        for vm_dir in VM_DIR.iterdir():
            if vm_dir.is_dir() and not vm_dir.name.startswith('.'):
                machines.append(vm_dir.name)
    
    # 重複を除去してソート
    return sorted(list(set(machines)))

def get_vm_dependencies(machine_name):
    """指定された機種のVM依存関係を取得"""
    vm_headers = []
    vm_sources = []
    
    # VM共通ファイル
    common_vm_files = [
        "device.h", "event.h", "io.h", "vm.h", "vm_template.h"
    ]
    
    for file in common_vm_files:
        vm_headers.append(f"${{SRC_DIR}}/vm/{file}")
    
    common_vm_sources = [
        "event.cpp", "io.cpp"
    ]
    
    for file in common_vm_sources:
        vm_sources.append(f"${{SRC_DIR}}/vm/{file}")
    
    # 機種別VMディレクトリ
    machine_vm_dir = VM_DIR / machine_name
    if machine_vm_dir.exists():
        for header in machine_vm_dir.glob("*.h"):
            vm_headers.append(f"${{SRC_DIR}}/vm/{machine_name}/{header.name}")
        
        for source in machine_vm_dir.glob("*.cpp"):
            vm_sources.append(f"${{SRC_DIR}}/vm/{machine_name}/{source.name}")
    
    # CPU・デバイス依存関係の推定
    cpu_files = {
        'z80': ['z80.h', 'z80.cpp'],
        'i86': ['i86.h', 'i86.cpp'],
        'i286': ['i286.h', 'i286.cpp'],
        'i386': ['i386.h', 'i386.cpp'],
        'mc6809': ['mc6809.h', 'mc6809.cpp', 'mc6809_base.h', 'mc6809_base.cpp'],
        'm6502': ['m6502.h', 'm6502.cpp'],
    }
    
    device_files = {
        'disk': ['disk.h', 'disk.cpp', 'mb8877.h', 'mb8877.cpp'],
        'floppy': ['upd765a.h', 'upd765a.cpp'],
        'hdd': ['harddisk.h', 'harddisk.cpp'],
        'sound': ['noise.h', 'noise.cpp'],
        'tape': ['datarec.h', 'datarec.cpp'],
    }
    
    # 機種名に基づく依存関係の推定
    machine_lower = machine_name.lower()
    
    # CPU依存関係
    if any(x in machine_lower for x in ['pc98', 'pc9801', 'fmr', 'j3100']):
        for file in cpu_files.get('i86', []):
            if file.endswith('.h'):
                vm_headers.append(f"${{SRC_DIR}}/vm/{file}")
            else:
                vm_sources.append(f"${{SRC_DIR}}/vm/{file}")
    elif any(x in machine_lower for x in ['x1', 'fm7', 'fm77', 'mz', 'pc88', 'pc8801']):
        for file in cpu_files.get('z80', []):
            if file.endswith('.h'):
                vm_headers.append(f"${{SRC_DIR}}/vm/{file}")
            else:
                vm_sources.append(f"${{SRC_DIR}}/vm/{file}")
    
    # デバイス依存関係
    if any(x in machine_lower for x in ['x1', 'pc88', 'pc98', 'mz', 'fm']):
        for file in device_files.get('disk', []):
            if file.endswith('.h'):
                vm_headers.append(f"${{SRC_DIR}}/vm/{file}")
            else:
                vm_sources.append(f"${{SRC_DIR}}/vm/{file}")
    
    # X1固有の依存デバイス
    if 'x1' in machine_lower:
        # 必要なデバイス
        x1_devices = [
            'ay_3_891x', 'datarec', 'hd46505', 'i8255', 'mb8877', 
            'pcm8bit', 'upd765a', 'ym2151', 'ym2203', 'z80ctc', 
            'z80dma', 'z80pio', 'z80sio'
        ]
        for device in x1_devices:
            if device + '.h' in [f.name for f in (VM_DIR).glob('*.h')]:
                vm_headers.append(f"${{SRC_DIR}}/vm/{device}.h")
            if device + '.cpp' in [f.name for f in (VM_DIR).glob('*.cpp')]:
                vm_sources.append(f"${{SRC_DIR}}/vm/{device}.cpp")
    
    return vm_headers, vm_sources

def generate_machine_config(machine_name):
    """機種別CMake設定ファイルを生成"""
    print(f"機種 {machine_name} の設定ファイルを生成中...")
    
    # VM依存関係を取得
    vm_headers, vm_sources = get_vm_dependencies(machine_name)
    
    # ヘッダーリスト作成
    headers = COMMON_HEADERS + vm_headers + XCODE_HEADERS
    
    # ソースリスト作成
    sources = COMMON_SOURCES + vm_sources + XCODE_SOURCES
    
    # メニューファイルを追加
    menu_file = f"${{SRC_DIR}}/menu/{machine_name}.cpp"
    sources.append(menu_file)
    
    # CMakeファイル生成
    config_content = f"""set(HEADER
{chr(10).join('        ' + h for h in sorted(set(headers)))}
)

set(SOURCES
{chr(10).join('        ' + s for s in sources)}
)
"""
    
    # ファイル出力
    output_file = MACHINES_DIR / f"_{machine_name.upper()}.txt"
    
    try:
        MACHINES_DIR.mkdir(exist_ok=True)
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(config_content)
        print(f"✓ {output_file} を生成しました")
        return True
    except Exception as e:
        print(f"✗ {output_file} の生成に失敗: {e}")
        return False

def generate_machine_list():
    """利用可能な機種リストを生成"""
    machines = get_machine_files()
    
    list_content = f"""# CommonSourceCodeProject 対応機種リスト
# 生成日時: {os.popen('date').read().strip()}

利用可能な機種数: {len(machines)}

機種名リスト:
{chr(10).join(f"  - {machine}" for machine in machines)}

使用方法:
  cmake -DMACHINE=<機種名> ..

例:
  cmake -DMACHINE=x1turbo ..
  cmake -DMACHINE=pc8801 ..
  cmake -DMACHINE=msx1 ..
"""
    
    list_file = MACHINES_DIR / "MACHINE_LIST.txt"
    
    try:
        with open(list_file, 'w', encoding='utf-8') as f:
            f.write(list_content)
        print(f"✓ {list_file} を生成しました")
        return machines
    except Exception as e:
        print(f"✗ 機種リストの生成に失敗: {e}")
        return []

def main():
    """メイン処理"""
    print("CommonSourceCodeProject CMake機種別設定ファイル生成")
    print("=" * 60)
    
    # 引数処理
    if len(sys.argv) > 1:
        target_machines = [arg.lower() for arg in sys.argv[1:]]
    else:
        target_machines = []
    
    # 利用可能な機種を取得
    available_machines = generate_machine_list()
    
    if not available_machines:
        print("エラー: 利用可能な機種が見つかりません")
        return 1
    
    print(f"\\n利用可能な機種: {len(available_machines)}個")
    
    # 生成対象の決定
    if target_machines:
        # 指定された機種のみ
        machines_to_generate = []
        for machine in target_machines:
            if machine in available_machines:
                machines_to_generate.append(machine)
            else:
                print(f"警告: 機種 '{machine}' が見つかりません")
        
        if not machines_to_generate:
            print("エラー: 有効な機種が指定されていません")
            return 1
    else:
        # 主要な機種のみ生成（全機種だと多すぎるため）
        priority_machines = [
            'x1turbo', 'x1', 'pc8801', 'pc9801', 'msx1', 'msx2', 
            'fm7', 'fm77', 'mz2500', 'mz700', 'pc6001', 'colecovision'
        ]
        machines_to_generate = [m for m in priority_machines if m in available_machines]
    
    print(f"\\n生成対象: {len(machines_to_generate)}機種")
    for machine in machines_to_generate:
        print(f"  - {machine}")
    
    # 生成実行
    print("\\n生成開始...")
    success_count = 0
    
    for machine in machines_to_generate:
        if generate_machine_config(machine):
            success_count += 1
    
    print(f"\\n生成完了: {success_count}/{len(machines_to_generate)}機種")
    
    if success_count == len(machines_to_generate):
        print("✓ すべての機種の設定ファイル生成に成功しました")
        return 0
    else:
        print("⚠ 一部の機種で生成に失敗しました")
        return 1

if __name__ == "__main__":
    sys.exit(main())