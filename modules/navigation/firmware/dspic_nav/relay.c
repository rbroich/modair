#include "relay.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"

extern const s_settings settings;

void relay_init(void)
{
    RELAY1t = OUTPUT_PIN;
    RELAY1 = 0;
}

void relay_toggle(void)
{
    RELAY1 = !RELAY1;
}

void* relay_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: RELAY1 = 1; break;
        case KP_ROT_DEC: RELAY1 = 0; break;
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[0*16], "Relay State:");
    if (RELAY1)
        strcopy(&rtxt[1*16+4], "ON");
    else
        strcopy(&rtxt[1*16+4], "OFF");
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &relay_menu;
}