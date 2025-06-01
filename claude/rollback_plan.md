# エミュレータコア修正のロールバック計画

## 不適切な修正を加えたファイル一覧

### 1. `/src/vm/mz2500/crtc.cpp` (行32-36)
```cpp
// 我々が追加した不適切な修正：
#ifdef USE_BOOT_MODE
	boot_mode = config.boot_mode;
#else
	boot_mode = 0;
#endif
```

### 2. `/src/vm/mz2500/cmt.cpp` (行31-34)
```cpp
// 我々が追加した不適切な修正：
#ifdef USE_BOOT_MODE
	is_mz80b = (config.boot_mode == 2);
#else
	is_mz80b = false;
```

### 3. `/src/vm/mz2500/mz2500.cpp` (行129-132, 252-257, 280-283)
```cpp
// 我々が追加した不適切な修正（複数箇所）：
#ifdef USE_BOOT_MODE
	event->set_context_cpu(cpu, config.boot_mode ? CPU_CLOCKS_LOW : CPU_CLOCKS);
#else
	event->set_context_cpu(cpu, CPU_CLOCKS);
```

### 4. `/src/vm/mz2500/memory.cpp` (行69-72)
```cpp
// 我々が追加した不適切な修正：
#ifdef USE_BOOT_MODE
	is_4mhz = (config.boot_mode != 0);
#else
	is_4mhz = false;
```

## ロールバック戦略

### Phase 1: 現在の状態をバックアップ
- 修正前にgitでコミット状態を確認
- 必要に応じて現在の修正をバックアップ

### Phase 2: 元のコードに戻す
- 各ファイルから我々が追加した`#ifdef USE_BOOT_MODE`セクションを削除
- 元のAndroidビルドと同じ直接参照に戻す

### Phase 3: 動作確認
- Android CMakeLists.txtの設定を参考に、正しい機種ヘッダーインクルードを確認
- ビルドテストで動作確認

## 注意点
- エミュレータコアの統一性を保つため、条件コンパイルは機種ヘッダーでの定義に依存させる
- Androidで動作している元のコードに戻すことが最優先
- プラットフォーム固有の修正は `src/Xcode/*` でのみ行う