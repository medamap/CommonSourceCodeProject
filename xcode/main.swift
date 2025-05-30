import Foundation

// C++ 側のメインループ関数（Bridge.h で宣言されてる前提）
@_silgen_name("run_emulator_mainloop")
func run_emulator_mainloop()

// メイン実行部
print("CSCP iPadOS版 起動")

run_emulator_mainloop()

print("CSCP iPadOS版 終了")
