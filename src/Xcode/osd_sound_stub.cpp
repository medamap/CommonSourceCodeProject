/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode sound ]

    Author : Medamap and Claude
    Date   : 2024
*/

#include "osd.h"

void OSD::initialize_sound(int rate, int samples)
{
    // TODO: Core Audio初期化
    
    // 仮の設定
    sound_rate = rate;
    sound_samples = samples;
    sound_available = false;
    sound_started = false;
    sound_muted = false;
    
    // サウンドバッファの初期化
    // sound_buffer = NULL;
    // sound_buffer_size = 0;
}

void OSD::release_sound()
{
    // TODO: Core Audio解放
    
    // if(sound_buffer) {
    //     delete[] sound_buffer;
    //     sound_buffer = NULL;
    // }
}

void OSD::update_sound(int* extra_frames)
{
    // TODO: サウンド更新
    *extra_frames = 0;
}

// Medamap and Claude: osd.hでインライン定義済みのメソッドはコメントアウト
// void OSD::mute_sound()
// {
//     sound_muted = true;
// }

// void OSD::stop_sound()
// {
//     sound_started = false;
// }

// void OSD::start_record_sound()
// {
//     // TODO: 録音開始
// }

// void OSD::stop_record_sound()
// {
//     // TODO: 録音停止
// }

// void OSD::restart_record_sound()
// {
//     // TODO: 録音再開
// }

// now_record_soundはメンバ変数なのでメソッドは不要

// bool OSD::is_sound_recording()
// {
//     return false;
// }

// set_volumeメソッドがosd.hに宣言されていない