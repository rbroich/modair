#include "busvoltage.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];

void busvoltage_cntdwn(u8 idx)
{
    u16 x = analog_read_inputvoltage();
    float vin = (float)x * (3.3/4095.0 * (22.0+1.5)/1.5); // 12-bit adc, 22k and 1.5k voltage divider
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, vin); // Send value
}

void* busvoltage_fnc_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "ADC Value:");
    u16 x = analog_read_inputvoltage();
    rtxt[2+1*16] = '0';
    rtxt[3+1*16] = 'x';
    mprint_int(&rtxt[4+1*16], x, 16, 4);
    
    float vin = (float)x * (3.3/4095.0 * (22.0+1.5)/1.5); // 12-bit adc, 22k and 1.5k voltage divider
    mprint_float(&rtxt[2+2*16], vin, 0, 2);
    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &busvoltage_fnc_homescreen;
}

