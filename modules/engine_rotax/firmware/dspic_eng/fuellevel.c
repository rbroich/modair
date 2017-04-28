#include "fuellevel.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const s_fuelcal fuellevel_rom;

void fuellevel_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8* dptr = (u8*)data;
    if ((msg_type==MT_REMOTE_CMD)&&(data[0]==PARAM_LIST[idx].pid)) {
        if (flags==0) {
            // Name request
            if ((dptr[2]==MC_GET_NAME)&&(len==3))
                ecan_tx_str(PARAM_LIST[idx].pid, (char*)PARAM_LIST[idx].name, MT_BROADCAST_NAME, 8);
            // Value request
            if ((dptr[2]==MC_GET_VALUE)&&(len==3))
                fuellevel_cntdwn(idx);
            // Terminal keystroke
            if ((dptr[2]==MC_TERMINAL_KEY)&&(len==4))
                fuellevel_fnc_homescreen(idx, dptr[3]);
                // current_menu_fnc = (*current_menu_fnc)(dptr[3]); // can remote console function
        }
    }
}

void fuellevel_cntdwn(u8 idx)
{
    u8 i;
    u16 x = analog_read_fuellevel();
    u16 y = 0;
    for (i=1;i<16;i++) // 16 LUT
    if (x <= fuellevel_rom.FLx[i]) {
        y = linear_interpolate(x, fuellevel_rom.FLx[i-1], fuellevel_rom.FLx[i],
                fuellevel_rom.FLy[i-1], fuellevel_rom.FLy[i]);
        break;
    }

    float tmp_f = (float)y*0.01; // convert to liters
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}

void fuellevel_fnc_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case TK_ROT_PUSH: // send Console EXIT
            ecan_tx_console(PARAM_LIST[idx].pid, 0);
            return;
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
    if (x <= fuellevel_rom.FLx[i]) {
        y = linear_interpolate(x, fuellevel_rom.FLx[i-1], fuellevel_rom.FLx[i],
                fuellevel_rom.FLy[i-1], fuellevel_rom.FLy[i]);
        break;
    }
    mprint_float(&rtxt[2+2*16], (float)y*0.01, 0, 2);
    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
}

