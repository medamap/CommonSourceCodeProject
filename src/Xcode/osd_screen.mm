/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode screen ]

    Author : Medamap and Claude
    Date   : 2024
*/

#ifdef __APPLE__
// Objective-Cヘッダーを先に読み込む（BOOLの競合を避けるため）
#import "MetalView.h"
#endif

#include "osd.h"

void OSD::initialize_screen()
{
    printf("画面システムを初期化中...\n");
    
#ifdef __APPLE__
    // Metal初期化
    metal_initialized = false;
    metal_view = nullptr;
#endif
    
    // X1 turbo用の画面サイズ設定
    host_window_width = 640;
    host_window_height = 400;
    host_window_mode = 0;
    
    // ビットマップバッファの初期化
    vm_screen_width = vm_window_width = 640;
    vm_screen_height = vm_window_height = 400;
    vm_window_width_aspect = 640;
    vm_window_height_aspect = 400;
    
    // Medamap and Claude: vm_screen_bufferを作成（全機種で必要）
    create_bitmap(&vm_screen_buffer, vm_screen_width, vm_screen_height);
    
    if (vm_screen_buffer.initialized()) {
        printf("画面バッファ初期化完了: %dx%d\n", vm_screen_width, vm_screen_height);
        
        // 画面を黒でクリア
        clear_bitmap(&vm_screen_buffer, 0, 0, 0);
        
        // テスト用のボーダーを描画
        draw_rectangle_to_bitmap(&vm_screen_buffer, 0, 0, vm_screen_width, 2, 255, 255, 255); // 上
        draw_rectangle_to_bitmap(&vm_screen_buffer, 0, vm_screen_height-2, vm_screen_width, 2, 255, 255, 255); // 下
        draw_rectangle_to_bitmap(&vm_screen_buffer, 0, 0, 2, vm_screen_height, 255, 255, 255); // 左
        draw_rectangle_to_bitmap(&vm_screen_buffer, vm_screen_width-2, 0, 2, vm_screen_height, 255, 255, 255); // 右
        
        printf("画面システム初期化完了！\n");
    } else {
        printf("エラー: 画面バッファの初期化に失敗しました\n");
    }

#ifdef USE_SCREEN_ROTATE
    // 回転バッファの初期化（回転機能があるマシンのみ）
    // Medamap and Claude: 必要に応じて追加の回転バッファを作成
#endif
}

void OSD::release_screen()
{
#ifdef __APPLE__
    // Metal解放
    if(metal_view) {
        metal_view_destroy(metal_view);
        metal_view = nullptr;
        metal_initialized = false;
    }
#endif
    // Medamap and Claude: vm_screen_bufferを解放（全機種で必要）
    release_bitmap(&vm_screen_buffer);

#ifdef USE_SCREEN_ROTATE
    // 回転バッファの解放（回転機能があるマシンのみ）
    // Medamap and Claude: 必要に応じて追加の回転バッファを解放
#endif
}

// Medamap and Claude: reload_bitmapは条件付きコンパイル
#ifdef ONE_BOARD_MICRO_COMPUTER
void OSD::reload_bitmap()
{
    // TODO: ビットマップリロード処理
}
#endif

void OSD::capture_screen()
{
    // TODO: スクリーンキャプチャ
}

bool OSD::start_record_video(int fps)
{
    // TODO: ビデオ録画開始
    return false;
}

void OSD::stop_record_video()
{
    // TODO: ビデオ録画停止
}

void OSD::restart_record_video()
{
    // TODO: ビデオ録画再開
}

void OSD::add_extra_frames(int extra_frames)
{
    // TODO: 追加フレーム処理
}

// Medamap and Claude: now_record_videoはメンバ変数として定義済み

scrntype_t* OSD::get_vm_screen_buffer(int y)
{
    return vm_screen_buffer.get_buffer(y);
}

int OSD::draw_screen()
{
    // Medamap and Claude: 基本的な画面描画処理
    
    static int draw_count = 0;
    draw_count++;
    
    // vm_screen_bufferが初期化されているかチェック
    if (!vm_screen_buffer.initialized()) {
        printf("警告: vm_screen_bufferが初期化されていません\n");
        return 0;
    }
    
    // Debug output (reduced frequency for production)
    if (draw_count % 300 == 0) {
        printf("画面描画フレーム %d: サイズ %dx%d\n", 
               draw_count, vm_screen_width, vm_screen_height);
    }
    
    // 重要：VMに実際の画面を描画させる
    if (vm) {
        vm->draw_screen();
        if (draw_count % 600 == 0) {
            printf("VM画面描画実行: フレーム %d\n", draw_count);
        }
    }
    
#ifdef __APPLE__
    // Metalレンダリング（CSCPViewControllerから設定されたMetalViewを使用）
    if (metal_initialized && metal_view) {
        // vm_screen_bufferのピッチ（バイト単位）を計算
        int pitch = vm_screen_width * sizeof(scrntype_t);
        
        // Debug: フレーム描画通知（600フレームに1回）
        if (draw_count % 600 == 0) {
            printf("Metal更新: フレーム %d, バッファサイズ %dx%d, pitch %d\n", 
                   draw_count, vm_screen_width, vm_screen_height, pitch);
        }
        
        // Metal textureを更新
        metal_view_update_screen(metal_view, vm_screen_buffer.lpBmp, 
                               vm_screen_width, vm_screen_height, pitch);
    } else {
        // Debug: Metal未初期化の場合
        if (draw_count % 600 == 0) {
            printf("Metal未初期化: metal_initialized=%s, metal_view=%p\n", 
                   metal_initialized ? "true" : "false", metal_view);
        }
    }
#endif
    
    // 注意：実際のエミュレータ画面データはVMから来るべき
    // vm_screen_bufferにはVMによって既に描画データが書き込まれているはず
    
    return 1; // 成功
}

// Medamap and Claude: Windows専用メソッドは削除

int OSD::get_window_mode_width(int mode)
{
    // 仮の実装
    return 640 * (mode + 1);
}

int OSD::get_window_mode_height(int mode)
{
    // 仮の実装
    return 400 * (mode + 1);
}

double OSD::get_window_mode_power(int mode)
{
    // 仮の実装
    return mode + 1.0;
}

void OSD::set_host_window_size(int window_width, int window_height, bool window_mode)
{
    host_window_width = window_width;
    host_window_height = window_height;
    host_window_mode = window_mode ? 1 : 0;
}

void OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
    vm_screen_width = screen_width;
    vm_screen_height = screen_height;
    vm_window_width = window_width;
    vm_window_height = window_height;
    vm_window_width_aspect = window_width_aspect;
    vm_window_height_aspect = window_height_aspect;
    
#ifdef USE_SCREEN_ROTATE
    // バッファの再作成
    release_bitmap(&vm_screen_buffer);
    create_bitmap(&vm_screen_buffer, vm_screen_width, vm_screen_height);
#endif
}

void OSD::set_vm_screen_lines(int lines)
{
    // TODO: スキャンライン設定
}

// Medamap and Claude: 以下のメソッドはosd.hでインライン定義済みなのでコメントアウト
// void OSD::set_window_title(const _TCHAR* title)
// {
//     // TODO: ウィンドウタイトル設定
// }

// int OSD::get_vm_window_width()
// {
//     return vm_window_width;
// }

// int OSD::get_vm_window_height()
// {
//     return vm_window_height;
// }

// int OSD::get_vm_window_width_aspect()
// {
//     return vm_window_width_aspect;
// }

// int OSD::get_vm_window_height_aspect()
// {
//     return vm_window_height_aspect;
// }

// bool OSD::is_screen_changed()
// {
//     // TODO: 画面変更検出
//     return false;
// }

// Medamap and Claude: bitmap関連メソッドもコメントアウト
// bitmap_t* OSD::create_bitmap(int width, int height, int bpp)
// {
//     bitmap_t* bitmap = new bitmap_t();
//     bitmap->create(width, height, bpp);
//     return bitmap;
// }

// void OSD::release_bitmap(bitmap_t* bitmap)
// {
//     bitmap->release();
//     delete bitmap;
// }

// bitmap_t* OSD::create_font(const _TCHAR* family, int width, int height, int rotate, bool bold, bool italic)
// {
//     // TODO: フォント作成
//     return create_bitmap(width, height, 1);
// }

// void OSD::create_bitmap(bitmap_t* bitmap, int width, int height, int bpp)
// {
//     bitmap->create(width, height, bpp);
// }

// void OSD::release_bitmap(bitmap_t* bitmap)
// {
//     bitmap->release();
// }

// void OSD::create_font(bitmap_t* bitmap, const _TCHAR* family, int width, int height, int rotate, bool bold, bool italic)
// {
//     // TODO: フォント作成
//     bitmap->create(width, height, 1);
// }

// Medamap and Claude: bitmap関連メソッドもコメントアウト
// void OSD::copy_bitmap(bitmap_t* dest, int dest_x, int dest_y, int width, int height, bitmap_t* source)
// {
//     // TODO: ビットマップコピー
//     for(int y = 0; y < height && dest_y + y < dest->get_height() && y < source->get_height(); y++) {
//         scrntype_t* dest_line = dest->get_buffer(dest_y + y);
//         scrntype_t* src_line = source->get_buffer(y);
//         for(int x = 0; x < width && dest_x + x < dest->get_width() && x < source->get_width(); x++) {
//             dest_line[dest_x + x] = src_line[x];
//         }
//     }
// }

// Medamap and Claude: 残りのbitmap関連メソッドもコメントアウト
// void OSD::draw_text_to_bitmap(bitmap_t* bitmap, int x, int y, const _TCHAR* text, size_t len)
// {
//     // TODO: テキスト描画
// }

// void OSD::draw_line_to_bitmap(bitmap_t* bitmap, int sx, int sy, int ex, int ey, int color)
// {
//     // TODO: 線描画
// }

// void OSD::draw_rectangle_to_bitmap(bitmap_t* bitmap, int x, int y, int width, int height, int color)
// {
//     // TODO: 矩形描画
// }

// void OSD::draw_rectangle_to_bitmap(bitmap_t* bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
// {
//     // Medamap and Claude: RGB版の矩形描画
//     int color = RGB_COLOR(r, g, b);
//     draw_rectangle_to_bitmap(bitmap, x, y, width, height, color);
// }

// void OSD::draw_point_to_bitmap(bitmap_t* bitmap, int x, int y, int color)
// {
//     if(x >= 0 && x < bitmap->get_width() && y >= 0 && y < bitmap->get_height()) {
//         bitmap->get_buffer(y)[x] = color;
//     }
// }

// void OSD::stretch_bitmap(bitmap_t* dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t* source, int source_x, int source_y, int source_width, int source_height)
// {
//     // TODO: ビットマップ拡大縮小
// }

// void OSD::write_bitmap_to_file(bitmap_t* bitmap, const _TCHAR* file_path)
// {
//     // TODO: ビットマップ保存
// }

#ifdef USE_CRT_FILTER
void OSD::screen_skip_line(bool skip_line)
{
    // TODO: CRTフィルター
}
#endif

#ifdef ONE_BOARD_MICRO_COMPUTER
void OSD::get_invalidated_rect(int* left, int* top, int* right, int* bottom)
{
    // TODO: 無効化領域取得
    *left = *top = 0;
    *right = vm_screen_width;
    *bottom = vm_screen_height;
}

void OSD::reload_bitmap()
{
    // TODO: ビットマップリロード
}
#endif

#ifdef OSD_WIN32
void OSD::invalidate_screen()
{
    // Windows用なので空実装
}

// Medamap and Claude: Windows専用メソッドは削除
#endif

// Medamap and Claude: 必要なbitmap関数群を実装
void OSD::create_bitmap(bitmap_t *bitmap, int width, int height)
{
    if(!bitmap) return;
    
    bitmap->width = width;
    bitmap->height = height;
    bitmap->lpBmp = new scrntype_t[width * height];
    
    // 初期化 (黒で塗りつぶし)
    if(bitmap->lpBmp) {
        memset(bitmap->lpBmp, 0, width * height * sizeof(scrntype_t));
    }
}

void OSD::release_bitmap(bitmap_t *bitmap)
{
    if(!bitmap || !bitmap->lpBmp) return;
    
    delete[] bitmap->lpBmp;
    bitmap->lpBmp = nullptr;
    bitmap->width = bitmap->height = 0;
}

void OSD::create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic)
{
    if(!font) return;
    
    // Medamap and Claude: フォント情報を保存（実際の描画は後で実装）
    my_tcscpy_s(font->family, family);
    font->width = width;
    font->height = height;
    font->rotate = rotate;
    font->bold = bold;
    font->italic = italic;
}

void OSD::release_font(font_t *font)
{
    if(!font) return;
    
    // TODO: Core Text フォント解放
    memset(font, 0, sizeof(font_t));
}

void OSD::create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)
{
    if(!pen) return;
    
    pen->width = width;
    pen->r = r;
    pen->g = g;
    pen->b = b;
}

void OSD::release_pen(pen_t *pen)
{
    if(!pen) return;
    
    memset(pen, 0, sizeof(pen_t));
}

void OSD::clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)
{
    if(!bitmap || !bitmap->lpBmp) return;
    
    scrntype_t color = RGB_COLOR(r, g, b);
    scrntype_t *buffer = bitmap->lpBmp;
    int total_pixels = bitmap->width * bitmap->height;
    
    for(int i = 0; i < total_pixels; i++) {
        buffer[i] = color;
    }
}

int OSD::get_text_width(bitmap_t *bitmap, font_t *font, const char *text)
{
    if(!text || !font) return 0;
    
    // Medamap and Claude: 仮実装 - 固定幅フォントとして計算
    return (int)strlen(text) * font->width;
}

void OSD::draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b)
{
    if(!bitmap || !bitmap->lpBmp || !font || !text) return;
    
    // TODO: Core Text を使った文字描画
    // 現在は何もしない（仮実装）
}

void OSD::draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)
{
    if(!bitmap || !bitmap->lpBmp || !pen) return;
    
    // Medamap and Claude: Bresenhamの直線アルゴリズム
    scrntype_t color = RGB_COLOR(pen->r, pen->g, pen->b);
    
    int dx = abs(ex - sx);
    int dy = abs(ey - sy);
    int x_inc = (sx < ex) ? 1 : -1;
    int y_inc = (sy < ey) ? 1 : -1;
    
    int x = sx, y = sy;
    int error = dx - dy;
    
    while(true) {
        if(x >= 0 && x < bitmap->width && y >= 0 && y < bitmap->height) {
            bitmap->get_buffer(y)[x] = color;
        }
        
        if(x == ex && y == ey) break;
        
        int error2 = error * 2;
        if(error2 > -dy) {
            error -= dy;
            x += x_inc;
        }
        if(error2 < dx) {
            error += dx;
            y += y_inc;
        }
    }
}

void OSD::draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
    if(!bitmap || !bitmap->lpBmp) return;
    
    scrntype_t color = RGB_COLOR(r, g, b);
    
    for(int yy = y; yy < y + height && yy < bitmap->height; yy++) {
        if(yy < 0) continue;
        scrntype_t* line = bitmap->get_buffer(yy);
        for(int xx = x; xx < x + width && xx < bitmap->width; xx++) {
            if(xx >= 0) {
                line[xx] = color;
            }
        }
    }
}

void OSD::draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if(!bitmap || !bitmap->lpBmp) return;
    
    if(x >= 0 && x < bitmap->width && y >= 0 && y < bitmap->height) {
        scrntype_t color = RGB_COLOR(r, g, b);
        bitmap->get_buffer(y)[x] = color;
    }
}

void OSD::stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, 
                        bitmap_t *source, int source_x, int source_y, int source_width, int source_height)
{
    if(!dest || !dest->lpBmp || !source || !source->lpBmp) return;
    
    // Medamap and Claude: 単純な最近傍補間
    for(int y = 0; y < dest_height; y++) {
        int src_y = source_y + (y * source_height) / dest_height;
        if(src_y < 0 || src_y >= source->height) continue;
        
        int dest_line_y = dest_y + y;
        if(dest_line_y < 0 || dest_line_y >= dest->height) continue;
        
        scrntype_t* dest_line = dest->get_buffer(dest_line_y);
        scrntype_t* src_line = source->get_buffer(src_y);
        
        for(int x = 0; x < dest_width; x++) {
            int src_x = source_x + (x * source_width) / dest_width;
            if(src_x < 0 || src_x >= source->width) continue;
            
            int dest_pixel_x = dest_x + x;
            if(dest_pixel_x < 0 || dest_pixel_x >= dest->width) continue;
            
            dest_line[dest_pixel_x] = src_line[src_x];
        }
    }
}

void OSD::write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)
{
    if(!bitmap || !bitmap->lpBmp || !file_path) return;
    
    // TODO: macOS/iOS用のビットマップ保存実装
    // 現在は何もしない（仮実装）
}

#ifdef USE_MOUSE
// Medamap and Claude: マウス関連の関数はosd_input.cppで定義されているため、ここではコメントアウト
// void OSD::enable_mouse()
// {
//     mouse_enabled = true;
// }

// void OSD::disable_mouse()
// {
//     mouse_enabled = false;
// }

// void OSD::toggle_mouse()
// {
//     mouse_enabled = !mouse_enabled;
// }
#endif

#ifdef USE_MIDI
bool OSD::recv_from_midi(uint8_t *data)
{
    if(!data) return false;
    
    // TODO: Core MIDI受信実装
    *data = 0; // 現在は何も受信しない
    return false; // 何も受信していない
}
#endif

// Bridge.mm等から呼び出すためのC言語インターフェース
extern "C" {
    void osd_set_metal_view(void* view) {
        extern OSD* osd;
        if (osd) {
#ifdef __APPLE__
            osd->metal_view = view;
            osd->metal_initialized = (view != nullptr);
            printf("Metal view設定: %p, 初期化状態: %s\n", 
                   view, osd->metal_initialized ? "true" : "false");
#endif
        }
    }
    
    int osd_draw_screen() {
        extern OSD* osd;
        if (osd) {
            return osd->draw_screen();
        }
        return 0;
    }
}