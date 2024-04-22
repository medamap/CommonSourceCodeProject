//
// Created by medamap on 2024/04/03.
//

#include "menu.h"
#include "BaseMenu.h"
#include "../../res/resource.h"

// BaseMenu を継承して Menu クラスを作成する
Menu::Menu() {
    // Root メニューを作成
    int rootId = addNode(0, "Root", Category, -1);
    int controlId = addNode(rootId, "Control", Category, -1);
    addNode(controlId, "Reset", Property, ID_RESET);
    addNode(controlId, "CPU x1", Property, ID_CPU_POWER0);
    addNode(controlId, "CPU x2", Property, ID_CPU_POWER1);
    addNode(controlId, "CPU x4", Property, ID_CPU_POWER2);
    addNode(controlId, "CPU x8", Property, ID_CPU_POWER3);
    addNode(controlId, "CPU x16", Property, ID_CPU_POWER4);
    addNode(controlId, "Full Speed", Property, ID_FULL_SPEED);
    addNode(controlId, "Paste", Property, ID_AUTOKEY_START);
    addNode(controlId, "Stop", Property, ID_AUTOKEY_STOP);
    addNode(controlId, "Romaji to Kana", Property, ID_ROMAJI_TO_KANA);
    int saveStateId = addNode(controlId, "Save State", Category, -1);
    for (int i = 0; i < 10; i++) {
        addNode(saveStateId, "State " + std::to_string(i), Property, ID_SAVE_STATE0 + i);
    }
    int loadStateId = addNode(controlId, "Load State", Category, -1);
    for (int i = 0; i < 10; i++) {
        addNode(loadStateId, "State " + std::to_string(i), Property, ID_LOAD_STATE0 + i);
    }
    addNode(controlId, "Debug Main CPU", Property, ID_OPEN_DEBUGGER0);
    addNode(controlId, "Debug Sub CPU", Property, ID_OPEN_DEBUGGER1);
    addNode(controlId, "Close Debugger", Property, ID_CLOSE_DEBUGGER);
    addNode(controlId, "Exit", Property, ID_EXIT);

    int fd1Id = addNode(rootId, "FD1", Category, -1, MENU_FDD0);
    addNode(fd1Id, "Insert", Property, ID_OPEN_FD1);
    addNode(fd1Id, "Eject", Property, ID_CLOSE_FD1);
    addNode(fd1Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD1);
    addNode(fd1Id, "Write Protected", Property, ID_WRITE_PROTECT_FD1);
    addNode(fd1Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD1);
    addNode(fd1Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD1);
    addNode(fd1Id, "Recent", Property, ID_RECENT_FD1);

    int fd2Id = addNode(rootId, "FD2", Category, -1, MENU_FDD1);
    addNode(fd2Id, "Insert", Property, ID_OPEN_FD2);
    addNode(fd2Id, "Eject", Property, ID_CLOSE_FD2);
    addNode(fd2Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD2);
    addNode(fd2Id, "Write Protected", Property, ID_WRITE_PROTECT_FD2);
    addNode(fd2Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD2);
    addNode(fd2Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD2);
    addNode(fd2Id, "Recent", Property, ID_RECENT_FD2);

    int cmtId = addNode(rootId, "CMT", Category, -1, MENU_TAPE0);
    addNode(cmtId, "Play", Property, ID_PLAY_TAPE1);
    addNode(cmtId, "Rec", Property, ID_REC_TAPE1);
    addNode(cmtId, "Eject", Property, ID_CLOSE_TAPE1);
    addNode(cmtId, "Play Button", Property, ID_PLAY_BUTTON1);
    addNode(cmtId, "Stop Button", Property, ID_STOP_BUTTON1);
    addNode(cmtId, "Fast Forward", Property, ID_FAST_FORWARD1);
    addNode(cmtId, "Fast Rewind", Property, ID_FAST_REWIND1);
    addNode(cmtId, "Waveform Shaper", Property, ID_USE_WAVE_SHAPER1);
    addNode(cmtId, "Recent", Property, ID_RECENT_TAPE1);

    int device = addNode(rootId, "Device", Category, -1);
    int boot = addNode(device, "Boot", Category, -1);
    addNode(boot, "BASIC", Property, ID_VM_BOOT_MODE0);
    addNode(boot, "DOS", Property, ID_VM_BOOT_MODE1);

    int cpu = addNode(device, "CPU", Category, -1);
    addNode(cpu, "68B09 2.0MHz", Property, ID_VM_CPU_TYPE0);
    addNode(cpu, "68B09 1.2MHz", Property, ID_VM_CPU_TYPE1);

    int option = addNode(device, "Option", Category, -1);
    addNode(option, "Cycle Steal (Hack)", Property, ID_VM_DIPSWITCH0);
    addNode(option, "Kanji ROM", Property, ID_VM_DIPSWITCH5);
    addNode(option, "320KB FDD", Property, ID_VM_DIPSWITCH6);
    addNode(option, "1MB FDD", Property, ID_VM_DIPSWITCH7);

    int mouse = addNode(device, "Mouse", Category, -1);
    addNode(mouse, "None", Property, ID_VM_MOUSE_TYPE0);
    addNode(mouse, "JS-Port1", Property, ID_VM_MOUSE_TYPE1);
    addNode(mouse, "JS-Port2", Property, ID_VM_MOUSE_TYPE2);

    int sound = addNode(device, "Sound", Category, -1);
    addNode(sound, "PSG", Property, ID_VM_SOUND_TYPE0);
    addNode(sound, "PSG+OPN", Property, ID_VM_SOUND_TYPE1);
    addNode(sound, "PSG+WHG", Property, ID_VM_SOUND_TYPE2);
    addNode(sound, "PSG+OPN+WHG", Property, ID_VM_SOUND_TYPE3);
    addNode(sound, "PSG+THG", Property, ID_VM_SOUND_TYPE4);
    addNode(sound, "PSG+OPN+THG", Property, ID_VM_SOUND_TYPE5);
    addNode(sound, "PSG+WHG+THG", Property, ID_VM_SOUND_TYPE6);
    addNode(sound, "PSG+OPN+WHG+THG", Property, ID_VM_SOUND_TYPE7);
    addNode(sound, "Play FDD Noise", Property, ID_VM_SOUND_NOISE_FDD);
    addNode(sound, "Play CMT Noise", Property, ID_VM_SOUND_NOISE_CMT);
    addNode(sound, "Play CMT Signal", Property, ID_VM_SOUND_TAPE_SIGNAL);
    addNode(sound, "Play CMT Voice", Property, ID_VM_SOUND_TAPE_VOICE);

    int display = addNode(device, "Display", Category, -1);
    addNode(display, "Scanline", Property, ID_VM_MONITOR_SCANLINE);

    int printer = addNode(device, "Printer", Category, -1);
    addNode(printer, "Write Printer to File", Property, ID_VM_PRINTER_TYPE0);
    addNode(printer, "Dempa Joystick #1", Property, ID_VM_PRINTER_TYPE1);
    addNode(printer, "Dempa Joystick #2", Property, ID_VM_PRINTER_TYPE2);
    addNode(printer, "None", Property, ID_VM_PRINTER_TYPE3);

    int host = addNode(rootId, "Host", Category, -1);
    addNode(host, "Rec Movie 60fps", Property, ID_HOST_REC_MOVIE_60FPS);
    addNode(host, "Rec Movie 30fps", Property, ID_HOST_REC_MOVIE_30FPS);
    addNode(host, "Rec Movie 15fps", Property, ID_HOST_REC_MOVIE_15FPS);
    addNode(host, "Rec Sound", Property, ID_HOST_REC_SOUND);
    addNode(host, "Stop", Property, ID_HOST_REC_STOP);
    addNode(host, "Capture Screen", Property, ID_HOST_CAPTURE_SCREEN);

    int screen = addNode(host, "Screen", Category, -1);
    addNode(screen, "Window x1", Property, ID_SCREEN_WINDOW);
    addNode(screen, "Fullscreen 640x400", Property, ID_SCREEN_FULLSCREEN);
    addNode(screen, "Window Stretch 1", Property, ID_SCREEN_WINDOW_STRETCH);
    addNode(screen, "Window Stretch 2", Property, ID_SCREEN_WINDOW_ASPECT);
    addNode(screen, "Fullscreen Stretch 1", Property, ID_SCREEN_FULLSCREEN_DOTBYDOT);
    addNode(screen, "Fullscreen Stretch 2", Property, ID_SCREEN_FULLSCREEN_STRETCH);
    addNode(screen, "Fullscreen Stretch 3", Property, ID_SCREEN_FULLSCREEN_ASPECT);
    addNode(screen, "Fullscreen Stretch 4", Property, ID_SCREEN_FULLSCREEN_FILL);
    addNode(screen, "Rotate 0deg", Property, ID_SCREEN_ROTATE_0);
    addNode(screen, "Rotate +90deg", Property, ID_SCREEN_ROTATE_90);
    addNode(screen, "Rotate 180deg", Property, ID_SCREEN_ROTATE_180);
    addNode(screen, "Rotate -90deg", Property, ID_SCREEN_ROTATE_270);

    int marginId1 = addNode(screen, "Screen Top Margin", Category, -1);
    addNode(marginId1, "0", Property, ID_SCREEN_TOP_MARGIN_0);
    addNode(marginId1, "30", Property, ID_SCREEN_TOP_MARGIN_30);
    addNode(marginId1, "60", Property, ID_SCREEN_TOP_MARGIN_60);
    addNode(marginId1, "90", Property, ID_SCREEN_TOP_MARGIN_90);
    addNode(marginId1, "120", Property, ID_SCREEN_TOP_MARGIN_120);
    addNode(marginId1, "150", Property, ID_SCREEN_TOP_MARGIN_150);
    addNode(marginId1, "180", Property, ID_SCREEN_TOP_MARGIN_180);
    addNode(marginId1, "210", Property, ID_SCREEN_TOP_MARGIN_210);
    addNode(marginId1, "240", Property, ID_SCREEN_TOP_MARGIN_240);
    addNode(marginId1, "270", Property, ID_SCREEN_TOP_MARGIN_270);

    int marginId2 = addNode(screen, "Screen Bottom Margin", Category, -1);
    addNode(marginId2, "0", Property, ID_SCREEN_BOTTOM_MARGIN_0);
    addNode(marginId2, "30", Property, ID_SCREEN_BOTTOM_MARGIN_30);
    addNode(marginId2, "60", Property, ID_SCREEN_BOTTOM_MARGIN_60);
    addNode(marginId2, "90", Property, ID_SCREEN_BOTTOM_MARGIN_90);
    addNode(marginId2, "120", Property, ID_SCREEN_BOTTOM_MARGIN_120);
    addNode(marginId2, "150", Property, ID_SCREEN_BOTTOM_MARGIN_150);
    addNode(marginId2, "180", Property, ID_SCREEN_BOTTOM_MARGIN_180);
    addNode(marginId2, "210", Property, ID_SCREEN_BOTTOM_MARGIN_210);
    addNode(marginId2, "240", Property, ID_SCREEN_BOTTOM_MARGIN_240);
    addNode(marginId2, "270", Property, ID_SCREEN_BOTTOM_MARGIN_270);

    int filterId = addNode(host, "Filter", Category, -1);
    addNode(filterId, "Green Filter", Property, ID_FILTER_GREEN);
    addNode(filterId, "RGB Filter", Property, ID_FILTER_RGB);
    addNode(filterId, "Blur Filter", Property, ID_FILTER_BLUR);
    addNode(filterId, "DOT", Property, ID_FILTER_DOT);
    addNode(filterId, "None", Property, ID_FILTER_NONE);

    int sound2 = addNode(host, "Sound", Category, -1);
    addNode(sound2, "Switch On / Off", Property, ID_SOUND_ON);
    addNode(sound2, "2000Hz", Property, ID_SOUND_FREQ0);
    addNode(sound2, "4000Hz", Property, ID_SOUND_FREQ1);
    addNode(sound2, "8000Hz", Property, ID_SOUND_FREQ2);
    addNode(sound2, "11025Hz", Property, ID_SOUND_FREQ3);
    addNode(sound2, "22050Hz", Property, ID_SOUND_FREQ4);
    addNode(sound2, "44100Hz", Property, ID_SOUND_FREQ5);
    addNode(sound2, "48000Hz", Property, ID_SOUND_FREQ6);
    addNode(sound2, "96000Hz", Property, ID_SOUND_FREQ7);
    addNode(sound2, "50msec", Property, ID_SOUND_LATE0);
    addNode(sound2, "100msec", Property, ID_SOUND_LATE1);
    addNode(sound2, "200msec", Property, ID_SOUND_LATE2);
    addNode(sound2, "300msec", Property, ID_SOUND_LATE3);
    addNode(sound2, "400msec", Property, ID_SOUND_LATE4);
    addNode(sound2, "Realtime Mix", Property, ID_SOUND_STRICT_RENDER);
    addNode(sound2, "Light Weight Mix", Property, ID_SOUND_LIGHT_RENDER);
    addNode(sound2, "Volume", Property, ID_SOUND_VOLUME);

    int input = addNode(host, "Input", Category, -1);
    addNode(input, "Joystick #1", Property, ID_INPUT_JOYSTICK0);
    addNode(input, "Joystick #2", Property, ID_INPUT_JOYSTICK1);
    addNode(input, "Joystick To Keyboard", Property, ID_INPUT_JOYTOKEY);

    addNode(host, "Use Direct2D1", Property, ID_HOST_USE_D2D1);
    addNode(host, "Use Direct3D9", Property, ID_HOST_USE_D3D9);
    addNode(host, "Wait Vsync", Property, ID_HOST_WAIT_VSYNC);
    addNode(host, "Use DirectInput", Property, ID_HOST_USE_DINPUT);
    addNode(host, "Disable Windows 8 DWM", Property, ID_HOST_DISABLE_DWM);
    addNode(host, "Show Status Bar", Property, ID_HOST_SHOW_STATUS_BAR);
}
