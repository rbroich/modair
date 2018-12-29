#include "fuellevel.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_settings settings;

void fuellevel_cntdwn(u8 idx)
{
    u8 i;
    u16 x = analog_read_fuellevel();
    u16 y = 0;
    for (i=1;i<16;i++) // 16 LUT
    if (x <= settings.fuellevel_rom.FLx[i]) {
        y = linear_interpolate(x, settings.fuellevel_rom.FLx[i-1], settings.fuellevel_rom.FLx[i],
                settings.fuellevel_rom.FLy[i-1], settings.fuellevel_rom.FLy[i]);
        break;
    }

    float tmp_f = (float)y*0.01; // convert to liters
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}

void* fuellevel_fnc_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    u16 y = 0;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "ADC Value:");
    u16 x = analog_read_fuellevel();
    rtxt[2+1*16] = '0';
    rtxt[3+1*16] = 'x';
    mprint_int(&rtxt[4+1*16], x, 16, 4);
    
    for (i=1;i<16;i++) // 16 LUT
    if (x <= settings.fuellevel_rom.FLx[i]) {
        y = linear_interpolate(x, settings.fuellevel_rom.FLx[i-1], settings.fuellevel_rom.FLx[i],
                settings.fuellevel_rom.FLy[i-1], settings.fuellevel_rom.FLy[i]);
        break;
    }
    mprint_float(&rtxt[2+2*16], (float)y*0.01, 0, 2);
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &fuellevel_fnc_homescreen;
}

