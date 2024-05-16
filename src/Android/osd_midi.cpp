/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ win32 midi ]
*/

#include "osd.h"
#include "../fifo.h"
#include <thread>
#include <mutex>
#include <unistd.h>

std::mutex send_mutex;
std::mutex recv_mutex;

#ifdef USE_MIDI
midi_thread_params_t midi_thread_params;

extern "C" JNIEXPORT void JNICALL
Java_jp_matrix_shikarunochi_emulator_MidiManagerActivity_passMidiMessageToNative(JNIEnv *env, jobject thiz, jbyteArray buffer, jint length) {
    jbyte *nativeBuffer = env->GetByteArrayElements(buffer, 0);
    std::lock_guard<std::mutex> lock(recv_mutex);
    for (int i = 0; i < length; i++) {
        midi_thread_params.recv_buffer->write(static_cast<uint8_t>(nativeBuffer[i]));
    }
    env->ReleaseByteArrayElements(buffer, nativeBuffer, 0);
}

void midi_thread_function(OSD *osd) {
    JNIEnv *env;
    JavaVM *vm = osd->state->activity->vm;
    vm->AttachCurrentThread(&env, nullptr);

    jclass clazz = env->GetObjectClass(osd->state->activity->clazz);
    jmethodID getMidiDeviceInfo = env->GetMethodID(clazz, "getMidiDeviceInfo", "(I)Ljava/lang/String;");
    jmethodID sendMidiMessage = env->GetMethodID(clazz, "sendMidiMessage", "([B)V");

    while (!midi_thread_params.terminate) {
        while (true) {
            uint8_t buffer[128];
            int length = 0;

            {
                std::lock_guard<std::mutex> lock(send_mutex);
                if (!midi_thread_params.send_buffer->empty()) {
                    uint8_t msg = midi_thread_params.send_buffer->read_not_remove(0);

                    switch (msg & 0xf0) {
                        case 0x80: case 0x90: case 0xa0: case 0xb0: case 0xe0:
                            length = 3;
                            break;
                        case 0xc0:
                        case 0xd0:
                            length = 2;
                            break;
                        case 0xf0:
                            switch (msg) {
                                case 0xf0:
                                    for (int i = 1; i < midi_thread_params.send_buffer->count(); i++) {
                                        if (midi_thread_params.send_buffer->read_not_remove(i) == 0xf7) {
                                            length = i + 1;
                                            break;
                                        }
                                    }
                                    break;
                                case 0xf1: case 0xf3:
                                    length = 2;
                                    break;
                                case 0xf2:
                                    length = 3;
                                    break;
                                case 0xf6: case 0xf8: case 0xfa: case 0xfb: case 0xfc: case 0xfe: case 0xff:
                                    length = 1;
                                    break;
                                default:
                                    midi_thread_params.send_buffer->read();
                                    break;
                            }
                            break;
                        default:
                            midi_thread_params.send_buffer->read();
                            break;
                    }
                    if (midi_thread_params.send_buffer->count() >= length) {
                        for (int i = 0; i < length; i++) {
                            buffer[i] = midi_thread_params.send_buffer->read();
                        }
                    } else {
                        length = 0;
                    }
                }
            }

            if (length > 0) {
                jbyteArray jBuffer = env->NewByteArray(length);
                env->SetByteArrayRegion(jBuffer, 0, length, reinterpret_cast<jbyte*>(buffer));
                env->CallVoidMethod(osd->state->activity->clazz, sendMidiMessage, jBuffer);
                env->DeleteLocalRef(jBuffer);
            } else {
                break;
            }
        }
        usleep(1000); // 1ミリ秒スリープ
    }

    vm->DetachCurrentThread();
}

void OSD::initialize_midi() {
    midi_thread_params.send_buffer = new FIFO(1024);
    midi_thread_params.recv_buffer = new FIFO(1024);
    midi_thread_params.terminate = false;

    midi_thread = std::thread(midi_thread_function, this);
}

void OSD::release_midi() {
    if (midi_thread.joinable()) {
        midi_thread_params.terminate = true;
        midi_thread.join();
    }

    delete midi_thread_params.send_buffer;
    midi_thread_params.send_buffer = nullptr;

    delete midi_thread_params.recv_buffer;
    midi_thread_params.recv_buffer = nullptr;
}

void OSD::send_to_midi(uint8_t data) {
    std::lock_guard<std::mutex> lock(send_mutex);
    midi_thread_params.send_buffer->write(data);
}

bool OSD::recv_from_midi(uint8_t *data) {
    std::lock_guard<std::mutex> lock(recv_mutex);
    if (!midi_thread_params.recv_buffer->empty()) {
        *data = midi_thread_params.recv_buffer->read();
        return true;
    }
    return false;
}
#endif
