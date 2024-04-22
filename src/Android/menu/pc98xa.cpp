//
// Created by medamap on 2024/04/08.
//

#include "menu.h"
#include "BaseMenu.h"
#include "../../res/resource.h"

// BaseMenu を継承して Menu クラスを作成する
Menu::Menu() {

    // Root メニューを作成
    int rootId = addNode(0, "Root", Category, -1);
    int controlId = addNode(rootId, "Control", Category, -1);
    int fd1Id = addNode(rootId, "FD1", Category, -1, MENU_FDD0);
    int fd2Id = addNode(rootId, "FD2", Category, -1, MENU_FDD1);
    int hd1Id = addNode(rootId, "HD1", Category, -1, MENU_HDD0);
    int hd2Id = addNode(rootId, "HD2", Category, -1, MENU_HDD1);
    int device = addNode(rootId, "Device", Category, -1);
    int hostId = addNode(rootId, "Host", Category, -1);

    // Control メニューを作成
    addNode(controlId, "Reset", Property, ID_RESET);
    addNode(controlId, "NMI", Property, ID_SPECIAL_RESET);
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
    addNode(controlId, "Debug Sub CPU", Property, ID_OPEN_DEBUGGER1);
    addNode(controlId, "Debug Keyboard CPU", Property, ID_OPEN_DEBUGGER2);
    addNode(controlId, "Close Debugger", Property, ID_CLOSE_DEBUGGER);
    addNode(controlId, "Exit", Property, ID_EXIT);

    // FD1 メニューを作成
    addNode(fd1Id, "Insert", Property, ID_OPEN_FD1);
    addNode(fd1Id, "Eject", Property, ID_CLOSE_FD1);
    addNode(fd1Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD1);
    addNode(fd1Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD1);
    addNode(fd1Id, "Recent", Property, ID_RECENT_FD1);

    // FD2 メニューを作成
    addNode(fd2Id, "Insert", Property, ID_OPEN_FD2);
    addNode(fd2Id, "Eject", Property, ID_CLOSE_FD2);
    addNode(fd2Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD2);
    addNode(fd2Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD2);
    addNode(fd2Id, "Recent", Property, ID_RECENT_FD2);

    // HD1 メニューを作成
    addNode(hd1Id, "Mount", Property, ID_OPEN_HD1);
    addNode(hd1Id, "Unmount", Property, ID_CLOSE_HD1);
    addNode(hd1Id, "Mount Blank 20MB Disk", Property, ID_OPEN_BLANK_20MB_HD1);
    addNode(hd1Id, "Mount Blank 40MB Disk", Property, ID_OPEN_BLANK_40MB_HD1);
    addNode(hd1Id, "Recent", Property, ID_RECENT_HD1);

    // HD2 メニューを作成
    addNode(hd2Id, "Mount", Property, ID_OPEN_HD2);
    addNode(hd2Id, "Unmount", Property, ID_CLOSE_HD2);
    addNode(hd2Id, "Mount Blank 20MB Disk", Property, ID_OPEN_BLANK_20MB_HD2);
    addNode(hd2Id, "Mount Blank 40MB Disk", Property, ID_OPEN_BLANK_40MB_HD2);
    addNode(hd2Id, "Recent", Property, ID_RECENT_HD2);

    // Device メニューを作成
    int dipSwitchId = addNode(device, "DIP Switch", Category, -1);
    int soundId = addNode(device, "Sound", Category, -1);
    int displayId = addNode(device, "Display", Category, -1);
    int printerId = addNode(device, "Printer", Category, -1);
    int serialId = addNode(device, "Serial", Category, -1);

    addNode(dipSwitchId, "SW 1-1", Property, ID_VM_DIPSWITCH16);
    addNode(dipSwitchId, "SW 1-2", Property, ID_VM_DIPSWITCH17);
    addNode(dipSwitchId, "SW 1-3", Property, ID_VM_DIPSWITCH18);
    addNode(dipSwitchId, "SW 1-4", Property, ID_VM_DIPSWITCH19);
    addNode(dipSwitchId, "SW 1-5 (Hold Memory Switch)", Property, ID_VM_DIPSWITCH20);
    addNode(dipSwitchId, "SW 1-6", Property, ID_VM_DIPSWITCH21);
    addNode(dipSwitchId, "SW 1-7", Property, ID_VM_DIPSWITCH22);
    addNode(dipSwitchId, "SW 1-8", Property, ID_VM_DIPSWITCH23);

    addNode(soundId, "PC-9801-26 (BIOS Enabled)", Property, ID_VM_SOUND_TYPE0);
    addNode(soundId, "PC-9801-26 (BIOS Disabled)", Property, ID_VM_SOUND_TYPE1);
    addNode(soundId, "None", Property, ID_VM_SOUND_TYPE4);
    addNode(soundId, "Play FDD Noise", Property, ID_VM_SOUND_NOISE_FDD);

    addNode(displayId, "High Resolution", Property, ID_VM_MONITOR_TYPE0);
    addNode(displayId, "Standard", Property, ID_VM_MONITOR_TYPE1);
    addNode(displayId, "Scanline", Property, ID_VM_MONITOR_SCANLINE);

    addNode(printerId, "Write Printer to File", Property, ID_VM_PRINTER_TYPE0);
    addNode(printerId, "PC-PR201", Property, ID_VM_PRINTER_TYPE1);
    addNode(printerId, "None", Property, ID_VM_PRINTER_TYPE2);

    addNode(serialId, "Physical Comm Port", Property, ID_VM_SERIAL_TYPE0);
    addNode(serialId, "Named Pipe", Property, ID_VM_SERIAL_TYPE1);
    addNode(serialId, "MIDI Device", Property, ID_VM_SERIAL_TYPE2);
    addNode(serialId, "None", Property, ID_VM_SERIAL_TYPE3);

    // Host メニューを作成
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

    int marginId1 = addNode(screenId, "Screen Top Margin", Category, -1);
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

    int marginId2 = addNode(screenId, "Screen Bottom Margin", Category, -1);
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

    int filterId = addNode(hostId, "Filter", Category, -1);
    addNode(filterId, "Green Filter", Property, ID_FILTER_GREEN);
    addNode(filterId, "RGB Filter", Property, ID_FILTER_RGB);
    addNode(filterId, "Blur Filter", Property, ID_FILTER_BLUR);
    addNode(filterId, "DOT", Property, ID_FILTER_DOT);
    addNode(filterId, "None", Property, ID_FILTER_NONE);

    int soundId2 = addNode(hostId, "Sound", Category, -1);
    addNode(soundId2, "Switch On / Off", Property, ID_SOUND_ON);
    addNode(soundId2, "2000Hz", Property, ID_SOUND_FREQ0);
    addNode(soundId2, "4000Hz", Property, ID_SOUND_FREQ1);
    addNode(soundId2, "8000Hz", Property, ID_SOUND_FREQ2);
    addNode(soundId2, "11025Hz", Property, ID_SOUND_FREQ3);
    addNode(soundId2, "22050Hz", Property, ID_SOUND_FREQ4);
    addNode(soundId2, "44100Hz", Property, ID_SOUND_FREQ5);
    addNode(soundId2, "62500Hz", Property, ID_SOUND_FREQ6);
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
