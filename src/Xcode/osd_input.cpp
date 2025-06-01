/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode input - Real Implementation ]

    macOS/iOS向けの実際の入力システム実装
    Author : Claude
    Date   : 2025.05.29
*/

#include "osd.h"
#include <cstring>

// Apple系ヘッダーは必要に応じてBridge.mm等で使用
// osd_input.cppでは純粋なC++実装とする

// キーコード変換テーブル（macOS NSEvent -> Windows VK codes）
static const struct {
    unsigned short nsKeyCode;
    int vkCode;
} keyCodeMap[] = {
    // 数字キー
    {29, 0x30}, // 0
    {18, 0x31}, // 1
    {19, 0x32}, // 2
    {20, 0x33}, // 3
    {21, 0x34}, // 4
    {23, 0x35}, // 5
    {22, 0x36}, // 6
    {26, 0x37}, // 7
    {28, 0x38}, // 8
    {25, 0x39}, // 9
    
    // アルファベット
    {0, 0x41},  // A
    {11, 0x42}, // B
    {8, 0x43},  // C
    {2, 0x44},  // D
    {14, 0x45}, // E
    {3, 0x46},  // F
    {5, 0x47},  // G
    {4, 0x48},  // H
    {34, 0x49}, // I
    {38, 0x4A}, // J
    {40, 0x4B}, // K
    {37, 0x4C}, // L
    {46, 0x4D}, // M
    {45, 0x4E}, // N
    {31, 0x4F}, // O
    {35, 0x50}, // P
    {12, 0x51}, // Q
    {15, 0x52}, // R
    {1, 0x53},  // S
    {17, 0x54}, // T
    {32, 0x55}, // U
    {9, 0x56},  // V
    {13, 0x57}, // W
    {7, 0x58},  // X
    {16, 0x59}, // Y
    {6, 0x5A},  // Z
    
    // 特殊キー
    {36, 0x0D}, // Return/Enter
    {48, 0x09}, // Tab
    {49, 0x20}, // Space
    {51, 0x08}, // Backspace
    {53, 0x1B}, // Escape
    
    // 矢印キー
    {123, 0x25}, // Left Arrow
    {124, 0x27}, // Right Arrow
    {126, 0x26}, // Up Arrow
    {125, 0x28}, // Down Arrow
    
    // ファンクションキー
    {122, 0x70}, // F1
    {120, 0x71}, // F2
    {99, 0x72},  // F3
    {118, 0x73}, // F4
    {96, 0x74},  // F5
    {97, 0x75},  // F6
    {98, 0x76},  // F7
    {100, 0x77}, // F8
    {101, 0x78}, // F9
    {109, 0x79}, // F10
    {103, 0x7A}, // F11
    {111, 0x7B}, // F12
    
    // モディファイアキー
    {54, 0x11},  // Right Command (Ctrl)
    {55, 0x11},  // Left Command (Ctrl)
    {56, 0x10},  // Left Shift
    {60, 0x10},  // Right Shift
    {58, 0x12},  // Left Option (Alt)
    {61, 0x12},  // Right Option (Alt)
    {59, 0x14},  // Left Control (Caps Lock equivalent)
    {62, 0x14},  // Right Control
};

static const int keyCodeMapSize = sizeof(keyCodeMap) / sizeof(keyCodeMap[0]);

// NSKeyCodeからWindows VKコードに変換
static int convertNSKeyCodeToVK(unsigned short nsKeyCode) {
    for (int i = 0; i < keyCodeMapSize; i++) {
        if (keyCodeMap[i].nsKeyCode == nsKeyCode) {
            return keyCodeMap[i].vkCode;
        }
    }
    return 0; // 不明なキー
}

void OSD::initialize_input()
{
    printf("入力システムを初期化中...\n");
    
    // キー状態初期化
    memset(key_status, 0, sizeof(key_status));
    
    // ジョイスティック初期化
#ifdef USE_JOYSTICK
    memset(joy_status, 0, sizeof(joy_status));
#endif
    
    // マウス状態初期化
#ifdef USE_MOUSE
    mouse_status[0] = mouse_status[1] = mouse_status[2] = 0;
    mouse_enabled = false;
#endif
    
    // タッチ状態初期化
#ifdef USE_TOUCH
    memset(touch_status, 0, sizeof(touch_status));
#endif

    // プラットフォーム固有の初期化は Bridge.mm 等で実装
    printf("プラットフォーム非依存の入力システム初期化完了\n");

    printf("入力システム初期化完了！\n");
}

void OSD::release_input()
{
    printf("入力システムを解放中...\n");
    
    // キー状態クリア
    memset(key_status, 0, sizeof(key_status));
    
#ifdef USE_JOYSTICK
    memset(joy_status, 0, sizeof(joy_status));
#endif
    
#ifdef USE_MOUSE
    // マウス状態クリア
    mouse_status[0] = mouse_status[1] = mouse_status[2] = 0;
#endif
    
#ifdef USE_TOUCH
    memset(touch_status, 0, sizeof(touch_status));
#endif

    printf("入力システム解放完了\n");
}

void OSD::update_input()
{
    // 定期的な入力状態の更新
    // 実際のイベント処理は各プラットフォーム固有の方法で行う
    
    // デバッグ情報（60フレームごと）
    static int frame_count = 0;
    frame_count++;
    
    if (frame_count % 3600 == 0) { // 60秒ごと
        int pressed_keys = 0;
        for (int i = 0; i < 256; i++) {
            if (key_status[i] & 0x80) {
                pressed_keys++;
            }
        }
        if (pressed_keys > 0) {
            printf("入力状態: %d個のキーが押下中\n", pressed_keys);
        }
    }
}

void OSD::key_down(int code, bool extended, bool repeat)
{
    if (!repeat && code >= 0 && code < 256) {
        key_status[code] = 0x80;
        printf("[DEBUG] キー押下: VKCode=0x%02X, key_status[0x%02X]=0x%02X\n", code, code, key_status[code]);
    }
    
    // [DEBUG_START - claude code verification] 
    printf("[DEBUG] OSD::key_down: Calling vm->key_down(0x%02X, %d)\n", code, repeat);
    // [DEBUG_END - claude code verification]
    
    // VMにキーイベントを伝達（Android版と同様）
    if (vm) {
        vm->key_down(code, repeat);
    }
}

void OSD::key_up(int code, bool extended)
{
    if (code >= 0 && code < 256) {
        key_status[code] = 0;
        printf("[DEBUG] キー離す: VKCode=0x%02X, key_status[0x%02X]=0x%02X\n", code, code, key_status[code]);
    }
    
    // VMにキーイベントを伝達（Android版と同様）
    if (vm) {
        vm->key_up(code);
    }
}

void OSD::key_down_native(int code, bool repeat)
{
    // NSEventのkeyCodeを受け取ってWindows VKコードに変換
    int vk_code = convertNSKeyCodeToVK(code);
    printf("[DEBUG] key_down_native: NSKeyCode=%d → VKCode=0x%02X\n", code, vk_code);
    if (vk_code != 0) {
        key_down(vk_code, false, repeat);
    } else {
        printf("未知のキーコード: %d\n", code);
    }
}

void OSD::key_up_native(int code)
{
    // NSEventのkeyCodeを受け取ってWindows VKコードに変換
    int vk_code = convertNSKeyCodeToVK(code);
    printf("[DEBUG] key_up_native: NSKeyCode=%d → VKCode=0x%02X\n", code, vk_code);
    if (vk_code != 0) {
        key_up(vk_code, false);
    }
}

// マウス関連の実装
#ifdef USE_MOUSE
void OSD::enable_mouse()
{
    mouse_enabled = true;
    printf("マウス入力を有効化\n");
}

void OSD::disable_mouse()
{
    mouse_enabled = false;
    mouse_status[0] = mouse_status[1] = mouse_status[2] = 0;
    printf("マウス入力を無効化\n");
}

void OSD::toggle_mouse()
{
    if (mouse_enabled) {
        disable_mouse();
    } else {
        enable_mouse();
    }
}

// mouse_down/mouse_upメソッドはOSDクラスで宣言されていないため削除
// マウス入力の処理は別途実装が必要

// mouse_moveメソッドはOSDクラスで宣言されていないため削除
// 必要に応じて別途実装
#endif

// タッチ関連の実装
#ifdef USE_TOUCH
void OSD::touch_down(int id, int x, int y)
{
    if (id >= 0 && id < MAX_TOUCH) {
        touch_status[id][0] = x;
        touch_status[id][1] = y;
        touch_status[id][2] = 0x80; // 押下フラグ
        printf("タッチ開始: ID=%d, X=%d, Y=%d\n", id, x, y);
    }
}

void OSD::touch_up(int id)
{
    if (id >= 0 && id < MAX_TOUCH) {
        touch_status[id][2] = 0; // 押下フラグクリア
        printf("タッチ終了: ID=%d\n", id);
    }
}

void OSD::touch_move(int id, int x, int y)
{
    if (id >= 0 && id < MAX_TOUCH && (touch_status[id][2] & 0x80)) {
        touch_status[id][0] = x;
        touch_status[id][1] = y;
    }
}
#endif

// ジョイスティック関連の実装
#ifdef USE_JOYSTICK
// update_joystickメソッドはOSDクラスで宣言されていないため削除
// ジョイスティック更新は他の場所で実装
#endif

// Bridge.mm等から呼び出すためのC言語インターフェース
extern "C" {
    void osd_key_down_native(int code, bool repeat) {
        extern OSD* osd;
        if (osd) osd->key_down_native(code, repeat);
    }
    
    void osd_key_up_native(int code) {
        extern OSD* osd;
        if (osd) osd->key_up_native(code);
    }
    
#ifdef USE_MOUSE
    // mouse_down/mouse_up/mouse_moveメソッドは未宣言のため削除
    // マウス入力の処理は別途実装
#endif

#ifdef USE_TOUCH
    void osd_touch_down(int id, int x, int y) {
        extern OSD* osd;
        if (osd) osd->touch_down(id, x, y);
    }
    
    void osd_touch_up(int id) {
        extern OSD* osd;
        if (osd) osd->touch_up(id);
    }
    
    void osd_touch_move(int id, int x, int y) {
        extern OSD* osd;
        if (osd) osd->touch_move(id, x, y);
    }
#endif
}