#include <string.h>

#include "module_console.h"
#include "modair_bus.h"
#include "ecan_mod.h"
#include "params.h"

extern const s_param_const PARAM_CONST[PARAM_CNT];
extern const s_param_settings PARAM_LIST[PARAM_CNT];
s16 idx = 0;
u16 jdx;
char tmp_str[8];

void* menu_fnc_homescreen(u8 zero, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case TK_ROT_INC: idx++; if (idx>3) idx=3; break;
        case TK_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case TK_ROT_PUSH:
            switch(idx) {
                case 0: return 0;
                case 1: return menu_fnc_version(0,0x00);
                case 2: idx=0;jdx=PARAM_LIST[0].pid; return menu_fnc_changeid(0,0x00);
                case 3: idx=0;memcpy(tmp_str,PARAM_LIST[0].name,8); return menu_fnc_changename(0,0x00);
            }
            break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "Exit");
    strcopy(&rtxt[1+1*16], "Version");
    strcopy(&rtxt[1+2*16], "Change ID");
    strcopy(&rtxt[1+3*16], "Change Name"); // Change: ID/Name/Rate submenu: discard changes, save changes, item0...N
    rtxt[(idx&0x03)*16] = '>';
    ecan_tx_console(PARAM_LIST[0].pid, rtxt);
    return &menu_fnc_homescreen;
}

void* menu_fnc_version(u8 zero, u8 key_input)
{
    switch(key_input) {
        case TK_ROT_PUSH:
            return menu_fnc_homescreen(0,0x00);
            break;
    }
    ecan_tx_console(PARAM_LIST[0].pid,
        "HW v1.0(mod)    "
        "FW v0.1         "
        "                "
        "                "
    );
    return &menu_fnc_version;
}

void* menu_fnc_changeid(u8 zero, u8 key_input)
{
    switch(key_input) {
        case TK_ROT_HOLD_DEC: idx++; if (idx>1) idx=1; break;
        case TK_ROT_HOLD_INC: idx--; if (idx<0) idx=0; break;
        case TK_ROT_INC:
            switch (idx) {
                case 0: jdx+=1; break;
                case 1: jdx+=16; break;
            }
            break;
        case TK_ROT_DEC:
            switch (idx) {
                case 0: jdx-=1; break;
                case 1: jdx-=16; break;
            }
            break;
        case TK_ROT_PUSH:
            // TODO
            //PARAM_LIST[0].pid = jdx; // apply new ID
            return 0;
        case TK_ROT_HOLD:
            idx=0; return menu_fnc_homescreen(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1], "Change ID from");
    mprint_int(&rtxt[2+1*16], PARAM_LIST[0].pid, 16, 4);
    rtxt[7+1*16] = 't';
    rtxt[8+1*16] = 'o';
    mprint_int(&rtxt[10+1*16], jdx, 16, 4);
    rtxt[13-idx+2*16] = '^';
    ecan_tx_console(PARAM_LIST[0].pid, rtxt);
    return &menu_fnc_changeid;
}

void* menu_fnc_changename(u8 zero, u8 key_input)
{
    switch(key_input) {
        case TK_ROT_HOLD_INC: idx++; if (idx>7) idx=7; break;
        case TK_ROT_HOLD_DEC: idx--; if (idx<0) idx=0; break;
        case TK_ROT_INC: tmp_str[idx]++; if (tmp_str[idx]>'z') tmp_str[idx]='z'; break;
        case TK_ROT_DEC: tmp_str[idx]--; if (tmp_str[idx]<' ') tmp_str[idx]=' '; break;
        case TK_ROT_PUSH:
            idx++; if (idx<=7) break; 
            // TODO
            //memcpy(PARAM_LIST[0].name,tmp_str,8); // apply new name
            // intentionally no break
        case TK_ROT_HOLD:
            idx=0; return menu_fnc_homescreen(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[0], "Change      Name");
    mprint_int(&rtxt[7], PARAM_LIST[0].pid, 16, 4);
    rtxt[3+1*16] = 0x22;
    rtxt[12+1*16] = 0x22;
    memcpy(&rtxt[4+1*16],tmp_str,8);
    rtxt[4+idx+2*16] = '^';
    ecan_tx_console(PARAM_LIST[0].pid, rtxt);
    return &menu_fnc_changename;
}
