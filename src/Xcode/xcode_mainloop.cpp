#include "Xcode/osd.h"
#include "emu.h"
#include "fileio.h"
#include <cstdio>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdlib>
#include "xcode_mainloop.h"

// emulator_args_t はヘッダーファイルで定義済み

// コマンドライン引数解析関数
extern "C" void parse_command_line_args(int argc, char** argv, emulator_args_t* args)
{
    // 初期化
    memset(args, 0, sizeof(emulator_args_t));
    
    // 実行ファイル名から機種名を自動判定（固定）
    const char* program_name = argv[0];
    const char* base_name = strrchr(program_name, '/');
    if (base_name) {
        base_name++; // '/' の次の文字から
    } else {
        base_name = program_name;
    }
    strncpy(args->machine_name, base_name, sizeof(args->machine_name) - 1);
    
    printf("コマンドライン引数を解析中...\n");
    printf("機種: %s\n", args->machine_name);
    
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-fdd", 4) == 0 && i + 1 < argc) {
            int drive = argv[i][4] - '0'; // -fdd0, -fdd1, -fdd2, -fdd3
            if (drive >= 0 && drive < 4) {
                strncpy(args->fdd_paths[drive], argv[i + 1], sizeof(args->fdd_paths[drive]) - 1);
                printf("FDD%d: %s\n", drive, args->fdd_paths[drive]);
                i++;
            }
        }
        else if (strncmp(argv[i], "-tape", 5) == 0 && i + 1 < argc) {
            int drive = argv[i][5] - '0'; // -tape0, -tape1
            if (drive >= 0 && drive < 2) {
                strncpy(args->tape_paths[drive], argv[i + 1], sizeof(args->tape_paths[drive]) - 1);
                printf("TAPE%d: %s\n", drive, args->tape_paths[drive]);
                i++;
            }
        }
        else if (strncmp(argv[i], "-hdd", 4) == 0 && i + 1 < argc) {
            int drive = argv[i][4] - '0'; // -hdd0, -hdd1, -hdd2, -hdd3
            if (drive >= 0 && drive < 4) {
                strncpy(args->hdd_paths[drive], argv[i + 1], sizeof(args->hdd_paths[drive]) - 1);
                printf("HDD%d: %s\n", drive, args->hdd_paths[drive]);
                i++;
            }
        }
        else if (strncmp(argv[i], "-cart", 5) == 0 && i + 1 < argc) {
            int drive = argv[i][5] - '0'; // -cart0, -cart1, -cart2, -cart3
            if (drive >= 0 && drive < 4) {
                strncpy(args->cart_paths[drive], argv[i + 1], sizeof(args->cart_paths[drive]) - 1);
                printf("CART%d: %s\n", drive, args->cart_paths[drive]);
                i++;
            }
        }
        else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = true;
        }
    }
}

// ヘルプメッセージ表示
extern "C" void show_help()
{
    printf("CSCPエミュレータ - macOS/iOS版\n");
    printf("\n使用方法:\n");
    printf("  ./<機種名> [オプション]\n");
    printf("\nオプション:\n");
    printf("  -fdd0 <パス>             フロッピーディスクドライブ0にマウント\n");
    printf("  -fdd1 <パス>             フロッピーディスクドライブ1にマウント\n");
    printf("  -fdd2 <パス>             フロッピーディスクドライブ2にマウント\n");
    printf("  -fdd3 <パス>             フロッピーディスクドライブ3にマウント\n");
    printf("  -tape0 <パス>            テープドライブ0にマウント\n");
    printf("  -tape1 <パス>            テープドライブ1にマウント\n");
    printf("  -hdd0 <パス>             ハードディスクドライブ0にマウント\n");
    printf("  -hdd1 <パス>             ハードディスクドライブ1にマウント\n");
    printf("  -hdd2 <パス>             ハードディスクドライブ2にマウント\n");
    printf("  -hdd3 <パス>             ハードディスクドライブ3にマウント\n");
    printf("  -cart0 <パス>            カートリッジスロット0にマウント\n");
    printf("  -cart1 <パス>            カートリッジスロット1にマウント\n");
    printf("  -cart2 <パス>            カートリッジスロット2にマウント\n");
    printf("  -cart3 <パス>            カートリッジスロット3にマウント\n");
    printf("  -help, --help            このヘルプを表示\n");
    printf("\n例:\n");
    printf("  ./x1turbo -fdd0 \"Alpha.d88\"              # X1 Turboでフロッピー起動\n");
    printf("  ./msx1 -cart0 \"Gradius.rom\"              # MSX1でカートリッジ起動\n");
    printf("  ./pc8801 -fdd0 \"game.d88\"                # PC-8801でフロッピー起動\n");
    printf("  ./mz700 -tape0 \"program.wav\"             # MZ-700でテープ起動\n");
}

// パス変換関数（char* から _TCHAR* へ）
#ifdef UNICODE
void convert_to_tchar(const char* src, _TCHAR* dest, size_t dest_size)
{
    // Unicode版では変換が必要だが、現在はシンプルにコピー
    mbstowcs(dest, src, dest_size - 1);
    dest[dest_size - 1] = L'\0';
}
#else
void convert_to_tchar(const char* src, _TCHAR* dest, size_t dest_size)
{
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}
#endif

// メディアマウント関数
extern "C" void mount_media(void* emu_ptr, const emulator_args_t* args)
{
    EMU* emu = (EMU*)emu_ptr;
    printf("メディアマウント中...\n");
    
    // フロッピーディスクドライブマウント
    for (int i = 0; i < 4; i++) {
        if (strlen(args->fdd_paths[i]) > 0) {
            printf("FDD%d に %s をマウント中...\n", i, args->fdd_paths[i]);
            #ifdef USE_FLOPPY_DISK
            _TCHAR tchar_path[512];
            convert_to_tchar(args->fdd_paths[i], tchar_path, sizeof(tchar_path)/sizeof(_TCHAR));
            
            try {
                emu->open_floppy_disk(i, tchar_path, 0); // bank 0 を使用
                printf("FDD%d マウント成功: %s\n", i, args->fdd_paths[i]);
            } catch (...) {
                printf("FDD%d マウント失敗: %s\n", i, args->fdd_paths[i]);
            }
            #else
            printf("FDD%d: フロッピーディスク機能が無効です\n", i);
            #endif
        }
    }
    
    // テープドライブマウント
    for (int i = 0; i < 2; i++) {
        if (strlen(args->tape_paths[i]) > 0) {
            printf("TAPE%d に %s をマウント中...\n", i, args->tape_paths[i]);
            #ifdef USE_TAPE
            _TCHAR tchar_path[512];
            convert_to_tchar(args->tape_paths[i], tchar_path, sizeof(tchar_path)/sizeof(_TCHAR));
            
            try {
                emu->play_tape(i, tchar_path);
                printf("TAPE%d マウント成功: %s\n", i, args->tape_paths[i]);
            } catch (...) {
                printf("TAPE%d マウント失敗: %s\n", i, args->tape_paths[i]);
            }
            #else
            printf("TAPE%d: テープ機能が無効です\n", i);
            #endif
        }
    }
    
    // ハードディスクドライブマウント
    for (int i = 0; i < 4; i++) {
        if (strlen(args->hdd_paths[i]) > 0) {
            printf("HDD%d に %s をマウント中...\n", i, args->hdd_paths[i]);
            #ifdef USE_HARD_DISK
            _TCHAR tchar_path[512];
            convert_to_tchar(args->hdd_paths[i], tchar_path, sizeof(tchar_path)/sizeof(_TCHAR));
            
            try {
                emu->open_hard_disk(i, tchar_path);
                printf("HDD%d マウント成功: %s\n", i, args->hdd_paths[i]);
            } catch (...) {
                printf("HDD%d マウント失敗: %s\n", i, args->hdd_paths[i]);
            }
            #else
            printf("HDD%d: ハードディスク機能が無効です\n", i);
            #endif
        }
    }
    
    // カートリッジマウント
    for (int i = 0; i < 4; i++) {
        if (strlen(args->cart_paths[i]) > 0) {
            printf("CART%d に %s をマウント中...\n", i, args->cart_paths[i]);
            #ifdef USE_CART
            _TCHAR tchar_path[512];
            convert_to_tchar(args->cart_paths[i], tchar_path, sizeof(tchar_path)/sizeof(_TCHAR));
            
            try {
                emu->open_cart(i, tchar_path);
                printf("CART%d マウント成功: %s\n", i, args->cart_paths[i]);
            } catch (...) {
                printf("CART%d マウント失敗: %s\n", i, args->cart_paths[i]);
            }
            #else
            printf("CART%d: カートリッジ機能が無効です\n", i);
            #endif
        }
    }
    
    printf("メディアマウント完了\n");
}

// 仮の ini ファイル読込み関数
extern "C" void load_ini_file(void* emu_ptr, const char* machine_name)
{
    printf("load_ini_file() - 機種: %s\n", machine_name);
    // Medamap and Claude: 将来的にmachine_name.iniファイルを読み込み
}

// 仮の ini ファイル保存関数
extern "C" void save_ini_file(void* emu_ptr, const char* machine_name)
{
    printf("save_ini_file() - 機種: %s\n", machine_name);
    // Medamap and Claude: 将来的にmachine_name.iniファイルに設定を保存
    // TODO: 実際のINIファイル保存処理を実装
}

extern "C" void run_emulator_mainloop_with_args(int argc, char** argv)
{
    // コマンドライン引数解析
    emulator_args_t args;
    parse_command_line_args(argc, argv, &args);
    
    // ヘルプ表示
    if (args.show_help) {
        show_help();
        return;
    }
    
    printf("エミュレータを初期化中...\n");
    printf("使用機種: %s\n", args.machine_name);

    // EMUオブジェクトの生成
    EMU* emu = new EMU();
    if (!emu) {
        printf("EMU オブジェクトの生成に失敗しました\n");
        return;
    }

    load_ini_file(emu, args.machine_name);
    
    // メディアマウント
    mount_media(emu, &args);

    printf("メインループを開始します...\n");

    // Medamap and Claude: 基本的な60FPSメインループ
    const int target_fps = 60;
    const auto frame_duration = std::chrono::milliseconds(1000 / target_fps);
    
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();
    
    // テスト用に300フレーム（5秒間）実行
    for (int i = 0; i < 300; i++) {
        auto frame_start = std::chrono::steady_clock::now();
        
        // エミュレーターを1フレーム実行（内部でinputの更新も行われる）
        emu->run();
        
        frame_count++;
        
        // フレームレート制御
        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;
        
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
        
        // 1秒ごとに状況を出力
        if (frame_count % 60 == 0) {
            auto current_time = std::chrono::steady_clock::now();
            auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            printf("フレーム %d実行完了 (経過時間: %lld ms)\n", frame_count, total_elapsed.count());
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    printf("メインループ終了: %d フレーム実行, 総経過時間: %lld ms\n", frame_count, total_elapsed.count());

    printf("リソース解放中...\n");
    delete emu;
}

// 引数なしバージョン（iOS用Bridge）
extern "C" void run_emulator_mainloop() {
    run_emulator_mainloop_with_args(0, nullptr);
}
