#ifndef WATERTEMP_H
#define WATERTEMP_H

#include "common.h"

typedef struct {
    u16 WTx[16];
    s16 WTy[16];
} s_watertemp;

void watertemp_cntdwn(u8 idx);
void* watertemp_menu(u8 idx, u8 key_input);

#endif
