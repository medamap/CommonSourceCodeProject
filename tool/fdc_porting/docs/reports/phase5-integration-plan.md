# Phase 5 MB8877互換レイヤー統合計画

## 統合戦略概要

### 1. 実装環境
- **ターゲット**: X1turboエミュレータ（macOS版）
- **ビルドシステム**: CMake + Swift Package Manager
- **プロジェクトパス**: `/Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/CommonSourceCodeProject/`

### 2. 統合手順

#### Step 1: Type III Commands重複解消
**問題点**:
- cmd_readaddr(): 3箇所で定義（1345, 1896, 2700行目）
- cmd_readtrack(): 3箇所で定義（1354, 1954, 2721行目）
- cmd_writetrack(): 3箇所で定義（1362, 1997, 2740行目）

**解決方針**:
1. 最も完全な実装を選択（1896-2696行目の実装が最も詳細）
2. 他の実装をコメントアウトまたは削除
3. 統一されたインターフェースを維持

#### Step 2: CMakeLists.txt修正
```cmake
# 59行目を修正
${VM_DIR}/mb8877.cpp → ${VM_DIR}/mb8877_compat.cpp
```

#### Step 3: オリジナルmb8877のバックアップ
```bash
cp src/vm/mb8877.cpp src/vm/mb8877_original.cpp
cp src/vm/mb8877.h src/vm/mb8877_original.h
```

#### Step 4: 互換レイヤーの配置
```bash
cp src/vm/mb8877_compat.cpp src/vm/mb8877.cpp
cp src/vm/mb8877_compat.h src/vm/mb8877.h
```

### 3. ビルドエラー対策

#### 予想されるエラーと対策
1. **イベント関連**: 実環境のevent.hとの統合
2. **DISK クラス**: disk.hインターフェースとの整合性
3. **デバッグ出力**: out_debug_log()メソッドの実装確認

### 4. テスト計画

#### Phase 5-A: 基本動作確認（70%目標）
- [ ] ビルド成功
- [ ] エミュレータ起動
- [ ] ディスクイメージ認識
- [ ] 基本的な読み込み動作

#### Phase 5-B: 実用レベル（85%目標）
- [ ] BASIC起動
- [ ] ゲームディスク動作
- [ ] エラーハンドリング確認

#### Phase 5-C: 完全互換性（95%目標）
- [ ] 特殊フォーマット対応
- [ ] タイミング互換性
- [ ] 長時間安定動作

### 5. リスク管理

#### 高リスク項目
1. **Type III Commands**: 重複定義の解消ミス
2. **イベントタイミング**: 実環境での精密制御
3. **メモリ管理**: buffer[]配列の境界チェック

#### 低リスク項目
1. **基本的なI/O**: 既に50%動作確認済み
2. **レジスタ操作**: 単体テストで検証済み
3. **Type I/II Commands**: 基本実装完了

### 6. 実装優先順位

1. **最優先**: Type III Commands重複解消
2. **高優先**: CMake統合とビルド成功
3. **中優先**: 実ディスクでの動作確認
4. **低優先**: 特殊フォーマット対応

## 次のステップ

1. Type III Commands重複定義の解消
2. CMakeLists.txt修正
3. 初期ビルド実行
4. エラー解析と修正
5. 基本動作テスト