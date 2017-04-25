#include <string.h>

#include "module_console.h"
#include "modair_bus.h"
#include "ecan_mod.h"

// use function pointers to navigate through the menu;
// default screen after bootup: home screen
void* (*current_menu_fnc)(u8) = &menu_fnc_homescreen;
volatile u16 module_flags = 0;
s16 idx = 0;

// TODO: make this a constant stored in flash
u16 THIS_MODULE_ID = 0xFF02;
char THIS_MODULE_NAME[] = "Rotax582";

void module_console_process(void)
{
    if (module_flags & 0x8000) {
        //process KEYSTROKE and return updated console text
        current_menu_fnc = (*current_menu_fnc)(module_flags&0xFF);
        module_flags = 0;
    }

    if (module_flags & 0x4000) {
        ecan_tx_str(THIS_MODULE_ID, THIS_MODULE_NAME, MT_BROADCAST_NAME, 8);
        module_flags = 0;
    }
}

void module_console_canrx(u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    // Handle incoming CAN messages; don't send messages from this IRQ
    u8* dptr = (u8*)data;
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_GET_NAME)&&(flags==0)&&(len==3))
        if (data[0]==DPI_ALL_MODULES) // send this module name
            module_flags = 0x4000;

    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_TERMINAL_KEY)&&(flags==0)&&(len==4))
        if (data[0]==THIS_MODULE_ID)
            module_flags = 0x8000 | dptr[3];
}

void* menu_fnc_homescreen(u8 key_input)
{
    switch(key_input) {
        case TK_ROT_INC: idx++; if (idx>3) idx=3; break;
        case TK_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case TK_ROT_PUSH:
            switch(idx) {
                case 0: ecan_tx_console(THIS_MODULE_ID, 0); return &menu_fnc_homescreen; break;
                case 1: return menu_fnc_version(0x00); break;
                case 2: break;
                case 3: break;
            }
            break;
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "Exit");
    strcopy(&rtxt[1+1*16], "Version");
    strcopy(&rtxt[1+2*16], "Change ID");
    strcopy(&rtxt[1+3*16], "Change Name");
    rtxt[(idx&0x03)*16] = '>';
    ecan_tx_console(THIS_MODULE_ID, rtxt);
    return &menu_fnc_homescreen;
}

void* menu_fnc_version(u8 key_input)
{
    // first process key input
    switch(key_input) {
        case TK_ROT_PUSH:
            return menu_fnc_homescreen(0x00);
            break;
    }
    // then print updated console text
    ecan_tx_console(THIS_MODULE_ID,
            "HW v1.0(mod)    "
            "FW v0.1         "
            "                "
            "                ");
    return &menu_fnc_version;
}