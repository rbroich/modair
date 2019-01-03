#include <string.h>
#include "busvoltage.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_settings settings;

void busvoltage_cntdwn(u8 idx)
{
    u16 x = analog_read_inputvoltage();
    float vin = (float)x * (3.3/4095.0 * (22.0+1.5)/1.5); // 12-bit adc, 22k and 1.5k voltage divider
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, vin); // Send value
}

void* busvoltage_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=8;i<16*4;i++) rtxt[i] = ' ';
    memcpy(rtxt, settings.param[idx].name, 8);
    strcopy(&rtxt[1+1*16], "ADC:");
    u16 x = analog_read_inputvoltage();
    rtxt[6+1*16] = '0';
    rtxt[7+1*16] = 'x';
    mprint_int(&rtxt[8+1*16], x, 16, 4);
    
    float vin = (float)x * (3.3/4095.0 * (22.0+1.5)/1.5); // 12-bit adc, 22k and 1.5k voltage divider
    char* end_ptr = mprint_float(&rtxt[1+2*16], vin, 0, 2);
    end_ptr++; *end_ptr = 'V';
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &busvoltage_menu;
}

