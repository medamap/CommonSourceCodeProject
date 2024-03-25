/*
	Skelton for retropc emulator

	Author : @shikarunochi
	Date   : 2020.06.01-

	[ android main ]
*/

#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <android/window.h>
#include <android/looper.h>

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>

#include <vector>
#include<string>

#include <unistd.h>
#include "emu.h"
#include "Android/osd.h"
#include "fifo.h"
#include "fileio.h"

#include <iconv.h>

// ストレージ権限付与イベント
#define PERMISSIONS_GRANTED_EVENT 1

// emulation core
EMU *emu;
#define SOFT_KEYBOARD_KEEP_COUNT  3
int softKeyboardCount = 0;
bool softKeyShift = false;
bool softKeyCtrl = false;

int softKeyCode = 0;
bool softKeyDelayFlag = false;

//ScreenSize screenSize = SCREEN_SIZE_JUST;
//ScreenSize preScreenSize = SCREEN_SIZE_JUST;
ScreenSize screenSize = SCREEN_SIZE_MAX;
ScreenSize preScreenSize = SCREEN_SIZE_MAX;
//#define  LOG_TAG    "libplasma"
//#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* Set to 1 to enable debug log traces. */
#define DEBUG 0

/* Set to 1 to optimize memory stores when generating plasma. */
#define OPTIMIZE_WRITES  1

/** @param onClickListener MUST be NULL because the model dialog is not implemented. */
typedef void *(OnClickListener)(int id);

void selectMedia(struct android_app *state);

void selectBootMode(struct android_app *state);

jint
showAlert(struct android_app *state, const char *message, const char *filenames, bool model = false,
          int selectMode = 0, int selectIndex = 0);

const char *jniGetDocumentPath(struct android_app *state);
const char *jniGetExternalStoragePath(struct android_app *state);
const char *jniGetDownloadPath(struct android_app *state);
const char *jniGetSdcardDownloadPath(struct android_app *state);


void jniReadIconData(struct android_app *state);

BitmapData jniCreateBitmapFromString(struct android_app *state, const char *text, int fontSize);

//const char* getSDCARDPath(struct android_app *state);
char documentDir[_MAX_PATH];

std::vector<std::string> fileList;

FileSelectType fileSelectType;
FileSelectIconData fileSelectIconData[MAX_FILE_SELECT_ICON];
int selectingIconIndex;
bool needSelectDiskBank = false;
int selectDiskDrive = 0;

bool resetFlag;

BitmapData systemIconData[16];
BitmapData mediaIconData[16];
DeviceInfo deviceInfo;

// status bar
//bool status_bar_visible = false;

#ifdef USE_FLOPPY_DISK
uint32_t fd_status = 0x80000000;
#endif
#ifdef USE_QUICK_DISK
uint32_t qd_status = 0x80000000;
#endif
#ifdef USE_HARD_DISK
uint32_t hd_status = 0x80000000;
#endif
#ifdef USE_COMPACT_DISC
uint32_t cd_status = 0x80000000;
#endif
#ifdef USE_LASER_DISC
uint32_t ld_status = 0x80000000;
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
_TCHAR tape_status[1024] = _T("uninitialized");
int tape_position = 0;
#endif


void selectDisk(struct android_app *state, int diskNo);

/* Return current time in milliseconds */
static double now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000. + tv.tv_usec / 1000.;
}


static uint16_t make565(int red, int green, int blue) {
    return (uint16_t) (((red << 8) & 0xf800) |
                       ((green << 3) & 0x07e0) |
                       ((blue >> 3) & 0x001f));
}

static void init_palette(void) {

}

/* Angles expressed as fixed point radians */

static void init_tables(void) {

}

static void draw_icon(ANativeWindow_Buffer *buffer) {
    int bufferWidth = buffer->width;
    int bufferHeight = buffer->height;

    uint16_t *pixels = (uint16_t *) (buffer->bits);

    int unitPixel = bufferWidth / 12;

    int offsetY = 100;
    int iconHeightMax = 0;

    for (int index = 0; index < MAX_FILE_SELECT_ICON; index++) {
        bool accessFlag = false;
        bool setFlag = false;
        int fileSelectType = fileSelectIconData[index].fileSelectType;
        if (fileSelectType >= 0) {
            //アクセス中は、緑色にして表示
#ifdef USE_FLOPPY_DISK
            if (fileSelectType == FLOPPY_DISK) {
                if (emu->is_floppy_disk_inserted(fileSelectIconData[index].driveNo) == true) {
                    setFlag = true;
                }
                int idx = (fd_status >> fileSelectIconData[index].driveNo) & 1;
                if (idx != 0) {
                    //アクセス中
                    accessFlag = true;
                }
            }
#endif
#ifdef USE_QUICK_DISK
            if (fileSelectType == QUICK_DISK) {
                if (emu->is_quick_disk_inserted(fileSelectIconData[index].driveNo) == true) {
                    setFlag = true;
                }
                if (emu->is_quick_disk_accessed() != 0) {
                    //アクセス中
                    accessFlag = true;
                }
            }
#endif
#ifdef USE_TAPE
            if (fileSelectType == CASETTE_TAPE) {
                if (emu->is_tape_inserted(0) == true) {
                    setFlag = true;
                }
                if (emu->is_tape_playing(0) == true) {
                    //アクセス中
                    accessFlag = true;
                }
            }
#endif
#ifdef USE_CART
            if(fileSelectType == CARTRIDGE){
                if(emu->is_cart_inserted(fileSelectIconData[index].driveNo)==true){
                    setFlag = true;
                }
            }
#endif
            if (iconHeightMax < mediaIconData[fileSelectType].height) {
                iconHeightMax = mediaIconData[fileSelectType].height;
            }
            uint16_t *iconPixels = pixels + buffer->stride * offsetY + (index) * unitPixel;

            for (int y = 0; y < mediaIconData[fileSelectType].height; y++) {

                uint16_t *line = (uint16_t *) iconPixels;
                for (int x = 0; x < mediaIconData[fileSelectType].width; x++) {
                    uint16_t dotData = mediaIconData[fileSelectType].bmpImage[x + y *
                                                                                  mediaIconData[fileSelectType].width];
                    //データ縮小時に色が付いているので白黒で表示する。
                    if (dotData > 0) {
                        if (setFlag == true) {
                            dotData = 0xFFFF;
                        } else {
                            dotData = 0x8410;
                        }
                    } else {
                        dotData = 0x0000;
                    }
                    if (accessFlag == true) {
                        dotData = dotData & 0b0000011111100000; //緑色のみにする
                    }
                    line[x] = dotData;
                }
                iconPixels = iconPixels + buffer->stride;
            }
        }
    }
    //右端に PCG / SOUND / SCREEN / RESET
    for (int index = 0; index < 4; index++) {
        int dotMask = 0xFFFF;
        if (index == 3) {
#if defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700)
            if((config.dipswitch & 1)==false){
                dotMask = 0x8410;
            }
#elif defined(SUPPORT_PC88_PCG8100)
            if((config.dipswitch & (1 << 3))==false){
                dotMask = 0x8410;
            }
#else
            continue;
#endif
        }
        if (index == 2) {
            if (emu->get_osd()->soundEnable == false) {
                dotMask = 0x8410;
            }
        }

        uint16_t *iconPixels = pixels + buffer->stride * offsetY + (11 - index) * unitPixel;
        for (int y = 0; y < systemIconData[index].height; y++) {
            uint16_t *line = (uint16_t *) iconPixels;
            for (int x = 0; x < systemIconData[index].width; x++) {
                line[x] = (systemIconData[index].bmpImage[x + y * systemIconData[index].width]) &
                          dotMask;
            }
            iconPixels = iconPixels + buffer->stride;
        }
    }
    //テープ読み込み中の場合、パーセントに応じた表示する。
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
    int tapeY = iconHeightMax + offsetY + 10;
    uint16_t *line = pixels + buffer->stride * tapeY;
    int tapePercentPixel = bufferWidth * tape_position / 100;

    for (int x = 0; x < bufferWidth; x++) {
        if (x >= tapePercentPixel) {
            line[x] = 0;
            line[x + buffer->stride] = 0;
        } else {
            line[x] = 0x6761;
            line[x + buffer->stride] = 0x6761;
        }
    }
#endif
}

static void
draw_message(struct android_app *state, ANativeWindow_Buffer *buffer, const char *message) {
    bitmap_t *screenBuffer = emu->get_osd()->getScreenBuffer();
    scrntype_t *lpBmp = screenBuffer->lpBmp;

    //画面のサイズ
    int bufferWidth = buffer->width;
    int bufferHeight = buffer->height;

    //エミュレータ側の画面のサイズ
    int width = emu->get_osd()->get_vm_window_width();
    int height = emu->get_osd()->get_vm_window_height();

    float widthRate = (float) bufferWidth / width;
    float heightRate = (float) bufferHeight / height;

    //縦横小さい側の倍率で拡大
    float screenRate = 0;
    if (widthRate < heightRate) {
        screenRate = widthRate;
    } else {
        screenRate = heightRate;
    }
    int realScreenWidth = (float) width * screenRate;
    int realScreenHeight = (float) height * screenRate;

    int messageY = realScreenHeight + 150 * screenRate;
    //messageY = 2000;
    uint16_t *pixels = (uint16_t *) (buffer->bits);
    pixels = pixels + buffer->stride * messageY;


    BitmapData messageBitmap = jniCreateBitmapFromString(state, message, 15 * screenRate);

    for (int y = 0; y < messageBitmap.height; y++) {
        uint16_t *line = (uint16_t *) pixels;
        for (int x = 0; x < messageBitmap.width; x++) {
            line[x] = messageBitmap.bmpImage[x + y * messageBitmap.width];
        }
        pixels = pixels + buffer->stride;
    }
    delete messageBitmap.bmpImage;
}

static void load_emulator_screen(ANativeWindow_Buffer *buffer) {
    bitmap_t *screenBuffer = emu->get_osd()->getScreenBuffer();
    scrntype_t *lpBmp = screenBuffer->lpBmp;
    void *pixels = buffer->bits;
    pixels = (uint16_t *) pixels + buffer->stride * 150;

    //画面のサイズ
    int bufferWidth = buffer->width;
    int bufferHeight = buffer->height;

    //エミュレータ側の画面のサイズ
    int width = emu->get_osd()->get_vm_window_width();
    int height = emu->get_osd()->get_vm_window_height();
    float aspect = (float) width / height;

    float widthRate = (float) bufferWidth / width;
    float heightRate = (float) bufferHeight / height;

    //縦横小さい側の倍率で拡大
    float screenRate = 0;
    if (widthRate < heightRate) {
        screenRate = widthRate;
    } else {
        screenRate = heightRate;
    }
    int realScreenWidth = (float) width * screenRate;
    int realScreenHeight = (float) height * screenRate;

    int widthOffset = (bufferWidth - realScreenWidth) / 2;
    int heightOffset = 50 * screenRate;

    pixels = (uint16_t *) pixels + buffer->stride * heightOffset + widthOffset;

    if (preScreenSize != screenSize) {
        void *tempPixels = pixels;
        for (int y = 0; y < realScreenHeight; y++) {
            uint16_t *line = (uint16_t *) tempPixels;
            for (int x = 0; x < realScreenWidth; x++) {
                line[x] = 0;
            }
            tempPixels = (uint16_t *) tempPixels + buffer->stride;
        }
        preScreenSize = screenSize;
    }


    if (screenSize == SCREEN_SIZE_MAX) {
        for (int y = 0; y < realScreenHeight; y++) {
            uint16_t *line = (uint16_t *) pixels;
            for (int x = 0; x < realScreenWidth; x++) {
                int emuX = x / screenRate;
                int emuY = y / screenRate;
                if (emuX < width && emuY < height) {
                    line[x] = *(lpBmp + (height - emuY - 1) * width + emuX - 1);
                } else {
                    line[x] = 0;
                }
            }
            pixels = (uint16_t *) pixels + buffer->stride;
        }
    } else if (screenSize == SCREEN_SIZE_SPECIAL) {
        //真ん中の描画画面
        int xOffset = (realScreenWidth - width) / 2;
        int yOffset = (realScreenHeight - height) / 2;

        int xScreenCount = xOffset / width + 1;
        int emuStartX = -(xOffset - width * xScreenCount);

        int yScreenCount = yOffset / height + 1;
        int emuStartY = -(yOffset - height * yScreenCount);

        int emuX = emuStartX;
        int emuY = emuStartY;

        for (int y = 0; y < realScreenHeight; y++) {
            uint16_t *line = (uint16_t *) pixels;
            for (int x = 0; x < realScreenWidth; x++) {
                line[x] =
                        *(lpBmp + (height - emuY - 1) * width + emuX - 1);
                emuX = emuX + 1;
                if (emuX > width) {
                    emuX = 0;
                }
            }
            emuX = emuStartX;
            emuY = emuY + 1;
            if (emuY > height) {
                emuY = 0;
            }
            pixels = (uint16_t *) pixels + buffer->stride;
        }
    } else {
        float scale = 1.0;
        //MAXより小さく、ドットバイドット等倍で入るギリギリサイズ
        int screenSize1ScaleWidth = realScreenWidth / width;
        int screenSize1ScaleHeight = realScreenHeight / height;
        int screenSize1Scale = 0;
        if (screenSize1ScaleWidth < screenSize1ScaleHeight) {
            screenSize1Scale = screenSize1ScaleWidth;
        } else {
            screenSize1Scale = screenSize1ScaleHeight;
        }
        float screenSize2Scale = screenSize1Scale;
        if (screenSize1Scale > 3) {
            screenSize2Scale = (float) screenSize1Scale / 2;
        } else {
            screenSize2Scale = 1.5;
        }
        switch (screenSize) {
            case SCREEN_SIZE_JUST:
                scale = screenSize1Scale;
                break;
            case SCREEN_SIZE_1:
                scale = screenSize2Scale;
                break;
            case SCREEN_SIZE_2:
                scale = 1.0;
                break;
        }

        int drawScreenWidth = (float) width * scale;
        int drawScreenHeight = (float) height * scale;
        int xOffset = (realScreenWidth - drawScreenWidth) / 2;
        int yOffset = (realScreenHeight - drawScreenHeight) / 2;

//        LOGI("width %d, height %d, screenSize %d: scale %f, drawScreenHeight %d, drawScreenWidth %d, xOffset %d, yOffset %d"
//               ,width, height, screenSize, scale, drawScreenHeight, drawScreenWidth, xOffset, yOffset);
        pixels = (uint16_t *) pixels + xOffset + yOffset * buffer->stride;
        for (int y = 0; y < drawScreenHeight; y++) {
            uint16_t *line = (uint16_t *) pixels;
            for (int x = 0; x < drawScreenWidth; x++) {
                int emuX = x / scale;
                int emuY = y / scale;
                if (0 < emuX && emuX < width && 0 < emuY && emuY < height) {
                    line[x] =
                            *(lpBmp + (height - emuY) * width + emuX - 1);
                } else {
                    line[x] = 0;
                }
            }
            pixels = (uint16_t *) pixels + buffer->stride;
        }
//        LOGI("draW END");
    }


    //if(*lpBmp !=0){
    //    char test[51] = {'\0'};
    //    for(int y = 100;y < 150;y++){
    //        for(int x = 0;x < 50;x++){
    //            if(G_OF_COLOR(*(lpBmp+y*width + x + 100))  > 0){
    //                test[x]='1';
    //            }else{
    //                test[x]='0';
    //            }
    //        }
    //        LOGI("%d: %s", y, test);
    //    }
    //}

}

void switchPCG() {

#if defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700)
    config.dipswitch = config.dipswitch ^ 1;
#endif
#if defined(SUPPORT_PC88_PCG8100)
    config.dipswitch = config.dipswitch ^ (1 << 3);
#endif

}

void switchSound() {
    emu->get_osd()->reset_sound();
    emu->get_osd()->soundEnable = !(emu->get_osd()->soundEnable);
}

/* simple stats management */
typedef struct {
    double renderTime;
    double frameTime;
} FrameStats;

#define  MAX_FRAME_STATS  200
#define  MAX_PERIOD_MS    1500

typedef struct {
    double firstTime;
    double lastTime;
    double frameTime;

    int firstFrame;
    int numFrames;
    FrameStats frames[MAX_FRAME_STATS];
} Stats;

static void
stats_init(Stats *s) {
    s->lastTime = now_ms();
    s->firstTime = 0.;
    s->firstFrame = 0;
    s->numFrames = 0;
}

static void
stats_startFrame(Stats *s) {
    s->frameTime = now_ms();
}

static void
stats_endFrame(Stats *s) {
    double now = now_ms();
    double renderTime = now - s->frameTime;
    double frameTime = now - s->lastTime;
    int nn;

    if (now - s->firstTime >= MAX_PERIOD_MS) {
        if (s->numFrames > 0) {
            double minRender, maxRender, avgRender;
            double minFrame, maxFrame, avgFrame;
            int count;

            nn = s->firstFrame;
            minRender = maxRender = avgRender = s->frames[nn].renderTime;
            minFrame = maxFrame = avgFrame = s->frames[nn].frameTime;
            for (count = s->numFrames; count > 0; count--) {
                nn += 1;
                if (nn >= MAX_FRAME_STATS)
                    nn -= MAX_FRAME_STATS;
                double render = s->frames[nn].renderTime;
                if (render < minRender) minRender = render;
                if (render > maxRender) maxRender = render;
                double frame = s->frames[nn].frameTime;
                if (frame < minFrame) minFrame = frame;
                if (frame > maxFrame) maxFrame = frame;
                avgRender += render;
                avgFrame += frame;
            }
            avgRender /= s->numFrames;
            avgFrame /= s->numFrames;

            //LOGI("frame/s (avg,min,max) = (%.1f,%.1f,%.1f) "
            //     "render time ms (avg,min,max) = (%.1f,%.1f,%.1f)\n",
            //     1000./avgFrame, 1000./maxFrame, 1000./minFrame,
            //     avgRender, minRender, maxRender);
        }
        s->numFrames = 0;
        s->firstFrame = 0;
        s->firstTime = now;
    }

    nn = s->firstFrame + s->numFrames;
    if (nn >= MAX_FRAME_STATS)
        nn -= MAX_FRAME_STATS;

    s->frames[nn].renderTime = renderTime;
    s->frames[nn].frameTime = frameTime;

    if (s->numFrames < MAX_FRAME_STATS) {
        s->numFrames += 1;
    } else {
        s->firstFrame += 1;
        if (s->firstFrame >= MAX_FRAME_STATS)
            s->firstFrame -= MAX_FRAME_STATS;
    }

    s->lastTime = now;
}

static void toggle_soft_keyboard(struct android_app *app) {
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass cls = jni->GetObjectClass(app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(cls, "getSystemService",
                                          "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring service_name = jni->NewStringUTF("input_method");
    jobject input_service = jni->CallObjectMethod(app->activity->clazz, methodID, service_name);

    jclass input_service_cls = jni->GetObjectClass(input_service);
    methodID = jni->GetMethodID(input_service_cls, "toggleSoftInput", "(II)V");
    jni->CallVoidMethod(input_service, methodID, 0, 0);

    jni->DeleteLocalRef(service_name);

    app->activity->vm->DetachCurrentThread();
}

static int
get_unicode_character(struct android_app *app, int event_type, int key_code, int meta_state) {
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread(&jni, NULL);

    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = "NativeThread";
    args.group = NULL;

    jclass KeyEvent = jni->FindClass("android/view/KeyEvent");
    jmethodID constructor = jni->GetMethodID(KeyEvent, "<init>", "(II)V");
    jmethodID getUnicodeChar = jni->GetMethodID(KeyEvent, "getUnicodeChar", "(I)I");

    jobject key_event = jni->NewObject(KeyEvent, constructor, event_type, key_code);
    jint unicode_key = jni->CallIntMethod(key_event, getUnicodeChar, meta_state);

    app->activity->vm->DetachCurrentThread();
    return unicode_key;
}

struct engine {
    struct android_app *app;
    Stats stats;
    int animating;
    bool emu_initialized;
};

static int64_t start_ms;

static void engine_draw_frame(struct engine *engine) {
    if (engine->app->window == NULL) {
        // No window.
        return;
    }
    if (!engine->emu_initialized) {
        return;
    }

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }

    stats_startFrame(&engine->stats);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t time_ms = (((int64_t) now.tv_sec) * 1000000000LL + now.tv_nsec) / 1000000;
    time_ms -= start_ms;

    /* Now fill the values with a nice little plasma */
    if (engine->emu_initialized) {
        load_emulator_screen(&buffer);
    }
    draw_icon(&buffer);

    ANativeWindow_unlockAndPost(engine->app->window);

    stats_endFrame(&engine->stats);
}

static void engine_draw_message(struct engine *engine, const char *message) {
    if (engine->app->window == NULL) {
        // No window.
        return;
    }

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }

    stats_startFrame(&engine->stats);
    draw_message(engine->app, &buffer, message);
    ANativeWindow_unlockAndPost(engine->app->window);
    stats_endFrame(&engine->stats);
}

static void engine_term_display(struct engine *engine) {
    engine->animating = 0;
}

static void clear_screen(struct engine *engine) {
    if (engine->app->window == NULL) {
        // No window.
        return;
    }

    LOGI("ClearScreen");
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }
    int bufferWidth = buffer.width;
    int bufferHeight = buffer.height;
    void *pixels = buffer.bits;
    memset(pixels, 0, bufferWidth * bufferHeight * sizeof(uint16_t));

    ANativeWindow_unlockAndPost(engine->app->window);

    //ダブルバッファになっているようなので２回繰り返しクリア（もっと良い方法があれば…）
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }
    bufferWidth = buffer.width;
    bufferHeight = buffer.height;
    pixels = buffer.bits;
    memset(pixels, 0, bufferWidth * bufferHeight * sizeof(uint16_t));

    ANativeWindow_unlockAndPost(engine->app->window);

}

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    struct engine *engine = (struct engine *) app->userData;
    if (!engine->emu_initialized) return 0;
    int action = AKeyEvent_getAction(event);
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP) {
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);
            //アイコンチェック
            LOGI("X:%f, Y:%f, witdh:%d", x, y, deviceInfo.width);
            if (y > 100 && y < 300) {
                int unitPixel = deviceInfo.width / 12;
                for (int index = 0; index < MAX_FILE_SELECT_ICON; index++) {
                    if (x > index * unitPixel && x < (index + 1) * unitPixel) {
                        if (fileSelectIconData[index].fileSelectType >= 0) {
                            selectingIconIndex = index;
                            selectMedia(app);
                            return 0;
                        }
                        break;
                    }
                }
                if (x > deviceInfo.width - unitPixel) {
                    LOGI("Reset!");
                    selectBootMode(app);
                    return 0;
                } else if (x > deviceInfo.width - unitPixel * 2) {
                    int newScreenSize = (int) screenSize + 1;
                    if (newScreenSize > SCREEN_SIZE_SPECIAL) {
                        screenSize = SCREEN_SIZE_JUST;
                    } else {
                        screenSize = (ScreenSize) newScreenSize;
                    }
                    clear_screen(engine);
                } else if (x > deviceInfo.width - unitPixel * 3) {
                    switchSound();
                } else if (x > deviceInfo.width - unitPixel * 4) {
                    switchPCG();
                }
            }
        }

        if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);
            if (y > deviceInfo.height / 2) {
                toggle_soft_keyboard(app);
            }
        }
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        LOGI("Key event: action=%d keyCode=%d metaState=0x%x flags=%d source=%d",
             AKeyEvent_getAction(event),
             AKeyEvent_getKeyCode(event),
             AKeyEvent_getMetaState(event),
             AKeyEvent_getFlags(event),
             AInputEvent_getSource(event)
        );
        int action = AKeyEvent_getAction(event);
        int code = AKeyEvent_getKeyCode(event);

        int flags = AKeyEvent_getFlags(event);
        int source = AInputEvent_getSource(event);

        if (action == AKEY_EVENT_ACTION_DOWN) {
            if (AKEYCODE_BACK == AKeyEvent_getKeyCode(event)) {
                char message[128];
                sprintf(message, "Exit Emulator? [%s/%s]",__DATE__,__TIME__);
                showAlert(app, message, "", true, EXIT_EMULATOR);
                return 1;
            }

            if ((flags & AKEY_EVENT_FLAG_FROM_SYSTEM) == 0) { //ソフトウェアキーボード
                LOGI("SoftwareKeyboard:count:%d", softKeyboardCount);
                if (softKeyboardCount > 0) {
                    return 1;
                } else {
                    //SHIFTキー
                    if (code == 59) {
                        softKeyShift = true;
                    } else if (code == 113) {
                        softKeyCtrl = true;
                    } else {
                        //ここでは shift / ctrl 押下のみ行い、キー入力自体はメインループ内で行っています。
                        //同時処理だと、 shift / ctrl を拾えない機種があったので。
                        if (softKeyCtrl == true) {
                            emu->get_osd()->key_down(113, false, false);
                            softKeyDelayFlag = true;
                            //emu->get_osd()->key_down(softKeyCode, false, false);
                            softKeyboardCount = SOFT_KEYBOARD_KEEP_COUNT;
                        }
                        if (softKeyShift == true) {
                            softKeyCode = usKeytoJISKeyShift[code][0];
                            if (usKeytoJISKeyShift[code][1] == 1) {
                                emu->get_osd()->key_down(59, false, false);
                                softKeyShift = true;
                            } else {
                                softKeyShift = false;
                            }
                            softKeyDelayFlag = true;
                            //emu->get_osd()->key_down(softKeyCode, false, false);
                            softKeyboardCount = SOFT_KEYBOARD_KEEP_COUNT;
                        } else {
                            softKeyCode = usKeytoJISKey[code][0];
                            if (usKeytoJISKey[code][1] == 1) {
                                emu->get_osd()->key_down(59, false, false);
                                softKeyShift = true;
                            } else {
                                softKeyShift = false;
                            }
                            softKeyDelayFlag = true;
                            //emu->get_osd()->key_down(softKeyCode, false, false);
                        }
                        softKeyboardCount = SOFT_KEYBOARD_KEEP_COUNT;
                    }
                }

            } else {
                emu->get_osd()->key_down(code, false, false);
            }
        } else if (action == AKEY_EVENT_ACTION_UP) {
            if ((flags & AKEY_EVENT_FLAG_FROM_SYSTEM) == 0) { //ソフトウェアキーボード

            } else {
                emu->get_osd()->key_up(code, false);
            }
        } else if (action == AKEY_EVENT_ACTION_MULTIPLE) {
            int unicodeCharactor = get_unicode_character(app,
                                                         AInputEvent_getType(event),
                                                         AKeyEvent_getKeyCode(event),
                                                         AKeyEvent_getMetaState(event));
            LOGI("unicodeCharactor:%d", unicodeCharactor);
        }
    }
    return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGB_565;
    struct engine *engine = (struct engine *) app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL) {
                // fill_plasma() assumes 565 format, get it here
                format = ANativeWindow_getFormat(app->window);
                ANativeWindow_setBuffersGeometry(app->window,
                                                 ANativeWindow_getWidth(app->window),
                                                 ANativeWindow_getHeight(app->window),
                                                 WINDOW_FORMAT_RGB_565);
                engine_draw_frame(engine);
                deviceInfo.width = ANativeWindow_getWidth(app->window);
                deviceInfo.height = ANativeWindow_getHeight(app->window);
                clear_screen(engine);
                engine->animating = 1;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine_term_display(engine);
            format = ANativeWindow_getFormat(app->window);
            ANativeWindow_setBuffersGeometry(app->window,
                                             ANativeWindow_getWidth(app->window),
                                             ANativeWindow_getHeight(app->window),
                                             WINDOW_FORMAT_RGB_565);
            engine_draw_frame(engine);
            deviceInfo.width = ANativeWindow_getWidth(app->window);
            deviceInfo.height = ANativeWindow_getHeight(app->window);
            clear_screen(engine);
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            //engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

DWORD timeGetTime() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    //return ts.tv_sec * 1000000L + ts.tv_nsec / 1000;
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000;
}

void selectDialog(struct android_app *state, const char *message, const char *addPath) {
    char long_message[_MAX_PATH+33];

    fileList.clear();

    const char *aplicationPath = get_application_path();
    char dirPath[_MAX_PATH];
    sprintf(dirPath, "%s%s", aplicationPath, addPath);
    DIR *dir;
    struct dirent *dp;
    dir = opendir(dirPath);

    std::string filenameList;
    filenameList = "[EJECT]";
    fileList.push_back("");

    if (dir == NULL) {
        char errorMessage[32];
        sprintf(errorMessage, "Directory does not exist: %s", addPath);
        sprintf(long_message, "%s\n%s", errorMessage, dirPath);
        showAlert(state, long_message, filenameList.c_str(), true, MEDIA_SELECT);
    } else {
        dp = readdir(dir);

        while (dp != NULL) {
            std::string filename = dp->d_name;
            if (filename.find_first_of(".") != 0) {
                if (filenameList.length() > 0) {
                    filenameList = filenameList + ";" + filename;
                }
                std::string filePath = dirPath;
                filePath = filePath + "/" + filename;
                fileList.push_back(filePath);
            }
            dp = readdir(dir);
        }
        showAlert(state, message, filenameList.c_str(), true, MEDIA_SELECT, 0);
    }

}

#ifdef USE_FLOPPY_DISK

void selectD88Bank(struct android_app *state, int driveNo) {
    if (emu->d88_file[driveNo].bank_num <= 1) {
        return;
    }
    char message[32];
    sprintf(message, "Select DISK %d BANK", driveNo);
    std::string disknameList;
    for (int index = 0; index < emu->d88_file[driveNo].bank_num; index++) {
        std::string diskname = emu->d88_file[driveNo].disk_name[index];
        if (disknameList.length() > 0) {
            disknameList = disknameList + ";" + diskname;
        } else {
            disknameList = diskname;
        }
    }
    showAlert(state, message, disknameList.c_str(), true, DISK_BANK_SELECT, 0);
}

#endif

void selectBootMode(struct android_app *state) {

    char message[128];
    sprintf(message, "Reset Emulator?");
    std::string itemList;
#ifdef _PC8801MA
    itemList="N88-V1(S) mode;N88-V1(H) mode;N88-V2 mode;N mode;N88-V2(CD) mode";
#endif

    showAlert(state, message, itemList.c_str(), true, BOOT_MODE_SELECT, 0);
}

#ifdef USE_FLOPPY_DISK

void selectDisk(struct android_app *state, int driveNo) {
    char message[32];
    if (emu->d88_file[driveNo].bank_num > 0 && emu->d88_file[driveNo].cur_bank != -1) {
        sprintf(message, "DISK[%d] %s", driveNo,
                emu->d88_file[driveNo].disk_name[emu->d88_file[driveNo].cur_bank]);
    } else {
        sprintf(message, "Select DISK[%d]", driveNo);
    }
    selectDialog(state, message, "DISK");
}

#endif

void selectTape(struct android_app *state) {
    char message[32];
    sprintf(message, "Select TAPE and Play");
    selectDialog(state, message, "TAPE");
}

void selectCart(struct android_app *state, int driveNo) {
    char message[32];
    sprintf(message, "Select CARTRIDGE[%d]", driveNo);
    selectDialog(state, message, "CART");
}

void selectQuickDisk(struct android_app *state, int driveNo) {
    char message[32];
    sprintf(message, "Select QD[%d]", driveNo);
    selectDialog(state, message, "QD");
}

void selectMedia(struct android_app *state) {
    if (fileSelectIconData[selectingIconIndex].fileSelectType < 0) {
        return;
    }
    switch (fileSelectIconData[selectingIconIndex].fileSelectType) {
#ifdef USE_FLOPPY_DISK
        case FLOPPY_DISK:
            selectDisk(state, fileSelectIconData[selectingIconIndex].driveNo);
            break;
#endif
#ifdef USE_TAPE
        case CASETTE_TAPE:
            selectTape(state);
            break;
#endif
#ifdef USE_CART
        case CARTRIDGE:
            selectCart(state, fileSelectIconData[selectingIconIndex].driveNo);
            break;
#endif
#ifdef USE_QUICK_DISK
        case QUICK_DISK:
            selectQuickDisk(state, fileSelectIconData[selectingIconIndex].driveNo);
            break;
#endif

    }

    return;
}


/** @return the id of the button clicked if model is true, or 0 */
jint showAlert(struct android_app *state, const char *message, const char *itemNames,
               bool model /* = false */, int selectMode, int selectIndex) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass clazz = jni->GetObjectClass(state->activity->clazz);

    // Get the ID of the method we want to call
    // This must match the name and signature from the Java side Signature has to match java
    // implementation (second string hints a java string parameter)
    jmethodID methodID = jni->GetMethodID(clazz, "showAlert",
                                          "(Ljava/lang/String;Ljava/lang/String;ZI)I");

    // Strings passed to the function need to be converted to a java string object
    jstring jmessage = jni->NewStringUTF(message);
    jobjectArray day = 0;
    jstring jfilenames = jni->NewStringUTF(itemNames);

    jint result = jni->CallIntMethod(state->activity->clazz, methodID, jmessage, jfilenames, model,
                                     selectMode);

    // Remember to clean up passed values
    jni->DeleteLocalRef(jmessage);

    state->activity->vm->DetachCurrentThread();

    return result;
}

#ifdef USE_FLOPPY_DISK

void open_floppy_disk(int drv, const _TCHAR *path, int bank) {
    emu->d88_file[drv].bank_num = 0;
    emu->d88_file[drv].cur_bank = -1;

    if (check_file_extension(path, _T(".d88")) || check_file_extension(path, _T(".d77")) ||
        check_file_extension(path, _T(".1dd"))) {
        FILEIO *fio = new FILEIO();
        if (fio->Fopen(path, FILEIO_READ_BINARY)) {
            try {
                fio->Fseek(0, FILEIO_SEEK_END);
                uint32_t file_size = fio->Ftell(), file_offset = 0;
                while (file_offset + 0x2b0 <= file_size &&
                       emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
                    fio->Fseek(file_offset, FILEIO_SEEK_SET);
//#ifdef _UNICODE
                    char tmp[18];
                    fio->Fread(tmp, 17, 1);
                    tmp[17] = 0;
                    convertUTF8fromSJIS(tmp,
                                        emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num],
                                        17 * 3);
//#else
//                    fio->Fread(emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num], 17, 1);
//                    emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num][17] = 0;
//#endif
                    fio->Fseek(file_offset + 0x1c, SEEK_SET);
                    file_offset += fio->FgetUint32_LE();
                    emu->d88_file[drv].bank_num++;
                }
                my_tcscpy_s(emu->d88_file[drv].path, _MAX_PATH, path);
                emu->d88_file[drv].cur_bank = bank;
            } catch (...) {
                emu->d88_file[drv].bank_num = 0;
            }
            fio->Fclose();
        }
        delete fio;
    }
    emu->open_floppy_disk(drv, path, bank);
#if USE_FLOPPY_DISK >= 2
    if ((drv & 1) == 0 && drv + 1 < USE_FLOPPY_DISK && bank + 1 < emu->d88_file[drv].bank_num) {
        open_floppy_disk(drv + 1, path, bank + 1);
    }
#endif
}

void select_d88_bank(int drv, int index) {
    if (emu->d88_file[drv].cur_bank != index) {
        emu->open_floppy_disk(drv, emu->d88_file[drv].path, index);
        emu->d88_file[drv].cur_bank = index;
    }
}

void close_floppy_disk(int drv) {
    emu->close_floppy_disk(drv);
    emu->d88_file[drv].bank_num = 0;
    emu->d88_file[drv].cur_bank = -1;
}

#endif


extern "C" {
// グローバル参照を保持するための変数

JavaVM* g_JavaVM = nullptr;
jobject g_ActivityObject = nullptr;
bool grantedStorage = false;
static struct android_app* globalAppState = nullptr;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_nativeOnPermissionsGranted(JNIEnv *env, jobject obj) {
    // 権限が付与された後の処理をここに実装
    grantedStorage = true;

    // カスタムイベントを送信してメインスレッドに通知
    if (globalAppState != nullptr) {
        ALooper_wake(ALooper_forThread());
    }
}

JNIEXPORT void JNICALL Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_nativeOnPermissionsDenied(JNIEnv *env, jobject obj) {
    // 権限が拒否された場合の処理をここに実装
}

void checkPermissionsAndInitialize(JNIEnv *env, jobject activity) {
    jclass clazz = env->GetObjectClass(activity);
    jmethodID methodID = env->GetMethodID(clazz, "checkPermissionsAsync", "()V");
    env->CallVoidMethod(activity, methodID);
}

JNIEXPORT void JNICALL
Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_fileSelectCallback(JNIEnv *env, jobject thiz,
                                                                     jint id) {
    LOGI("fileSelectCallback %d", id);
    if (fileList.size() < id) {
        return;
    }
    if (id == 0) { //EJECT
        switch (fileSelectIconData[selectingIconIndex].fileSelectType) {
#ifdef USE_FLOPPY_DISK
            case FLOPPY_DISK:
                close_floppy_disk(fileSelectIconData[selectingIconIndex].driveNo);
#endif
#ifdef USE_TAPE
            case CASETTE_TAPE:
                emu->close_tape(0);
                break;
#endif
#ifdef USE_CART
            case CARTRIDGE:
                    emu->close_cart(fileSelectIconData[selectingIconIndex].driveNo);
                    break;
#endif
        }
    } else {

        std::string filePath = fileList.at(id);

        switch (fileSelectIconData[selectingIconIndex].fileSelectType) {
#ifdef USE_FLOPPY_DISK
            case FLOPPY_DISK:
                open_floppy_disk(fileSelectIconData[selectingIconIndex].driveNo,
                                 filePath.c_str(), 0);
                LOGI("D88 bankNo:%d",
                     emu->d88_file[fileSelectIconData[selectingIconIndex].driveNo].bank_num);
                if (emu->d88_file[fileSelectIconData[selectingIconIndex].driveNo].bank_num > 1) {
                    needSelectDiskBank = true;
                    selectDiskDrive = fileSelectIconData[selectingIconIndex].driveNo;
                    //selectD88Bank(selectDiskDrive); //メインループから呼び出す
                }
                break;
#endif
#ifdef USE_TAPE
            case CASETTE_TAPE:
                if (emu->is_tape_inserted(0)) {
                    emu->close_tape(0);
                }
                emu->play_tape(0, filePath.c_str());
                break;
#endif
#ifdef USE_QUICK_DISK
            case QUICK_DISK:
                if (emu->is_quick_disk_inserted(0)) {
                    emu->close_quick_disk(0);
                }
                emu->open_quick_disk(0, filePath.c_str());
                break;
#endif
#ifdef USE_CART
            case CARTRIDGE:
                emu->open_cart(fileSelectIconData[selectingIconIndex].driveNo , filePath.c_str());
                break;
#endif
        }
    }
    fileList.clear();
}
} //extern"C"

extern "C" {
JNIEXPORT void JNICALL
Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_bankSelectCallback(JNIEnv *env, jobject thiz,
                                                                     jint id) {

    LOGI("bankSelectCallback %d", id);
    if (id < 0) {
        return;
    }
#ifdef USE_FLOPPY_DISK
    if (emu->d88_file[fileSelectIconData[selectingIconIndex].driveNo].cur_bank != id) {
        emu->open_floppy_disk(fileSelectIconData[selectingIconIndex].driveNo,
                              emu->d88_file[fileSelectIconData[selectingIconIndex].driveNo].path,
                              id);
        emu->d88_file[fileSelectIconData[selectingIconIndex].driveNo].cur_bank = id;
    }
#endif
}
} //extern"C"

extern "C" {
JNIEXPORT void JNICALL
Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_bootSelectCallback(JNIEnv *env, jobject thiz,
                                                                     jint id) {
    if (id < 0) {
        return;
    }
#ifdef _PC8801MA
    config.boot_mode = id;
    if(emu) {
        emu->update_config();
    }
#endif
    resetFlag = true;
}
} //extern"C"

extern "C" {
JNIEXPORT void JNICALL
Java_jp_matrix_shikarunochi_emulator_EmulatorActivity_exitSelectCallback(JNIEnv *env, jobject thiz,
                                                                     jint id) {
    if (id < 0) {
        return;
    }
    //TODO:free?
    exit(0);
}
} //extern"C"

const char *jniGetDocumentPath(struct android_app *state) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass environmentClass = jni->FindClass("android/os/Environment");
    jfieldID fieldID = jni->GetStaticFieldID(environmentClass, "DIRECTORY_DOCUMENTS", "Ljava/lang/String;");
    jobject DIRECTORY_DOCUMENTS = jni->GetStaticObjectField(environmentClass, fieldID);

    jmethodID getStoragePDirMethod = jni->GetStaticMethodID(environmentClass, "getExternalStoragePublicDirectory", "(Ljava/lang/String;)Ljava/io/File;");
    jobject fileObject = jni->CallStaticObjectMethod(environmentClass, getStoragePDirMethod, DIRECTORY_DOCUMENTS);

    jclass fileClass = jni->FindClass("java/io/File");
    jmethodID getPathMethod = jni->GetMethodID(fileClass, "getPath", "()Ljava/lang/String;");
    jobject strPathObject = jni->CallObjectMethod(fileObject, getPathMethod);

    const char *documentDirPath = jni->GetStringUTFChars((jstring) strPathObject, 0);
    char *dup = strdup(documentDirPath);  // Duplicate the string to return

    jni->ReleaseStringUTFChars((jstring) strPathObject, documentDirPath);  // Release temporary JNI string
    jni->DeleteLocalRef(strPathObject);  // Clean up local references
    jni->DeleteLocalRef(fileObject);
    jni->DeleteLocalRef(DIRECTORY_DOCUMENTS);
    jni->DeleteLocalRef(fileClass);
    jni->DeleteLocalRef(environmentClass);

    state->activity->vm->DetachCurrentThread();
    return dup;  // Return the duplicated string
}

const char *jniGetExternalStoragePath(struct android_app *state) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    // アプリ固有の外部ストレージディレクトリを取得
    jclass contextClass = jni->FindClass("android/content/Context");
    jmethodID getExternalFilesDirMethod = jni->GetMethodID(contextClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
    jobject fileObject = jni->CallObjectMethod(state->activity->clazz, getExternalFilesDirMethod, NULL);

    jclass fileClass = jni->FindClass("java/io/File");
    jmethodID getPathMethod = jni->GetMethodID(fileClass, "getPath", "()Ljava/lang/String;");
    jobject pathObject = jni->CallObjectMethod(fileObject, getPathMethod);

    const char *path = jni->GetStringUTFChars((jstring) pathObject, NULL);
    char *dup = strdup(path);

    jni->ReleaseStringUTFChars((jstring) pathObject, path);
    state->activity->vm->DetachCurrentThread();

    return dup;  // 呼び出し側で free() する必要がある
}

const char *jniGetDownloadPath(struct android_app *state) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass environmentClass = jni->FindClass("android/os/Environment");
    jfieldID downloadDirField = jni->GetStaticFieldID(environmentClass, "DIRECTORY_DOWNLOADS", "Ljava/lang/String;");
    jobject downloadDir = jni->GetStaticObjectField(environmentClass, downloadDirField);

    jmethodID getExternalStoragePublicDirectoryMethod = jni->GetStaticMethodID(environmentClass, "getExternalStoragePublicDirectory", "(Ljava/lang/String;)Ljava/io/File;");
    jobject fileObject = jni->CallStaticObjectMethod(environmentClass, getExternalStoragePublicDirectoryMethod, downloadDir);

    jclass fileClass = jni->FindClass("java/io/File");
    jmethodID getPathMethod = jni->GetMethodID(fileClass, "getPath", "()Ljava/lang/String;");
    jobject pathObject = jni->CallObjectMethod(fileObject, getPathMethod);

    const char *path = jni->GetStringUTFChars((jstring) pathObject, NULL);
    char *dup = strdup(path);

    jni->ReleaseStringUTFChars((jstring) pathObject, path);
    state->activity->vm->DetachCurrentThread();

    return dup; // 呼び出し側で free() する必要がある
}

const char *jniGetSdcardDownloadPath(struct android_app *state) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass environmentClass = jni->FindClass("android/os/Environment");
    jmethodID getExternalStorageDirectoryMethod = jni->GetStaticMethodID(environmentClass, "getExternalStorageDirectory", "()Ljava/io/File;");
    jobject externalStorageFileObject = jni->CallStaticObjectMethod(environmentClass, getExternalStorageDirectoryMethod);

    jclass fileClass = jni->FindClass("java/io/File");
    jmethodID getPathMethod = jni->GetMethodID(fileClass, "getPath", "()Ljava/lang/String;");
    jstring externalStoragePath = (jstring) jni->CallObjectMethod(externalStorageFileObject, getPathMethod);

    const char *externalPath = jni->GetStringUTFChars(externalStoragePath, NULL);
    const char *downloadPathSuffix = "/Download";
    char *downloadPath = (char *)malloc(strlen(externalPath) + strlen(downloadPathSuffix) + 1); // +1 for the null terminator
    strcpy(downloadPath, externalPath);
    strcat(downloadPath, downloadPathSuffix);

    jni->ReleaseStringUTFChars(externalStoragePath, externalPath);
    state->activity->vm->DetachCurrentThread();

    return downloadPath; // 呼び出し側で free() する必要がある
}

void jniReadIconData(struct android_app *state) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass clazz = jni->GetObjectClass(state->activity->clazz);
    jmethodID jIDLoadBitmap = jni->GetMethodID(clazz, "loadBitmap", "(II)[I");
    int width, height;

    for (int iconType = 0; iconType < 2; iconType++) {
        int iconNumber = 0;
        if (iconType == 0) {
            iconNumber = SYSTEM_ICON_MAX;
        } else {
            iconNumber = FILE_SELECT_TYPE_MAX;
        }

        for (int iconIndex = 0; iconIndex < iconNumber; iconIndex++) {
            LOGI("Load Icon:%d", iconIndex);
            jintArray ja = (jintArray) (jni->CallObjectMethod(state->activity->clazz, jIDLoadBitmap,
                                                              iconType, iconIndex));

            int jasize = jni->GetArrayLength(ja);

            jint *arr1;
            arr1 = jni->GetIntArrayElements(ja, 0);
            int cnt;
            // 画像の幅を取得
            width = arr1[0];
            // 画像の縦サイズを取得
            height = arr1[1];
            if (iconType == 0) {
                systemIconData[iconIndex].height = height;
                systemIconData[iconIndex].width = width;

                systemIconData[iconIndex].bmpImage = new uint16_t[height * width];
                for (int index = 0; index < jasize; index++) {
                    systemIconData[iconIndex].bmpImage[index] = arr1[index + 2];
                }
            } else {
                mediaIconData[iconIndex].height = height;
                mediaIconData[iconIndex].width = width;

                mediaIconData[iconIndex].bmpImage = new uint16_t[height * width];
                for (int index = 0; index < jasize; index++) {
                    mediaIconData[iconIndex].bmpImage[index] = arr1[index + 2];
                }
            }
            jni->ReleaseIntArrayElements(ja, arr1, 0);
        }
    }
    state->activity->vm->DetachCurrentThread();
}

void setFileSelectIcon() {
    for(int iconIndex = 0;iconIndex < MAX_FILE_SELECT_ICON;iconIndex++){
        fileSelectIconData[iconIndex].fileSelectType = FILE_SELECT_NONE;
    }
    int index = 0;
    //FD
#ifdef USE_FLOPPY_DISK
    fileSelectIconData[index].fileSelectType = FLOPPY_DISK;
    fileSelectIconData[index].driveNo = 0;
    index++;
    if (USE_FLOPPY_DISK >= 2) {
        fileSelectIconData[index].fileSelectType = FLOPPY_DISK;
        fileSelectIconData[index].driveNo = 1;
        index++;
    }
#endif

#ifdef USE_QUICK_DISK
    fileSelectIconData[index].fileSelectType = QUICK_DISK;
    fileSelectIconData[index].driveNo = 0;
    index++;
#endif

#ifdef USE_TAPE
    fileSelectIconData[index].fileSelectType = CASETTE_TAPE;
    fileSelectIconData[index].driveNo = 0;
    index++;
#endif

#ifdef USE_CART
    fileSelectIconData[index].fileSelectType = CARTRIDGE;
    fileSelectIconData[index].driveNo=0;
    index++;
    if(USE_CART >=2){
        fileSelectIconData[index].fileSelectType = CARTRIDGE;
        fileSelectIconData[index].driveNo=1;
        index++;
    }
#endif
}


BitmapData
jniCreateBitmapFromString(struct android_app *state, const char *text, int fontSize) {
    JNIEnv *jni = NULL;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass clazz = jni->GetObjectClass(state->activity->clazz);
    jmethodID jCreateBitmapFromString = jni->GetMethodID(clazz, "createBitmapFromString",
                                                         "(Ljava/lang/String;I)[I");

    int width, height;
    jstring jtext = jni->NewStringUTF(text);
    jintArray ja = (jintArray) (jni->CallObjectMethod(state->activity->clazz,
                                                      jCreateBitmapFromString, jtext,
                                                      fontSize));
    int jasize = jni->GetArrayLength(ja);
    jint *arr1;
    arr1 = jni->GetIntArrayElements(ja, 0);
    // 画像の幅を取得
    width = arr1[0];
    // 画像の縦サイズを取得
    height = arr1[1];

    BitmapData returnBitmapData;
    returnBitmapData.width = width;
    returnBitmapData.height = height;
    returnBitmapData.bmpImage = new uint16_t[width * height];
    for (int index = 0; index < width * height; index++) {
        returnBitmapData.bmpImage[index] = arr1[index + 2];
    }
    jni->ReleaseIntArrayElements(ja, arr1, 0);

    state->activity->vm->DetachCurrentThread();
    return returnBitmapData;
}

bool get_status_bar_updated() {
    bool updated = false;

#ifdef USE_FLOPPY_DISK
    uint32_t new_fd_status = emu->is_floppy_disk_accessed();
    if (fd_status != new_fd_status) {
        updated = true;
        fd_status = new_fd_status;
    }
#endif
#ifdef USE_QUICK_DISK
    uint32_t new_qd_status = emu->is_quick_disk_accessed();
    if (qd_status != new_qd_status) {
        updated = true;
        qd_status = new_qd_status;
    }
#endif
#ifdef USE_HARD_DISK
    uint32_t new_hd_status = emu->is_hard_disk_accessed();
    if(hd_status != new_hd_status) {
        updated = true;
        hd_status = new_hd_status;
    }
#endif
#ifdef USE_COMPACT_DISC
    uint32_t new_cd_status = emu->is_compact_disc_accessed();
    if(cd_status != new_cd_status) {
        updated = true;
        cd_status = new_cd_status;
    }
#endif
#ifdef USE_LASER_DISC
    uint32_t new_ld_status = emu->is_laser_disc_accessed();
    if(ld_status != new_ld_status) {
        updated = true;
        ld_status = new_ld_status;
    }
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
    _TCHAR new_tape_status[array_length(tape_status)] = {0};
    for (int drv = 0; drv < USE_TAPE; drv++) {
        tape_position = emu->get_tape_position(drv);
        const _TCHAR *message = emu->get_tape_message(drv);
        if (message != NULL) {
            my_tcscpy_s(new_tape_status, array_length(new_tape_status), message);
            break;
        }
    }
    if (_tcsicmp(tape_status, new_tape_status) != 0) {
        updated = true;
        my_tcscpy_s(tape_status, array_length(tape_status), new_tape_status);
    }
#endif
    return updated;
}

void android_main(struct android_app *state) {

    ANativeActivity_setWindowFlags(state->activity,AWINDOW_FLAG_KEEP_SCREEN_ON , 0);    //Sleepさせない

    static int init;
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
    engine.emu_initialized = false;

    // アプリケーションの状態をグローバル変数に保持
    globalAppState = state;

    {
        JNIEnv* env = nullptr;
        state->activity->vm->AttachCurrentThread(&env, nullptr);

        // 権限チェックと初期化を非同期で行う
        checkPermissionsAndInitialize(env, state->activity->clazz);

        state->activity->vm->DetachCurrentThread();
    }

    //const char *documentDirTemp = jniGetDocumentPath(state);
    //const char *documentDirTemp = jniGetExternalStoragePath(state);
    //const char *documentDirTemp = jniGetDownloadPath(state);
    const char *documentDirTemp = jniGetSdcardDownloadPath(state);

    sprintf(documentDir, "%s", documentDirTemp);
    free((void*)documentDirTemp);
    LOGI("documentDir: %s", documentDir);

    //Read Icon Image
    jniReadIconData(state);
    setFileSelectIcon();

    initialize_config();

    // 権限が付与されるまで先に進まないループ
    while (1) {
        int events;
        struct android_poll_source* source;

        int ident = ALooper_pollAll(-1, NULL, &events, (void **) &source);
        if (source) {
            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }
        }
        // 他のスレッドに処理を譲るために少し待機
        usleep(1000 * 1000);  // 1秒待機
        if (grantedStorage) {
            // 権限が許可された後の処理
            break;
        }
    }

    // 権限付与待機ループ処理後のクリーンアップ
    globalAppState = nullptr;

    emu = new EMU(state);
    engine.emu_initialized = true;

    if (!init) {
        init_tables();
        init = 1;
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    start_ms = (((int64_t) now.tv_sec) * 1000000000LL + now.tv_nsec) / 1000000;

    stats_init(&engine.stats);

    // loop waiting for stuff to do.

    int total_frames = 0, draw_frames = 0, skip_frames = 0;
    DWORD next_time = 0;
    bool prev_skip = false;
    DWORD update_fps_time = 0;
    DWORD update_status_bar_time = 0;
    DWORD disable_screen_saver_time = 0;
    bool needDraw = false;

    resetFlag = false;
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source *source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        //while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
        //                             (void**)&source)) >= 0) {
        ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                (void **) &source);

        // Process this event.
        if (source != NULL) {
            source->process(state, source);
        }

        // Check if we are exiting.
        if (state->destroyRequested != 0) {
            LOGI("Engine thread destroy requested!");
            engine_term_display(&engine);
            return;
        }
        if (resetFlag == true) {
            engine.animating = 0;
#ifdef USE_TAPE
            emu->close_tape(0);
#endif
            emu->reset();
            needDraw = true;
            resetFlag = false;
            engine.animating = 1;
        }

        if (emu) {
            // drive machine
            int run_frames = emu->run();
            total_frames += run_frames;

            // timing controls
            int sleep_period = 0;
            bool now_skip = (config.full_speed || emu->is_frame_skippable()) &&
                            !emu->is_video_recording() && !emu->is_sound_recording();

            if ((prev_skip && !now_skip) || next_time == 0) {
                next_time = timeGetTime();
            }
            if (!now_skip) {
                static int accum = 0;
                accum += emu->get_frame_interval();
                int interval = accum >> 10;
                accum -= interval << 10;
                next_time += interval;
            }
            prev_skip = now_skip;

            if (next_time > timeGetTime()) {
                // update window if enough time
                draw_frames += emu->draw_screen();
                needDraw = true;
                skip_frames = 0;
                // sleep 1 frame priod if need
                DWORD current_time = timeGetTime();
                if ((int) (next_time - current_time) >= 10) {
                    sleep_period = next_time - current_time;
                }
            } else if (++skip_frames > (int) emu->get_frame_rate()) {
                // update window at least once per 1 sec in virtual machine time
                draw_frames += emu->draw_screen();
                needDraw = true;
                skip_frames = 0;
                next_time = timeGetTime();
            }
            usleep(sleep_period);

            // calc frame rate
            DWORD current_time = timeGetTime();
            if (update_fps_time <= current_time) {
                if (update_fps_time != 0) {
                    if (emu->message_count > 0) {
                        //        SetWindowText(hWnd, create_string(_T("%s - %s"), _T(DEVICE_NAME), emu->message));
                        emu->message_count--;
                    } else if (now_skip) {
                        //        int ratio = (int)(100.0 * (double)total_frames / emu->get_frame_rate() + 0.5);
                        //        SetWindowText(hWnd, create_string(_T("%s - Skip Frames (%d %%)"), _T(DEVICE_NAME), ratio));
                    } else {
                        //       int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
                        //       SetWindowText(hWnd, create_string(_T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), draw_frames, ratio));
                    }
                    update_fps_time += 1000;
                    total_frames = draw_frames = 0;
                }
                update_fps_time = current_time + 1000;
            }

            // update status bar
            if (update_status_bar_time <= current_time) {
                if (get_status_bar_updated()) {
                    //engine_draw_message(&engine, tape_status);
                }
                update_status_bar_time = current_time + 200;
            }

        }

        if(softKeyDelayFlag == true){
            emu->get_osd()->key_down(softKeyCode, false, false);
            softKeyDelayFlag = false;
        }

        if (engine.animating && needDraw) {
            engine_draw_frame(&engine);
            needDraw = false;
            if (softKeyboardCount > 0) {
                softKeyboardCount--;
                if (softKeyboardCount == 0) {
                    if (softKeyCtrl == true) {
                        emu->get_osd()->key_up(113, false);
                    }
                    if (softKeyShift == true) {
                        emu->get_osd()->key_up(59, false);
                    }
                    emu->get_osd()->key_up(softKeyCode, false);
                    softKeyShift = false;
                    softKeyCtrl = false;
                    softKeyCode = 0;
                } else {

#if defined(_MZ700)
                    //機種によってはしばらくの間keyDownを送り続ける必要がある…と思ったけど、やらなくても大丈夫でした。
                    //emu->get_osd()->key_down(softKeyCode,false,false);
#endif
                }
            }
        }
#ifdef USE_FLOPPY_DISK
        if (needSelectDiskBank == true) {
            needSelectDiskBank = false;
            selectD88Bank(state, selectDiskDrive);
        }
#endif
    }
}
