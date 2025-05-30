//
// Created by medamap on 2024/04/05.
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
    addNode(controlId, "Debug PC-98 Main CPU", Property, ID_OPEN_DEBUGGER0);
    addNode(controlId, "Debug PC-88 Main CPU", Property, ID_OPEN_DEBUGGER1);
    addNode(controlId, "Debug PC-88 Sub CPU", Property, ID_OPEN_DEBUGGER2);
    addNode(controlId, "Close Debugger", Property, ID_CLOSE_DEBUGGER);
    addNode(controlId, "Exit", Property, ID_EXIT);

    int pc98Fd1Id = addNode(rootId, "PC98-FD1", Category, -1, MENU_FDD0);
    addNode(pc98Fd1Id, "Insert", Property, ID_OPEN_FD1);
    addNode(pc98Fd1Id, "Eject", Property, ID_CLOSE_FD1);
    addNode(pc98Fd1Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD1);
    addNode(pc98Fd1Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD1);
    addNode(pc98Fd1Id, "Write Protected", Property, ID_WRITE_PROTECT_FD1);
    addNode(pc98Fd1Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD1);
    addNode(pc98Fd1Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD1);
    addNode(pc98Fd1Id, "Recent", Property, ID_RECENT_FD1);

    int pc98Fd2Id = addNode(rootId, "PC98-FD2", Category, -1, MENU_FDD1);
    addNode(pc98Fd2Id, "Insert", Property, ID_OPEN_FD2);
    addNode(pc98Fd2Id, "Eject", Property, ID_CLOSE_FD2);
    addNode(pc98Fd2Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD2);
    addNode(pc98Fd2Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD2);
    addNode(pc98Fd2Id, "Write Protected", Property, ID_WRITE_PROTECT_FD2);
    addNode(pc98Fd2Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD2);
    addNode(pc98Fd2Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD2);
    addNode(pc98Fd2Id, "Recent", Property, ID_RECENT_FD2);

    int pc88Fd1Id = addNode(rootId, "PC88-FD1", Category, -1, MENU_FDD2);
    addNode(pc88Fd1Id, "Insert", Property, ID_OPEN_FD3);
    addNode(pc88Fd1Id, "Eject", Property, ID_CLOSE_FD3);
    addNode(pc88Fd1Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD3);
    addNode(pc88Fd1Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD3);
    addNode(pc88Fd1Id, "Write Protected", Property, ID_WRITE_PROTECT_FD3);
    addNode(pc88Fd1Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD3);
    addNode(pc88Fd1Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD3);
    addNode(pc88Fd1Id, "Recent", Property, ID_RECENT_FD3);

    int pc88Fd2Id = addNode(rootId, "PC88-FD2", Category, -1, MENU_FDD3);
    addNode(pc88Fd2Id, "Insert", Property, ID_OPEN_FD4);
    addNode(pc88Fd2Id, "Eject", Property, ID_CLOSE_FD4);
    addNode(pc88Fd2Id, "Insert Blank 2D Disk", Property, ID_OPEN_BLANK_2D_FD4);
    addNode(pc88Fd2Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD4);
    addNode(pc88Fd2Id, "Write Protected", Property, ID_WRITE_PROTECT_FD4);
    addNode(pc88Fd2Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD4);
    addNode(pc88Fd2Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD4);
    addNode(pc88Fd2Id, "Recent", Property, ID_RECENT_FD4);

    int pc88CmtId = addNode(rootId, "PC88-CMT", Category, -1, MENU_TAPE0);
    addNode(pc88CmtId, "Play", Property, ID_PLAY_TAPE1);
    addNode(pc88CmtId, "Rec", Property, ID_REC_TAPE1);
    addNode(pc88CmtId, "Eject", Property, ID_CLOSE_TAPE1);
    addNode(pc88CmtId, "Recent", Property, ID_RECENT_TAPE1);

    int device = addNode(rootId, "Device", Category, -1);
    int boot = addNode(device, "Boot", Category, -1);
    addNode(boot, "PC-98", Property, ID_VM_BOOT_MODE0);
    addNode(boot, "PC-88 V1(S)", Property, ID_VM_BOOT_MODE1);
    addNode(boot, "PC-88 V1(H)", Property, ID_VM_BOOT_MODE2);
    addNode(boot, "PC-88 V2", Property, ID_VM_BOOT_MODE3);
    addNode(boot, "PC-88 N", Property, ID_VM_BOOT_MODE4);

    int cpu = addNode(device, "CPU", Category, -1);
    addNode(cpu, "V30 10MHz / Z80 8MHz", Property, ID_VM_CPU_TYPE0);
    addNode(cpu, "V30  8MHz / Z80 4MHz", Property, ID_VM_CPU_TYPE1);

    int dipSwitch = addNode(device, "DIP Switch", Category, -1);
    addNode(dipSwitch, "PC-98 SW 2-1", Property, ID_VM_DIPSWITCH16);
    addNode(dipSwitch, "PC-98 SW 2-2 (Terminal)", Property, ID_VM_DIPSWITCH17);
    addNode(dipSwitch, "PC-98 SW 2-3 (80 Columns)", Property, ID_VM_DIPSWITCH18);
    addNode(dipSwitch, "PC-98 SW 2-4 (25 Lines)", Property, ID_VM_DIPSWITCH19);
    addNode(dipSwitch, "PC-98 SW 2-5 (Hold Memory Switch)", Property, ID_VM_DIPSWITCH20);
    addNode(dipSwitch, "PC-98 SW 2-6 (Disable HDD)", Property, ID_VM_DIPSWITCH21);
    addNode(dipSwitch, "PC-98 SW 2-7", Property, ID_VM_DIPSWITCH22);
    addNode(dipSwitch, "PC-98 SW 2-8", Property, ID_VM_DIPSWITCH23);
    addNode(dipSwitch, "PC-98 SW 2-8 (GDC 5MHz)", Property, ID_VM_DIPSWITCH23);
    addNode(dipSwitch, "PC-88 Memory Wait", Property, ID_VM_DIPSWITCH0);

    int sound = addNode(device, "Sound", Category, -1);
    addNode(sound, "PC-9801-26 (BIOS Enabled)", Property, ID_VM_SOUND_TYPE0);
    addNode(sound, "PC-9801-26 (BIOS Disabled)", Property, ID_VM_SOUND_TYPE1);
    addNode(sound, "PC-9801-14 (BIOS Enabled)", Property, ID_VM_SOUND_TYPE2);
    addNode(sound, "PC-9801-14 (BIOS Disabled)", Property, ID_VM_SOUND_TYPE3);
    addNode(sound, "None", Property, ID_VM_SOUND_TYPE4);
    addNode(sound, "Play FDD Noise", Property, ID_VM_SOUND_NOISE_FDD);
    addNode(sound, "Play CMT Noise", Property, ID_VM_SOUND_NOISE_CMT);
    addNode(sound, "Play CMT Signal", Property, ID_VM_SOUND_TAPE_SIGNAL);
    addNode(sound, "Play CMT Voice", Property, ID_VM_SOUND_TAPE_VOICE);

    int display = addNode(device, "Display", Category, -1);
    addNode(display, "High Resolution", Property, ID_VM_MONITOR_TYPE0);
    addNode(display, "Standard", Property, ID_VM_MONITOR_TYPE1);
    addNode(display, "Set Scanline Automatically", Property, ID_VM_MONITOR_SCANLINE_AUTO);
    addNode(display, "Scanline", Property, ID_VM_MONITOR_SCANLINE);
    addNode(display, "Ignore Palette Changed Outside VBLANK", Property, ID_VM_DIPSWITCH5);

    int printer = addNode(device, "Printer", Category, -1);
    addNode(printer, "Write Printer to File", Property, ID_VM_PRINTER_TYPE0);
    addNode(printer, "PC-PR201", Property, ID_VM_PRINTER_TYPE1);
    addNode(printer, "JAST SOUND", Property, ID_VM_PRINTER_TYPE2);
    addNode(printer, "None", Property, ID_VM_PRINTER_TYPE3);

    int serial = addNode(device, "Serial", Category, -1);
    addNode(serial, "Physical Comm Port", Property, ID_VM_SERIAL_TYPE0);
    addNode(serial, "Named Pipe", Property, ID_VM_SERIAL_TYPE1);
    addNode(serial, "MIDI Device", Property, ID_VM_SERIAL_TYPE2);
    addNode(serial, "None", Property, ID_VM_SERIAL_TYPE3);

    int specialFeatures = addNode(device, "Special Features", Category, -1);
    addNode(specialFeatures, "M88 DiskDrv", Property, ID_VM_DIPSWITCH8);
    addNode(specialFeatures, "QUASIS88 Pesudo CMT", Property, ID_VM_DIPSWITCH9);

    int host = addNode(rootId, "Host", Category, -1);
    addNode(host, "Rec Movie 60fps", Property, ID_HOST_REC_MOVIE_60FPS);
    addNode(host, "Rec Movie 30fps", Property, ID_HOST_REC_MOVIE_30FPS);
    addNode(host, "Rec Movie 15fps", Property, ID_HOST_REC_MOVIE_15FPS);
    addNode(host, "Rec Sound", Property, ID_HOST_REC_SOUND);
    addNode(host, "Stop", Property, ID_HOST_REC_STOP);
    addNode(host, "Capture Screen", Property, ID_HOST_CAPTURE_SCREEN);

    int mouseId2 = addNode(host, "Mouse", Category, -1);
    addNode(mouseId2, "Mouse Sensitive 10", Property, ID_MOUSE_SENSITIVE_10);
    addNode(mouseId2, "Mouse Sensitive 9", Property, ID_MOUSE_SENSITIVE_9);
    addNode(mouseId2, "Mouse Sensitive 8", Property, ID_MOUSE_SENSITIVE_8);
    addNode(mouseId2, "Mouse Sensitive 7", Property, ID_MOUSE_SENSITIVE_7);
    addNode(mouseId2, "Mouse Sensitive 6", Property, ID_MOUSE_SENSITIVE_6);
    addNode(mouseId2, "Mouse Sensitive 5", Property, ID_MOUSE_SENSITIVE_5);
    addNode(mouseId2, "Mouse Sensitive 4", Property, ID_MOUSE_SENSITIVE_4);
    addNode(mouseId2, "Mouse Sensitive 3", Property, ID_MOUSE_SENSITIVE_3);
    addNode(mouseId2, "Mouse Sensitive 2", Property, ID_MOUSE_SENSITIVE_2);
    addNode(mouseId2, "Mouse Sensitive 1", Property, ID_MOUSE_SENSITIVE_1);
    addNode(mouseId2, "Mouse Sensitive 0", Property, ID_MOUSE_SENSITIVE_0);

    int screen = addNode(host, "Screen", Category, -1);
    addNode(screen, "Rotate 0deg", Property, ID_SCREEN_ROTATE_0);
    addNode(screen, "Rotate +90deg", Property, ID_SCREEN_ROTATE_90);
    addNode(screen, "Rotate 180deg", Property, ID_SCREEN_ROTATE_180);
    addNode(screen, "Rotate -90deg", Property, ID_SCREEN_ROTATE_270);

    int iconSize1 = addNode(screen, "H System Icon Size", Category, -1);
    addNode(iconSize1, "12", Property, ID_SCREEN_HS_ICON_SIZE_12);
    addNode(iconSize1, "19", Property, ID_SCREEN_HS_ICON_SIZE_19);
    addNode(iconSize1, "26", Property, ID_SCREEN_HS_ICON_SIZE_26);
    addNode(iconSize1, "33", Property, ID_SCREEN_HS_ICON_SIZE_33);
    addNode(iconSize1, "40", Property, ID_SCREEN_HS_ICON_SIZE_40);
    addNode(iconSize1, "47", Property, ID_SCREEN_HS_ICON_SIZE_47);
    addNode(iconSize1, "54", Property, ID_SCREEN_HS_ICON_SIZE_54);
    addNode(iconSize1, "61", Property, ID_SCREEN_HS_ICON_SIZE_61);

    int iconSize2 = addNode(screen, "H File Icon Size", Category, -1);
    addNode(iconSize2, "12", Property, ID_SCREEN_HF_ICON_SIZE_12);
    addNode(iconSize2, "19", Property, ID_SCREEN_HF_ICON_SIZE_19);
    addNode(iconSize2, "26", Property, ID_SCREEN_HF_ICON_SIZE_26);
    addNode(iconSize2, "33", Property, ID_SCREEN_HF_ICON_SIZE_33);
    addNode(iconSize2, "40", Property, ID_SCREEN_HF_ICON_SIZE_40);
    addNode(iconSize2, "47", Property, ID_SCREEN_HF_ICON_SIZE_47);
    addNode(iconSize2, "54", Property, ID_SCREEN_HF_ICON_SIZE_54);
    addNode(iconSize2, "61", Property, ID_SCREEN_HF_ICON_SIZE_61);

    int iconSize3 = addNode(screen, "V System Icon Size", Category, -1);
    addNode(iconSize3, "12", Property, ID_SCREEN_VS_ICON_SIZE_12);
    addNode(iconSize3, "19", Property, ID_SCREEN_VS_ICON_SIZE_19);
    addNode(iconSize3, "26", Property, ID_SCREEN_VS_ICON_SIZE_26);
    addNode(iconSize3, "33", Property, ID_SCREEN_VS_ICON_SIZE_33);
    addNode(iconSize3, "40", Property, ID_SCREEN_VS_ICON_SIZE_40);
    addNode(iconSize3, "47", Property, ID_SCREEN_VS_ICON_SIZE_47);
    addNode(iconSize3, "54", Property, ID_SCREEN_VS_ICON_SIZE_54);
    addNode(iconSize3, "61", Property, ID_SCREEN_VS_ICON_SIZE_61);

    int iconSize4 = addNode(screen, "V File Icon Size", Category, -1);
    addNode(iconSize4, "12", Property, ID_SCREEN_VF_ICON_SIZE_12);
    addNode(iconSize4, "19", Property, ID_SCREEN_VF_ICON_SIZE_19);
    addNode(iconSize4, "26", Property, ID_SCREEN_VF_ICON_SIZE_26);
    addNode(iconSize4, "33", Property, ID_SCREEN_VF_ICON_SIZE_33);
    addNode(iconSize4, "40", Property, ID_SCREEN_VF_ICON_SIZE_40);
    addNode(iconSize4, "47", Property, ID_SCREEN_VF_ICON_SIZE_47);
    addNode(iconSize4, "54", Property, ID_SCREEN_VF_ICON_SIZE_54);
    addNode(iconSize4, "61", Property, ID_SCREEN_VF_ICON_SIZE_61);

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
    addNode(filterId, "None", Property, ID_FILTER_NONE);
    addNode(filterId, "DOT Mode", Property, ID_FILTER_DOT);
    addNode(filterId, "SuperImpose Mode", Property, ID_FILTER_SUPERIMPOSE);

    int soundId2 = addNode(host, "Sound", Category, -1);
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
