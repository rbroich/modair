#include "watertemp.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const s_watertemp watertemp_rom;

void watertemp_cntdwn(u8 idx)
{
    u8 i;
    u16 x = analog_read_watertemp();
    s16 y = 0;
    for (i=1;i<16;i++) // 16 LUT
    if (x <= watertemp_rom.WTx[i]) {
        y = linear_interpolate(x, watertemp_rom.WTx[i-1], watertemp_rom.WTx[i],
                watertemp_rom.WTy[i-1], watertemp_rom.WTy[i]);
        break;
    }
    float tmp_f = (float)y * 0.1; // convert 0.1 deg to degrees
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}

void* watertemp_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    s16 y = 0;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "ADC Value:");
    u16 x = analog_read_watertemp();
    rtxt[2+1*16] = '0';
    rtxt[3+1*16] = 'x';
    mprint_int(&rtxt[4+1*16], x, 16, 4);
    
    for (i=1;i<16;i++) // 16 LUT
    if (x <= watertemp_rom.WTx[i]) {
        y = linear_interpolate(x, watertemp_rom.WTx[i-1], watertemp_rom.WTx[i],
                watertemp_rom.WTy[i-1], watertemp_rom.WTy[i]);
        break;
    }
    mprint_float(&rtxt[2+2*16], (float)y*0.1, 0, 2);
    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &watertemp_menu;
}