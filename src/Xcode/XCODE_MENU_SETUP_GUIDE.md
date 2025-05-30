# Xcodeプロジェクトでのメニュー設定ガイド

## 概要

このガイドでは、AndroidとXcodeで共通のメニューファイル（`src/menu`配下）を使用する方法を説明します。

## 設定方法

### 方法1: Xcodeプロジェクトに直接追加

1. **Xcodeでプロジェクトを開く**

2. **メニューファイルをプロジェクトに追加**
   - プロジェクトナビゲータで右クリック → "Add Files to..."
   - 以下のファイルを追加：
     ```
     src/menu/BaseMenu.h
     src/menu/BaseMenu.cpp
     src/menu/menu.h
     src/menu/[機種名].cpp  （例: x1turbo.cpp）
     ```
   - "Create groups" を選択
   - "Add to targets" で適切なターゲットを選択

3. **ラッパーファイルを追加**
   ```
   src/Xcode/xcode_menu_wrapper.h
   src/Xcode/xcode_menu_wrapper.cpp
   ```

4. **ビルド設定でヘッダー検索パスを追加**
   - Build Settings → Search Paths → Header Search Paths
   - 以下のパスを追加：
     ```
     $(PROJECT_DIR)/src
     ```

### 方法2: CMakeを使用する場合

1. **CMakeLists.txtで機種を指定**
   ```cmake
   # 機種を定義
   set(TARGET_MACHINE "X1TURBO")
   
   # 機種別の設定ファイルをインクルード
   if(TARGET_MACHINE STREQUAL "X1TURBO")
       include(CMakeLists_X1TURBO.txt)
   elseif(TARGET_MACHINE STREQUAL "MSX1")
       include(CMakeLists_MSX1.txt)
   endif()
   
   # ソースファイルに追加
   add_executable(emulator
       ${ALL_MENU_SOURCES}
       # ... 他のソースファイル
   )
   ```

2. **Xcodeでビルド**
   ```bash
   mkdir build_xcode
   cd build_xcode
   cmake -G Xcode -DTARGET_MACHINE=X1TURBO ..
   open emulator.xcodeproj
   ```

### 方法3: 複数ターゲットを使用する場合

1. **各機種用のターゲットを作成**
   - Product → Scheme → Manage Schemes
   - 新しいターゲットを追加（例: X1Turbo, MSX1, PC8801）

2. **ターゲットごとに異なるメニューファイルを設定**
   - X1Turbo Target:
     ```
     BaseMenu.cpp
     x1turbo.cpp
     xcode_menu_wrapper.cpp
     ```
   - MSX1 Target:
     ```
     BaseMenu.cpp
     msx1.cpp
     xcode_menu_wrapper.cpp
     ```

3. **プリプロセッサマクロを設定**
   - Build Settings → Preprocessor Macros
   - X1Turbo: `_X1TURBO`
   - MSX1: `_MSX1`

## 使用例

### Bridge.mmでの使用

```objc
#import "Bridge.h"
#include "xcode_menu_wrapper.h"

@implementation Bridge

- (void)setupMenu {
    // メニューシステム初期化
    init_menu_system();
    
    // メニュー取得
    Menu* menu = get_main_menu();
    
    // NSMenuに変換（例）
    [self convertToNSMenu:menu];
}

- (void)menuAction:(NSMenuItem*)sender {
    int menuId = (int)sender.tag;
    handle_menu_event(menuId, emu);
}

@end
```

### main.cppでの使用

```cpp
#include "xcode_menu_wrapper.h"
#include "emu.h"

int main() {
    // エミュレータ初期化
    EMU* emu = new EMU();
    
    // メニュー初期化
    init_menu_system();
    
    // メインループ
    while (running) {
        emu->run();
        
        // 定期的にメニュー更新
        if (frame_count % 60 == 0) {
            update_menu_system(emu);
        }
    }
    
    // クリーンアップ
    cleanup_menu_system();
    delete emu;
}
```

## トラブルシューティング

### 問題: メニューファイルが見つからない

**解決方法:**
- ヘッダー検索パスが正しく設定されているか確認
- 相対パスが正しいか確認（`../menu/` など）

### 問題: シンボルが重複する

**解決方法:**
- 複数の機種のメニューファイルを同時にビルドしていないか確認
- 1つのターゲットには1つの機種のメニューファイルのみを含める

### 問題: Menu クラスが定義されていないエラー

**解決方法:**
- 機種別のメニューファイル（例: x1turbo.cpp）が正しくビルドに含まれているか確認
- プリプロセッサマクロが正しく設定されているか確認

## メニューファイルの追加方法

新しい機種のメニューを追加する場合：

1. `src/menu/[新機種].cpp` を作成（既存のファイルを参考に）
2. CMakeLists_[新機種].txt を作成
3. Xcodeプロジェクトに追加
4. 必要に応じてプリプロセッサマクロを定義

## まとめ

この方式により：
- AndroidとXcodeで同じメニューファイルを共有
- 機種ごとのメニュー定義が独立
- ビルドシステムで機種を選択
- 条件付きコンパイルの乱立を回避

メンテナンスが容易で、新機種の追加も簡単になります。