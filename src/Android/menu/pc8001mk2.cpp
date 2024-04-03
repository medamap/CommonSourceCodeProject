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
    addNode(controlId, "Drive VM in M1/R/W Cycle", Property, ID_DRIVE_VM_IN_OPECODE);
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
    addNode(controlId, "Debug PC-80S31K CPU", Property, ID_OPEN_DEBUGGER1);
    addNode(controlId, "Close Debugger", Property, ID_CLOSE_DEBUGGER);
    addNode(controlId, "Exit", Property, ID_EXIT);
    int fd1Id = addNode(rootId, "5inch-FD1", Category, -1);
    addNode(fd1Id, "Insert", Property, ID_OPEN_FD1);
    addNode(fd1Id, "Eject", Property, ID_CLOSE_FD1);
    addNode(fd1Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD1);
    addNode(fd1Id, "Write Protected", Property, ID_WRITE_PROTECT_FD1);
    addNode(fd1Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD1);
    addNode(fd1Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD1);
    addNode(fd1Id, "Recent", Property, ID_RECENT_FD1);
    int fd2Id = addNode(rootId, "FD2", Category, -1);
    addNode(fd2Id, "Insert", Property, ID_OPEN_FD2);
    addNode(fd2Id, "Eject", Property, ID_CLOSE_FD2);
    addNode(fd2Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD2);
    addNode(fd2Id, "Write Protected", Property, ID_WRITE_PROTECT_FD2);
    addNode(fd2Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD2);
    addNode(fd2Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD2);
    addNode(fd2Id, "Recent", Property, ID_RECENT_FD2);
    int fd3Id = addNode(rootId, "8inch-FD1", Category, -1);
    addNode(fd3Id, "Insert", Property, ID_OPEN_FD3);
    addNode(fd3Id, "Eject", Property, ID_CLOSE_FD3);
    addNode(fd3Id, "Insert Blank 8inch-2D Disk", Property, ID_OPEN_BLANK_2HD_FD3);
    addNode(fd3Id, "Write Protected", Property, ID_WRITE_PROTECT_FD3);
    addNode(fd3Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD3);
    addNode(fd3Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD3);
    addNode(fd3Id, "Recent", Property, ID_RECENT_FD3);
    int fd4Id = addNode(rootId, "FD4", Category, -1);
    addNode(fd4Id, "Insert", Property, ID_OPEN_FD4);
    addNode(fd4Id, "Eject", Property, ID_CLOSE_FD4);
    addNode(fd4Id, "Insert Blank 8inch-2D Disk", Property, ID_OPEN_BLANK_2HD_FD4);
    addNode(fd4Id, "Write Protected", Property, ID_WRITE_PROTECT_FD4);
    addNode(fd4Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD4);
    addNode(fd4Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD4);
    addNode(fd4Id, "Recent", Property, ID_RECENT_FD4);

    int cmtId = addNode(rootId, "CMT", Category, -1);
    addNode(cmtId, "Play", Property, ID_PLAY_TAPE1);
    addNode(cmtId, "Rec", Property, ID_REC_TAPE1);
    addNode(cmtId, "Eject", Property, ID_CLOSE_TAPE1);
    addNode(cmtId, "Recent", Property, ID_RECENT_TAPE1);

    int deviceId = addNode(rootId, "Device", Category, -1);
    int bootId = addNode(deviceId, "Boot", Category, -1);
    addNode(bootId, "N80 mode", Property, ID_VM_BOOT_MODE0);
    addNode(bootId, "N mode", Property, ID_VM_BOOT_MODE2);
    int fdId = addNode(deviceId, "Floppy Drive", Category, -1);
    addNode(fdId, "5inch FD", Property, ID_VM_DIPSWITCH6);
    addNode(fdId, "8inch FD", Property, ID_VM_DIPSWITCH7);
    int soundId = addNode(deviceId, "Sound", Category, -1);
    addNode(soundId, "None", Property, ID_VM_SOUND_TYPE0);
    addNode(soundId, "OPN", Property, ID_VM_SOUND_TYPE1);
    addNode(soundId, "GSX-8800", Property, ID_VM_DIPSWITCH2);
    addNode(soundId, "PCG-8100", Property, ID_VM_DIPSWITCH3);
    addNode(soundId, "Play FDD Noise", Property, ID_VM_SOUND_NOISE_FDD);
    addNode(soundId, "Play CMT Noise", Property, ID_VM_SOUND_NOISE_CMT);
    addNode(soundId, "Play CMT Signal", Property, ID_VM_SOUND_TAPE_SIGNAL);
    addNode(soundId, "Play CMT Voice", Property, ID_VM_SOUND_TAPE_VOICE);
    int displayId = addNode(deviceId, "Display", Category, -1);
    addNode(displayId, "Scanline", Property, ID_VM_MONITOR_SCANLINE);
    addNode(displayId, "Ignore Palette Changed Outside VBLANK", Property, ID_VM_DIPSWITCH5);
    int printerId = addNode(deviceId, "Printer", Category, -1);
    addNode(printerId, "Write Printer to File", Property, ID_VM_PRINTER_TYPE0);
    addNode(printerId, "PC-PR201", Property, ID_VM_PRINTER_TYPE1);
    addNode(printerId, "None", Property, ID_VM_PRINTER_TYPE2);
    int specialId = addNode(deviceId, "Special Features", Category, -1);
    addNode(specialId, "M88 DiskDrv", Property, ID_VM_DIPSWITCH8);
    addNode(specialId, "QUASIS88 Pesudo CMT", Property, ID_VM_DIPSWITCH9);

    int hostId = addNode(rootId, "Host", Category, -1);
    addNode(hostId, "Rec Movie 60fps", Property, ID_HOST_REC_MOVIE_60FPS);
    addNode(hostId, "Rec Movie 30fps", Property, ID_HOST_REC_MOVIE_30FPS);
    addNode(hostId, "Rec Movie 15fps", Property, ID_HOST_REC_MOVIE_15FPS);
    addNode(hostId, "Rec Sound", Property, ID_HOST_REC_SOUND);
    addNode(hostId, "Stop", Property, ID_HOST_REC_STOP);
    addNode(hostId, "Capture Screen", Property, ID_HOST_CAPTURE_SCREEN);
    int screenId = addNode(hostId, "Screen", Category, -1);
    addNode(screenId, "Window x1", Property, ID_SCREEN_WINDOW);
    addNode(screenId, "Fullscreen 640x400", Property, ID_SCREEN_FULLSCREEN);
    addNode(screenId, "Window Stretch 1", Property, ID_SCREEN_WINDOW_STRETCH);
    addNode(screenId, "Window Stretch 2", Property, ID_SCREEN_WINDOW_ASPECT);
    addNode(screenId, "Fullscreen Stretch 1", Property, ID_SCREEN_FULLSCREEN_DOTBYDOT);
    addNode(screenId, "Fullscreen Stretch 2", Property, ID_SCREEN_FULLSCREEN_STRETCH);
    addNode(screenId, "Fullscreen Stretch 3", Property, ID_SCREEN_FULLSCREEN_ASPECT);
    addNode(screenId, "Fullscreen Stretch 4", Property, ID_SCREEN_FULLSCREEN_FILL);
    addNode(screenId, "Rotate 0deg", Property, ID_SCREEN_ROTATE_0);
    addNode(screenId, "Rotate +90deg", Property, ID_SCREEN_ROTATE_90);
    addNode(screenId, "Rotate 180deg", Property, ID_SCREEN_ROTATE_180);
    addNode(screenId, "Rotate -90deg", Property, ID_SCREEN_ROTATE_270);

    int marginId = addNode(screenId, "Screen Bottom Margin", Category, -1);
    addNode(marginId, "0", Property, ID_SCREEN_BOTTOM_MARGIN_0);
    addNode(marginId, "30", Property, ID_SCREEN_BOTTOM_MARGIN_30);
    addNode(marginId, "60", Property, ID_SCREEN_BOTTOM_MARGIN_60);
    addNode(marginId, "90", Property, ID_SCREEN_BOTTOM_MARGIN_90);
    addNode(marginId, "120", Property, ID_SCREEN_BOTTOM_MARGIN_120);
    addNode(marginId, "150", Property, ID_SCREEN_BOTTOM_MARGIN_150);
    addNode(marginId, "180", Property, ID_SCREEN_BOTTOM_MARGIN_180);
    addNode(marginId, "210", Property, ID_SCREEN_BOTTOM_MARGIN_210);
    addNode(marginId, "240", Property, ID_SCREEN_BOTTOM_MARGIN_240);
    addNode(marginId, "270", Property, ID_SCREEN_BOTTOM_MARGIN_270);

    int filterId = addNode(hostId, "Filter", Category, -1);
    addNode(filterId, "RGB Filter", Property, ID_FILTER_RGB);
    addNode(filterId, "None", Property, ID_FILTER_NONE);
    int soundId2 = addNode(hostId, "Sound", Category, -1);
    addNode(soundId2, "Switch On / Off", Property, ID_SOUND_ON);
    addNode(soundId2, "2000Hz", Property, ID_SOUND_FREQ0);
    addNode(soundId2, "4000Hz", Property, ID_SOUND_FREQ1);
    addNode(soundId2, "8000Hz", Property, ID_SOUND_FREQ2);
    addNode(soundId2, "11025Hz", Property, ID_SOUND_FREQ3);
    addNode(soundId2, "22050Hz", Property, ID_SOUND_FREQ4);
    addNode(soundId2, "44100Hz", Property, ID_SOUND_FREQ5);
    addNode(soundId2, "55467Hz", Property, ID_SOUND_FREQ6);
    addNode(soundId2, "96000Hz", Property, ID_SOUND_FREQ7);
    addNode(soundId2, "50msec", Property, ID_SOUND_LATE0);
    addNode(soundId2, "100msec", Property, ID_SOUND_LATE1);
    addNode(soundId2, "200msec", Property, ID_SOUND_LATE2);
    addNode(soundId2, "300msec", Property, ID_SOUND_LATE3);
    addNode(soundId2, "400msec", Property, ID_SOUND_LATE4);
    addNode(soundId2, "Realtime Mix", Property, ID_SOUND_STRICT_RENDER);
    addNode(soundId2, "Light Weight Mix", Property, ID_SOUND_LIGHT_RENDER);
    addNode(soundId2, "Volume", Property, ID_SOUND_VOLUME);
    int inputId = addNode(hostId, "Input", Category, -1);
    addNode(inputId, "Joystick #1", Property, ID_INPUT_JOYSTICK0);
    addNode(inputId, "Joystick #2", Property, ID_INPUT_JOYSTICK1);
    addNode(inputId, "Joystick To Keyboard", Property, ID_INPUT_JOYTOKEY);
    addNode(hostId, "Use Direct2D1", Property, ID_HOST_USE_D2D1);
    addNode(hostId, "Use Direct3D9", Property, ID_HOST_USE_D3D9);
    addNode(hostId, "Wait Vsync", Property, ID_HOST_WAIT_VSYNC);
    addNode(hostId, "Use DirectInput", Property, ID_HOST_USE_DINPUT);
    addNode(hostId, "Disable Windows 8 DWM", Property, ID_HOST_DISABLE_DWM);
    addNode(hostId, "Show Status Bar", Property, ID_HOST_SHOW_STATUS_BAR);
}
