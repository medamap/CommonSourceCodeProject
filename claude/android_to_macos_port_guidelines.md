# Android to macOS Port Guidelines

## 基本原則

### エミュレータコアの統一性
- `src/*.cpp|h` - プラットフォーム間で同じであるべき
- `src/vm/*.cpp|h` - プラットフォーム間で同じであるべき  
- `src/vm/*/*.cpp|h` - プラットフォーム間で同じであるべき

### プラットフォーム固有部分
```
Android: src/Android/*     → Mac: src/Xcode/*
Android: src/Android/menu/* → Mac: src/menu/*
```

## 許可される修正パターン

### ✅ 許可される修正
- C++実装やライブラリの方言への対応（POSIX、WIN系、GCC等）
- プラットフォーム固有のAPI呼び出し差異
- コンパイラ固有の警告抑制
- 標準ライブラリの実装差異への対応

### ❌ 避けるべき修正
- 機種固有の機能（USE_BOOT_MODE等）をエミュレータコアで条件分岐
- ビジネスロジックの変更
- エミュレータの動作に影響する修正

## 正しい移植手順

### 1. Android設定のコピー
```bash
# Android版_{機種名大文字}.txtからコピー
cp androidstudio/app/src/main/cpp/_MZ80B.txt xcode/Machines/_MZ80B.txt
```

### 2. プラットフォーム固有部分の置換
```cmake
# src/Android/* → src/Xcode/*
# src/Android/menu/* → src/menu/*
```

### 3. エミュレータコアは変更しない
- 機種固有ヘッダー（mz80b.h、mz2500.h）が正しくインクルードされる
- CMakeの-D_{機種名}定義が正しく伝播される

## 根本原因の推測
- 機種固有ヘッダーのインクルード漏れ
- インクルード順序の問題
- CMake定義の伝播不備

## 調査で判明した事実

### USE_BOOT_MODE定義状況
- **MZ2500**: `mz2500.h:37`で`USE_BOOT_MODE 3`定義済み
- **MZ80B**: `mz80b.h`で`USE_BOOT_MODE`未定義（boot_mode = 0固定）

### Android CMakeLists.txt設定
```cmake
# MZ80B
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ80B -D_Android -D_RGB565 -D_EXTEND_MENU -D_USE_OPENGL_ES30")

# MZ2500  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ2500 -D_Android -D_HAS_TENKEY -D_RGB565 -D_EXTEND_MENU -D_USE_OPENGL_ES30")
```

## 参考情報
- Android build.gradle: 全102機種の定義済み
- Android CMakeLists.txt: 機種別ビルド設定完備
- Android _{機種}.txt: 機種別ファイルリスト完備