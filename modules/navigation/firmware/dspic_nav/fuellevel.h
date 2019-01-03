#ifndef FUELLEVEL_H
#define FUELLEVEL_H

#include "common.h"

typedef struct {
    u16 FLx[16];
    s16 FLy[16];
} s_fuelcal;

void fuellevel_cntdwn(u8 idx);
float fuellevel_interpolate(u16 adc_val);

void* fuellevel_menu(u8 idx, u8 key_input);
    void* fuellevel_info(u8 idx, u8 key_input);
    void* fuellevel_edit(u8 idx, u8 key_input);
        void* fuellevel_entry(u8 idx, u8 key_input);

#endif
