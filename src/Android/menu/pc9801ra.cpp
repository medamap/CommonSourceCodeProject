//
// Created by medamap on 2024/04/06.
//

#include "menu.h"
#include "BaseMenu.h"
#include "../../res/resource.h"

// BaseMenu を継承して Menu クラスを作成する
Menu::Menu() {

    // Root メニューを作成
    int rootId = addNode(0, "Root", Category, -1);
    int controlId = addNode(rootId, "Control", Category, -1);

    // Control メニューを作成
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
    int loadStateId = addNode(controlId, "Load State", Category, -1);
    for (int i = 0; i < 10; i++) {
        addNode(saveStateId, "State " + std::to_string(i), Property, ID_SAVE_STATE0 + i);
        addNode(loadStateId, "State " + std::to_string(i), Property, ID_LOAD_STATE0 + i);
    }
    addNode(controlId, "Debug Main CPU", Property, ID_OPEN_DEBUGGER0);
    addNode(controlId, "Close Debugger", Property, ID_CLOSE_DEBUGGER);
    addNode(controlId, "Exit", Property, ID_EXIT);

    int fd1Id = addNode(rootId, "FD1", Category, -1, MENU_FDD0);
    int fd2Id = addNode(rootId, "FD2", Category, -1, MENU_FDD1);
    int hd1Id = addNode(rootId, "HD1", Category, -1, MENU_HDD0);
    int hd2Id = addNode(rootId, "HD2", Category, -1, MENU_HDD1);
    int deviceId = addNode(rootId, "Device", Category, -1);
    int hostId = addNode(rootId, "Host", Category, -1);

    // FD1 メニューを作成
    addNode(fd1Id, "Insert", Property, ID_OPEN_FD1);
    addNode(fd1Id, "Eject", Property, ID_CLOSE_FD1);
    addNode(fd1Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD1);
    addNode(fd1Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD1);
    addNode(fd1Id, "Write Protected", Property, ID_WRITE_PROTECT_FD1);
    addNode(fd1Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD1);
    addNode(fd1Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD1);
    addNode(fd1Id, "Recent", Property, ID_RECENT_FD1);

    // FD2 メニューを作成
    addNode(fd2Id, "Insert", Property, ID_OPEN_FD2);
    addNode(fd2Id, "Eject", Property, ID_CLOSE_FD2);
    addNode(fd2Id, "Insert Blank 2DD Disk", Property, ID_OPEN_BLANK_2DD_FD2);
    addNode(fd2Id, "Insert Blank 2HD Disk", Property, ID_OPEN_BLANK_2HD_FD2);
    addNode(fd2Id, "Write Protected", Property, ID_WRITE_PROTECT_FD2);
    addNode(fd2Id, "Correct Timing", Property, ID_CORRECT_TIMING_FD2);
    addNode(fd2Id, "Ignore CRC Errors", Property, ID_IGNORE_CRC_FD2);
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
    int cpuId = addNode(deviceId, "CPU", Category, -1);
    int dipswitchId = addNode(deviceId, "DIP Switch", Category, -1);
    int soundId = addNode(deviceId, "Sound", Category, -1);
    int displayId = addNode(deviceId, "Display", Category, -1);
    int printerId = addNode(deviceId, "Printer", Category, -1);
    int serialId = addNode(deviceId, "Serial", Category, -1);

    addNode(cpuId, "80386 20MHz", Property, ID_VM_CPU_TYPE0);
    addNode(cpuId, "80386 16MHz", Property, ID_VM_CPU_TYPE1);
    addNode(cpuId, "V30 10MHz", Property, ID_VM_CPU_TYPE2);
    addNode(cpuId, "V30 8MHz", Property, ID_VM_CPU_TYPE3);

    addNode(dipswitchId, "SW 2-1", Property, ID_VM_DIPSWITCH16);
    addNode(dipswitchId, "SW 2-2 (Terminal)", Property, ID_VM_DIPSWITCH17);
    addNode(dipswitchId, "SW 2-3 (80 Columns)", Property, ID_VM_DIPSWITCH18);
    addNode(dipswitchId, "SW 2-4 (25 Lines)", Property, ID_VM_DIPSWITCH19);
    addNode(dipswitchId, "SW 2-5 (Hold Memory Switch)", Property, ID_VM_DIPSWITCH20);
    addNode(dipswitchId, "SW 2-6 (Disable HDD)", Property, ID_VM_DIPSWITCH21);
    addNode(dipswitchId, "SW 2-7", Property, ID_VM_DIPSWITCH22);
    addNode(dipswitchId, "SW 2-8 (GDC 5MHz)", Property, ID_VM_DIPSWITCH23);

    addNode(soundId, "PC-9801-86 (BIOS Enabled)", Property, ID_VM_SOUND_TYPE0);
    addNode(soundId, "PC-9801-86 (BIOS Disabled)", Property, ID_VM_SOUND_TYPE1);
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
    int hostRecId = addNode(hostId, "Rec", Category, -1);
    int hostScreenId = addNode(hostId, "Screen", Category, -1);
    int hostFilterId = addNode(hostId, "Filter", Category, -1);
    int hostSoundId = addNode(hostId, "Sound", Category, -1);
    int hostInputId = addNode(hostId, "Input", Category, -1);

    addNode(hostRecId, "Rec Movie 60fps", Property, ID_HOST_REC_MOVIE_60FPS);
    addNode(hostRecId, "Rec Movie 30fps", Property, ID_HOST_REC_MOVIE_30FPS);
    addNode(hostRecId, "Rec Movie 15fps", Property, ID_HOST_REC_MOVIE_15FPS);
    addNode(hostRecId, "Rec Sound", Property, ID_HOST_REC_SOUND);
    addNode(hostRecId, "Stop", Property, ID_HOST_REC_STOP);
    addNode(hostRecId, "Capture Screen", Property, ID_HOST_CAPTURE_SCREEN);

    addNode(hostScreenId, "Rotate 0deg", Property, ID_SCREEN_ROTATE_0);
    addNode(hostScreenId, "Rotate +90deg", Property, ID_SCREEN_ROTATE_90);
    addNode(hostScreenId, "Rotate 180deg", Property, ID_SCREEN_ROTATE_180);
    addNode(hostScreenId, "Rotate -90deg", Property, ID_SCREEN_ROTATE_270);

    int iconSize1 = addNode(screenId, "H System Icon Size", Category, -1);
    addNode(iconSize1, "12", Property, ID_SCREEN_HS_ICON_SIZE_12);
    addNode(iconSize1, "19", Property, ID_SCREEN_HS_ICON_SIZE_19);
    addNode(iconSize1, "26", Property, ID_SCREEN_HS_ICON_SIZE_26);
    addNode(iconSize1, "33", Property, ID_SCREEN_HS_ICON_SIZE_33);
    addNode(iconSize1, "40", Property, ID_SCREEN_HS_ICON_SIZE_40);
    addNode(iconSize1, "47", Property, ID_SCREEN_HS_ICON_SIZE_47);
    addNode(iconSize1, "54", Property, ID_SCREEN_HS_ICON_SIZE_54);
    addNode(iconSize1, "61", Property, ID_SCREEN_HS_ICON_SIZE_61);

    int iconSize2 = addNode(screenId, "H File Icon Size", Category, -1);
    addNode(iconSize2, "12", Property, ID_SCREEN_HF_ICON_SIZE_12);
    addNode(iconSize2, "19", Property, ID_SCREEN_HF_ICON_SIZE_19);
    addNode(iconSize2, "26", Property, ID_SCREEN_HF_ICON_SIZE_26);
    addNode(iconSize2, "33", Property, ID_SCREEN_HF_ICON_SIZE_33);
    addNode(iconSize2, "40", Property, ID_SCREEN_HF_ICON_SIZE_40);
    addNode(iconSize2, "47", Property, ID_SCREEN_HF_ICON_SIZE_47);
    addNode(iconSize2, "54", Property, ID_SCREEN_HF_ICON_SIZE_54);
    addNode(iconSize2, "61", Property, ID_SCREEN_HF_ICON_SIZE_61);

    int iconSize3 = addNode(screenId, "V System Icon Size", Category, -1);
    addNode(iconSize3, "12", Property, ID_SCREEN_VS_ICON_SIZE_12);
    addNode(iconSize3, "19", Property, ID_SCREEN_VS_ICON_SIZE_19);
    addNode(iconSize3, "26", Property, ID_SCREEN_VS_ICON_SIZE_26);
    addNode(iconSize3, "33", Property, ID_SCREEN_VS_ICON_SIZE_33);
    addNode(iconSize3, "40", Property, ID_SCREEN_VS_ICON_SIZE_40);
    addNode(iconSize3, "47", Property, ID_SCREEN_VS_ICON_SIZE_47);
    addNode(iconSize3, "54", Property, ID_SCREEN_VS_ICON_SIZE_54);
    addNode(iconSize3, "61", Property, ID_SCREEN_VS_ICON_SIZE_61);

    int iconSize4 = addNode(screenId, "V File Icon Size", Category, -1);
    addNode(iconSize4, "12", Property, ID_SCREEN_VF_ICON_SIZE_12);
    addNode(iconSize4, "19", Property, ID_SCREEN_VF_ICON_SIZE_19);
    addNode(iconSize4, "26", Property, ID_SCREEN_VF_ICON_SIZE_26);
    addNode(iconSize4, "33", Property, ID_SCREEN_VF_ICON_SIZE_33);
    addNode(iconSize4, "40", Property, ID_SCREEN_VF_ICON_SIZE_40);
    addNode(iconSize4, "47", Property, ID_SCREEN_VF_ICON_SIZE_47);
    addNode(iconSize4, "54", Property, ID_SCREEN_VF_ICON_SIZE_54);
    addNode(iconSize4, "61", Property, ID_SCREEN_VF_ICON_SIZE_61);

    int marginId1 = addNode(hostScreenId, "Screen Top Margin", Category, -1);
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

    int marginId2 = addNode(hostScreenId, "Screen Bottom Margin", Category, -1);
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

    addNode(hostSoundId, "Switch On / Off", Property, ID_SOUND_ON);
    addNode(hostSoundId, "2000Hz", Property, ID_SOUND_FREQ0);
    addNode(hostSoundId, "4000Hz", Property, ID_SOUND_FREQ1);
    addNode(hostSoundId, "8000Hz", Property, ID_SOUND_FREQ2);
    addNode(hostSoundId, "11025Hz", Property, ID_SOUND_FREQ3);
    addNode(hostSoundId, "22050Hz", Property, ID_SOUND_FREQ4);
    addNode(hostSoundId, "44100Hz", Property, ID_SOUND_FREQ5);
    addNode(hostSoundId, "55467Hz", Property, ID_SOUND_FREQ6);
    addNode(hostSoundId, "96000Hz", Property, ID_SOUND_FREQ7);
    addNode(hostSoundId, "50msec", Property, ID_SOUND_LATE0);
    addNode(hostSoundId, "100msec", Property, ID_SOUND_LATE1);
    addNode(hostSoundId, "200msec", Property, ID_SOUND_LATE2);
    addNode(hostSoundId, "300msec", Property, ID_SOUND_LATE3);
    addNode(hostSoundId, "400msec", Property, ID_SOUND_LATE4);
    addNode(hostSoundId, "Realtime Mix", Property, ID_SOUND_STRICT_RENDER);
    addNode(hostSoundId, "Light Weight Mix", Property, ID_SOUND_LIGHT_RENDER);
    addNode(hostSoundId, "Volume", Property, ID_SOUND_VOLUME);

    addNode(hostInputId, "Joystick #1", Property, ID_INPUT_JOYSTICK0);
    addNode(hostInputId, "Joystick #2", Property, ID_INPUT_JOYSTICK1);
    addNode(hostInputId, "Joystick To Keyboard", Property, ID_INPUT_JOYTOKEY);

    addNode(hostId, "Use Direct2D1", Property, ID_HOST_USE_D2D1);
    addNode(hostId, "Use Direct3D9", Property, ID_HOST_USE_D3D9);
    addNode(hostId, "Wait Vsync", Property, ID_HOST_WAIT_VSYNC);
    addNode(hostId, "Use DirectInput", Property, ID_HOST_USE_DINPUT);
    addNode(hostId, "Disable Windows 8 DWM", Property, ID_HOST_DISABLE_DWM);
    addNode(hostId, "Show Status Bar", Property, ID_HOST_SHOW_STATUS_BAR);
}
