// xcode_mainloop.h

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Medamap and Claude: コマンドライン引数解析構造体
typedef struct {
    char machine_name[256];
    char fdd_paths[4][512];  // フロッピーディスクドライブ0-3
    char tape_paths[2][512]; // テープドライブ0-1
    char hdd_paths[4][512];  // ハードディスクドライブ0-3
    char cart_paths[4][512]; // カートリッジスロット0-3
    bool show_help;
} emulator_args_t;

// 関数宣言
void parse_command_line_args(int argc, char** argv, emulator_args_t* args);
void show_help();
void load_ini_file(void* emu, const char* machine_name);
void save_ini_file(void* emu, const char* machine_name);
void mount_media(void* emu, const emulator_args_t* args);

void run_emulator_mainloop_with_args(int argc, char** argv);
void run_emulator_mainloop();

#ifdef __cplusplus
}
#endif

