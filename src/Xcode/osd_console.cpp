/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode console ]

    Author : Medamap and Claude
    Date   : 2024
*/

#include "osd.h"
#include <iostream>

#ifdef USE_DEBUGGER

void OSD::open_console(int width, int height, const _TCHAR* title)
{
    // TODO: デバッグコンソール実装
    // 現在は標準出力を使用
    // Medamap and Claude: 引数を修正してヘッダー宣言と一致させる
}

void OSD::close_console()
{
    // TODO: コンソールクローズ
}

unsigned int OSD::get_console_code_page()
{
    // UTF-8を返す (macOS/iOS標準)
    return 65001;
}

void OSD::set_console_text_attribute(unsigned short attr)
{
    // TODO: テキスト属性設定（色など）
}

void OSD::write_console(const _TCHAR* buffer, unsigned int length)
{
    // 標準出力に書き込み
    fwrite(buffer, sizeof(_TCHAR), length, stdout);
    fflush(stdout);
}

int OSD::read_console_input(_TCHAR* buffer, unsigned int length)
{
    // TODO: コンソール入力読み取り
    if(fgets((char*)buffer, length, stdin)) {
        return (int)strlen((char*)buffer);
    }
    return 0;
}

bool OSD::is_console_key_pressed(int vk)
{
    // TODO: コンソールキー押下チェック
    return false;
}

bool OSD::is_console_active()
{
    // TODO: コンソールアクティブ状態チェック
    return false;
}

bool OSD::is_console_closed()
{
    // TODO: コンソールクローズ状態チェック
    return true;
}

void OSD::close_debugger_console()
{
    close_console();
}

#endif