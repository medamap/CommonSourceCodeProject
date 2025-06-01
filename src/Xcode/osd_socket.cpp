/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2015.11.20-

    [ Xcode socket ]

    Author : Medamap and Claude
    Date   : 2024
*/

#include "osd.h"
#include <unistd.h>
#include <sys/socket.h>

#ifdef USE_SOCKET

void OSD::initialize_socket()
{
    // TODO: ソケット初期化
    for(int i = 0; i < SOCKET_MAX; i++) {
        soc[i] = -1;
        socket_delay[i] = 0;
    }
}

void OSD::release_socket()
{
    // TODO: ソケット解放
    for(int i = 0; i < SOCKET_MAX; i++) {
        if(soc[i] != -1) {
            disconnect_socket(i);
        }
    }
}


void OSD::notify_socket_connected(int ch)
{
    // TODO: 接続通知
}

void OSD::notify_socket_disconnected(int ch)
{
    // TODO: 切断通知
}

void OSD::update_socket()
{
    // TODO: ソケット更新
}

bool OSD::initialize_socket_tcp(int ch)
{
    // TODO: TCP初期化
    return false;
}

bool OSD::initialize_socket_udp(int ch)
{
    // TODO: UDP初期化
    return false;
}

bool OSD::connect_socket(int ch, uint32_t ipaddr, int port)
{
    // TODO: ソケット接続
    return false;
}

void OSD::disconnect_socket(int ch)
{
    // TODO: ソケット切断
    if(soc[ch] != -1) {
        close(soc[ch]);
        soc[ch] = -1;
    }
}

bool OSD::listen_socket(int ch)
{
    // TODO: リスン開始
    return false;
}

void OSD::send_socket_data_tcp(int ch)
{
    // TODO: TCPデータ送信
}

void OSD::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
    // TODO: UDPデータ送信
}

void OSD::send_socket_data(int ch)
{
    // TODO: データ送信
}

void OSD::recv_socket_data(int ch)
{
    // TODO: データ受信
}

#endif