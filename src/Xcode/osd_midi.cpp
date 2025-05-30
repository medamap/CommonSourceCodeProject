/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode midi ]

    Author : Medamap and Claude
    Date   : 2024
*/

#include "osd.h"

#ifdef USE_MIDI

void OSD::initialize_midi()
{
    // TODO: Core MIDI初期化
    // midi_initialized = false;
}

void OSD::release_midi()
{
    // TODO: MIDI解放
}

void OSD::send_to_midi(uint8_t data)
{
    // TODO: MIDIデータ送信
}

#endif