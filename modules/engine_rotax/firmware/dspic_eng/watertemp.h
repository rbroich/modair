#ifndef WATERTEMP_H
#define WATERTEMP_H

#include "common.h"

typedef struct {
    u16 WTx[16];
    s16 WTy[16];
} s_watertemp;

void watertemp_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);
void watertemp_cntdwn(u8 idx);

#endif
