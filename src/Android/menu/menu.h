//
// Created by medamap on 2024/04/02.
//

#ifndef ANDROIDSTUDIO_MENU_H
#define ANDROIDSTUDIO_MENU_H

#include "BaseMenu.h"

class Menu : public BaseMenu {
public:
    Menu();
};

#define MENU_FDD0       0x0000
#define MENU_FDD1       0x0001
#define MENU_FDD2       0x0002
#define MENU_FDD3       0x0003
#define MENU_FDD4       0x0004
#define MENU_FDD5       0x0005
#define MENU_FDD6       0x0006
#define MENU_FDD7       0x0007

#define MENU_TAPE0      0x0100
#define MENU_TAPE1      0x0101

#define MENU_CART0      0x0200
#define MENU_CART1      0x0201
#define MENU_CART2      0x0202
#define MENU_CART3      0x0203
#define MENU_CART4      0x0204
#define MENU_CART5      0x0205
#define MENU_CART6      0x0206
#define MENU_CART7      0x0207

#define MENU_QD0        0x0300
#define MENU_QD1        0x0301
#define MENU_QD2        0x0302
#define MENU_QD3        0x0303

#define MENU_HDD0       0x0400
#define MENU_HDD1       0x0401
#define MENU_HDD2       0x0402
#define MENU_HDD3       0x0403
#define MENU_HDD4       0x0404
#define MENU_HDD5       0x0405
#define MENU_HDD6       0x0406
#define MENU_HDD7       0x0407

#define MENU_CD0        0x0500
#define MENU_CD1        0x0501
#define MENU_CD2        0x0502
#define MENU_CD3        0x0503

#define MENU_BUBBLE0    0x0600
#define MENU_BUBBLE1    0x0601
#define MENU_BUBBLE2    0x0602
#define MENU_BUBBLE3    0x0603

#define MENU_BIN0       0x0700
#define MENU_BIN1       0x0701
#define MENU_BIN2       0x0702
#define MENU_BIN3       0x0703

#endif //ANDROIDSTUDIO_MENU_H
