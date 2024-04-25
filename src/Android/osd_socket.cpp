/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

    Ported by MedamaP on 2024/04/04.

	[ Android socket ]
*/

#include "osd.h"

#if defined(USE_SOCKET)

void OSD::initialize_socket()
{
    // init winsock
    // WSADATA wsaData;
    //WSAStartup(0x0101, &wsaData);

    // init sockets
    for(int i = 0; i < SOCKET_MAX; i++) {
        soc[i] = INVALID_SOCKET;
        socket_delay[i] = 0;
        recv_r_ptr[i] = recv_w_ptr[i] = 0;
    }
}

void OSD::release_socket()
{
    // release sockets
    for(int i = 0; i < SOCKET_MAX; i++) {
        if(soc[i] != INVALID_SOCKET) {
            shutdown(soc[i], 2);
            close(soc[i]);
        }
    }

    // release winsock
    //WSACleanup();
}

void OSD::notify_socket_connected(int ch)
{
    // winmain notify that network is connected
    vm->notify_socket_connected(ch);
}

void OSD::notify_socket_disconnected(int ch)
{
    // winmain notify that network is disconnected
    if(!socket_delay[ch]) {
        socket_delay[ch] = 1;//56;
    }
}

void OSD::update_socket()
{
    for(int i = 0; i < SOCKET_MAX; i++) {
        if(recv_r_ptr[i] < recv_w_ptr[i]) {
            // get buffer
            int size0, size1;
            uint8_t* buf0 = vm->get_socket_recv_buffer0(i, &size0, &size1);
            uint8_t* buf1 = vm->get_socket_recv_buffer1(i);

            int size = recv_w_ptr[i] - recv_r_ptr[i];
            if(size > size0 + size1) {
                size = size0 + size1;
            }
            char* src = &recv_buffer[i][recv_r_ptr[i]];
            recv_r_ptr[i] += size;

            if(size <= size0) {
                memcpy(buf0, src, size);
            } else {
                memcpy(buf0, src, size0);
                memcpy(buf1, src + size0, size - size0);
            }
            vm->inc_socket_recv_buffer_ptr(i, size);
        } else if(socket_delay[i] != 0) {
            if(--socket_delay[i] == 0) {
                vm->notify_socket_disconnected(i);
            }
        }
    }
}

bool OSD::initialize_socket_tcp(int ch) {
    is_tcp[ch] = true;

    if(soc[ch] != INVALID_SOCKET) {
        disconnect_socket(ch);
    }
    soc[ch] = socket(PF_INET, SOCK_STREAM, 0);
    if (soc[ch] == INVALID_SOCKET) {
        return false;
    }

    // 非同期通知の設定は、select または epoll を使った状態監視に置き換える
    // ここでは、ソケットをノンブロッキングモードに設定する例を示す
    int flags = fcntl(soc[ch], F_GETFL, 0);
    if (flags == -1 || fcntl(soc[ch], F_SETFL, flags | O_NONBLOCK) == -1) {
        close(soc[ch]);
        soc[ch] = -1;
        return false;
    }

    recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
    return true;
}

bool OSD::initialize_socket_udp(int ch) {
    is_tcp[ch] = false;

    disconnect_socket(ch);
    soc[ch] = socket(PF_INET, SOCK_DGRAM, 0);  // UDPソケットの作成
    if (soc[ch] == INVALID_SOCKET) {  // INVALID_SOCKET の代わりに -1 を使用
        return false;
    }

    // ソケットをノンブロッキングモードに設定
    int flags = fcntl(soc[ch], F_GETFL, 0);
    if (flags == -1 || fcntl(soc[ch], F_SETFL, flags | O_NONBLOCK) == -1) {
        close(soc[ch]);
        soc[ch] = INVALID_SOCKET;  // INVALID_SOCKET の代わりに -1 を使用
        return false;
    }

    recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
    return true;
}

bool OSD::connect_socket(int ch, uint32_t ipaddr, int port) {
    struct sockaddr_in tcpaddr;
    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_addr.s_addr = htonl(ipaddr);  // ネットワークバイトオーダーに変換
    tcpaddr.sin_port = htons(static_cast<unsigned short>(port));
    memset(&(tcpaddr.sin_zero), 0, sizeof(tcpaddr.sin_zero));  // sin_zero を0で埋める

    if (connect(soc[ch], (struct sockaddr *)&tcpaddr, sizeof(tcpaddr)) == -1) {
        if (errno != EINPROGRESS) {  // 非ブロッキングモードの場合は EINPROGRESS が返される
            return false;
        }
    }
    return true;
}

void OSD::disconnect_socket(int ch)
{
    if (soc[ch] != INVALID_SOCKET) {  // INVALID_SOCKET の代わりに -1 を使用
        shutdown(soc[ch], SHUT_RDWR);  // 2 の代わりに SHUT_RDWR を使用
        close(soc[ch]);  // closesocket の代わりに close を使用
        soc[ch] = INVALID_SOCKET;  // INVALID_SOCKET の代わりに -1 を使用
    }
    vm->notify_socket_disconnected(ch);
}
bool OSD::listen_socket(int ch)
{
    return false;
}

void OSD::send_socket_data_tcp(int ch)
{
    if(is_tcp[ch]) {
        send_socket_data(ch);
    }
}

void OSD::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
    if(!is_tcp[ch]) {
        udpaddr[ch].sin_family = AF_INET;
        udpaddr[ch].sin_addr.s_addr = ipaddr;
        udpaddr[ch].sin_port = htons((unsigned short)port);
        memset(udpaddr[ch].sin_zero, (int)0, sizeof(udpaddr[ch].sin_zero));

        send_socket_data(ch);
    }
}

void OSD::send_socket_data(int ch) {
    // ループ：送信バッファが空でない、またはブロッキングエラーが発生するまで
    while (true) {
        // 送信バッファとデータサイズを取得
        int size;
        uint8_t* buf = vm->get_socket_send_buffer(ch, &size);

        if (size == 0) {
            return;
        }
        if (is_tcp[ch]) {
            size = send(soc[ch], buf, size, 0);
            if (size == SOCKET_ERROR) {  // SOCKET_ERROR の代わりに -1 を使用
                // EWOULDBLOCK の場合、後で書き込み可能イベントが発生
                if (errno != EWOULDBLOCK) {
                    disconnect_socket(ch);
                    notify_socket_disconnected(ch);
                }
                return;
            }
        } else {
            size = sendto(soc[ch], buf, size, 0, (struct sockaddr *)&udpaddr[ch], sizeof(udpaddr[ch]));
            if (size == SOCKET_ERROR) {  // SOCKET_ERROR の代わりに -1 を使用
                // EWOULDBLOCK の場合、後で書き込み可能イベントが発生
                if (errno != EWOULDBLOCK) {
                    disconnect_socket(ch);
                    notify_socket_disconnected(ch);
                }
                return;
            }
        }
        vm->inc_socket_send_buffer_ptr(ch, size);
    }
}

void OSD::recv_socket_data(int ch) {
    if(is_tcp[ch]) {
        int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
        char* buf = &recv_buffer[ch][recv_w_ptr[ch]];
        if((size = recv(soc[ch], buf, size, 0)) == -1) {  // SOCKET_ERROR を -1 に置き換え
            disconnect_socket(ch);
            notify_socket_disconnected(ch);
            return;
        }
        recv_w_ptr[ch] += size;
    } else {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
        char* buf = &recv_buffer[ch][recv_w_ptr[ch]];

        if(size < 8) {
            return;
        }
        if((size = recvfrom(soc[ch], buf + 8, size - 8, 0, (struct sockaddr *)&addr, &len)) == -1) {  // SOCKET_ERROR を -1 に置き換え
            disconnect_socket(ch);
            notify_socket_disconnected(ch);
            return;
        }
        size += 8;
        buf[0] = size >> 8;
        buf[1] = size & 0xFF;
        buf[2] = (char)addr.sin_addr.s_addr;
        buf[3] = (char)(addr.sin_addr.s_addr >> 8);
        buf[4] = (char)(addr.sin_addr.s_addr >> 16);
        buf[5] = (char)(addr.sin_addr.s_addr >> 24);
        buf[6] = (char)addr.sin_port;
        buf[7] = (char)(addr.sin_port >> 8);
        recv_w_ptr[ch] += size;
    }
}

#endif
