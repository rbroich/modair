#include <string.h>

#include "iopins.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"

extern const s_settings settings;
extern const s_param_fptr PARAM_CONST[PARAM_CNT];

void iopins_init(void)
{
    LED1t = OUTPUT_PIN;
    LED1 = 0;
    
    RELAY1t = OUTPUT_PIN;
    RELAY1 = 0;
    
    OPENDRAIN1t = OUTPUT_PIN;
    OPENDRAIN1 = 0;
    OPENDRAIN2t = OUTPUT_PIN;
    OPENDRAIN2 = 0;
}

void led_toggle(void)
{
    LED1 = !LED1;
}

void iopins_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8* dptr = (u8*)data;
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_SET_VALUE)&&(flags==0)&&(len==7)) {
        if (data[0]==settings.param[idx].pid) { // dest PID matches
            float *new_value = (float*)&data[2];
            if (PARAM_CONST[idx].sendval_fnc_ptr==&iopins_sendrelay) {
                if (*new_value==0.0)
                    RELAY1 = 0;
                else RELAY1 = 1;
                return;
            }
            if (PARAM_CONST[idx].sendval_fnc_ptr==&iopins_sendod1) {
                if (*new_value==0.0)
                    OPENDRAIN1 = 0;
                else OPENDRAIN1 = 1;
                return;
            }
            if (PARAM_CONST[idx].sendval_fnc_ptr==&iopins_sendod2) {
                if (*new_value==0.0)
                    OPENDRAIN2 = 0;
                else OPENDRAIN2 = 1;
                return;
            }
        }
    }
}

void iopins_sendrelay(u8 idx)
{
    float tx_val = RELAY1? 1.0 : 0.0;
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, tx_val); // Send value
}
void iopins_sendod1(u8 idx)
{
    float tx_val = OPENDRAIN1? 1.0 : 0.0;
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, tx_val);
}
void iopins_sendod2(u8 idx)
{
    float tx_val = OPENDRAIN2? 1.0 : 0.0;
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, tx_val);
}

void* iopins_relay_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: RELAY1 = 1; break;
        case KP_ROT_DEC: RELAY1 = 0; break;
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    iopins_printmenu(idx, RELAY1);
    return &iopins_relay_menu;
}
void* iopins_od1_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: OPENDRAIN1 = 1; break;
        case KP_ROT_DEC: OPENDRAIN1 = 0; break;
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    iopins_printmenu(idx, OPENDRAIN1);
    return &iopins_od1_menu;
}
void* iopins_od2_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: OPENDRAIN2 = 1; break;
        case KP_ROT_DEC: OPENDRAIN2 = 0; break;
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    iopins_printmenu(idx, OPENDRAIN2);
    return &iopins_od2_menu;
}

void iopins_printmenu(u8 idx, u8 state)
{
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    memcpy(rtxt, settings.param[idx].name, 8);
    strcopy(&rtxt[1+1*16], "State:");
    if (state)
        strcopy(&rtxt[1*16+8], "ON");
    else
        strcopy(&rtxt[1*16+8], "OFF");
    ecan_tx_console(settings.param[idx].pid, rtxt);
}