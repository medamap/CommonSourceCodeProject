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
    int cmtId = addNode(rootId, "CMT", Category, -1, MENU_TAPE0);
    int deviceId = addNode(rootId, "Device", Category, -1);
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

    // CMT メニューを作成
    addNode(cmtId, "Play", Property, ID_PLAY_TAPE1);
    addNode(cmtId, "Rec", Property, ID_REC_TAPE1);
    addNode(cmtId, "Eject", Property, ID_CLOSE_TAPE1);
    addNode(cmtId, "Play Button", Property, ID_PLAY_BUTTON1);
    addNode(cmtId, "Stop Button", Property, ID_STOP_BUTTON1);
    addNode(cmtId, "Fast Forward", Property, ID_FAST_FORWARD1);
    addNode(cmtId, "Fast Rewind", Property, ID_FAST_REWIND1);
    addNode(cmtId, "Waveform Shaper", Property, ID_USE_WAVE_SHAPER1);
    addNode(cmtId, "Recent", Property, ID_RECENT_TAPE1);

    // Device メニューを作成
    int soundId = addNode(deviceId, "Sound", Category, -1);
    addNode(soundId, "Play CMT Noise", Property, ID_VM_SOUND_NOISE_CMT);

    // Host メニューを作成
    addNode(hostId, "Rec Movie 60fps", Property, ID_HOST_REC_MOVIE_60FPS);
    addNode(hostId, "Rec Movie 30fps", Property, ID_HOST_REC_MOVIE_30FPS);
    addNode(hostId, "Rec Movie 15fps", Property, ID_HOST_REC_MOVIE_15FPS);
    addNode(hostId, "Rec Sound", Property, ID_HOST_REC_SOUND);
    addNode(hostId, "Stop", Property, ID_HOST_REC_STOP);
    addNode(hostId, "Capture Screen", Property, ID_HOST_CAPTURE_SCREEN);

    int screenId = addNode(hostId, "Screen", Category, -1);
    int soundId2 = addNode(hostId, "Sound", Category, -1);
    int inputId = addNode(hostId, "Input", Category, -1);

    int mouseId2 = addNode(hostId, "Mouse", Category, -1);
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

    addNode(screenId, "Rotate 0deg", Property, ID_SCREEN_ROTATE_0);
    addNode(screenId, "Rotate +90deg", Property, ID_SCREEN_ROTATE_90);
    addNode(screenId, "Rotate 180deg", Property, ID_SCREEN_ROTATE_180);
    addNode(screenId, "Rotate -90deg", Property, ID_SCREEN_ROTATE_270);

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
    addNode(filterId, "None", Property, ID_FILTER_NONE);
    addNode(filterId, "DOT Mode", Property, ID_FILTER_DOT);
    addNode(filterId, "SuperImpose Mode", Property, ID_FILTER_SUPERIMPOSE);

    addNode(soundId2, "Switch On / Off", Property, ID_SOUND_ON);
    addNode(soundId2, "2000Hz", Property, ID_SOUND_FREQ0);
    addNode(soundId2, "4000Hz", Property, ID_SOUND_FREQ1);
    addNode(soundId2, "8000Hz", Property, ID_SOUND_FREQ2);
    addNode(soundId2, "11025Hz", Property, ID_SOUND_FREQ3);
    addNode(soundId2, "22050Hz", Property, ID_SOUND_FREQ4);
    addNode(soundId2, "44100Hz", Property, ID_SOUND_FREQ5);
    addNode(soundId2, "48000Hz", Property, ID_SOUND_FREQ6);
    addNode(soundId2, "96000Hz", Property, ID_SOUND_FREQ7);
    addNode(soundId2, "50msec", Property, ID_SOUND_LATE0);
    addNode(soundId2, "100msec", Property, ID_SOUND_LATE1);
    addNode(soundId2, "200msec", Property, ID_SOUND_LATE2);
    addNode(soundId2, "300msec", Property, ID_SOUND_LATE3);
    addNode(soundId2, "400msec", Property, ID_SOUND_LATE4);
    addNode(soundId2, "Realtime Mix", Property, ID_SOUND_STRICT_RENDER);
    addNode(soundId2, "Light Weight Mix", Property, ID_SOUND_LIGHT_RENDER);
    addNode(soundId2, "Volume", Property, ID_SOUND_VOLUME);

    addNode(inputId, "Joystick #1", Property, ID_INPUT_JOYSTICK0);
    addNode(inputId, "Joystick #2", Property, ID_INPUT_JOYSTICK1);
    addNode(inputId, "Joystick #3", Property, ID_INPUT_JOYSTICK3);

    addNode(hostId, "Use Direct2D1", Property, ID_HOST_USE_D2D1);
    addNode(hostId, "Use Direct3D9", Property, ID_HOST_USE_D3D9);
    addNode(hostId, "Wait Vsync", Property, ID_HOST_WAIT_VSYNC);
    addNode(hostId, "Use DirectInput", Property, ID_HOST_USE_DINPUT);
    addNode(hostId, "Disable Windows 8 DWM", Property, ID_HOST_DISABLE_DWM);
    addNode(hostId, "Show Status Bar", Property, ID_HOST_SHOW_STATUS_BAR);
}
