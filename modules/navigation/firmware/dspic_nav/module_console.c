#include <string.h>

#include "common.h"
#include "module_console.h"
#include "modair_bus.h"
#include "ecan_mod.h"
#include "params.h"

extern const s_param_fptr PARAM_CONST[PARAM_CNT];
extern const s_settings settings;
extern s_settings tmp_settings;
s16 idx = 0;
u16 jdx;
char tmp_str[8];

void* menu_fnc_homescreen(u8 zero, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: idx++; break;
        case KP_ROT_DEC: idx--; break;
        case KP_ROT_PUSH:
            switch(idx) {
                case 0: idx=0; return 0;
                case 1: idx=0; flash_tmp_settings(); return 0;
                case 2: idx=0; return menu_fnc_version(0,0x00); // call function to print updated console text, and then return with that pointer
            }
            // TODO: Handle Param settings
            break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    if (idx>2+PARAM_CNT) idx=2+PARAM_CNT;
    if (idx<0) idx=0;
    int pg = idx/4;
    if (pg==0) {
        strcopy(&rtxt[1+0*16], "Exit & Discard");
        strcopy(&rtxt[1+1*16], "Save Changes");
        strcopy(&rtxt[1+2*16], "Version Info");
        strcopy(&rtxt[1+3*16], "00:");
        memcpy(&rtxt[5+3*16], tmp_settings.param[0].name, 8);
    } else {
        int lineoffs = 0;
        int pg_last = pg*4;
        if (pg_last>=PARAM_CNT-1)
            pg_last=PARAM_CNT-1;
        for (i=pg*4-3; i<=pg_last; i++) {
            mprint_int(&rtxt[1+lineoffs], i, 10, 2);
            rtxt[3+lineoffs] = ':';
            memcpy(&rtxt[5+lineoffs], tmp_settings.param[i].name, 8);
            lineoffs += 16;
        }
    }
    rtxt[(idx&0x3)*16] = '>';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_homescreen;
}

void* menu_fnc_changesettings(u8 zero, u8 key_input) // Change: ID/Name/Rate submenu: discard changes, save changes, item0...N
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: idx++; break;
        case KP_ROT_DEC: idx--; break;
        case KP_ROT_PUSH:
            return 0;
            switch(idx) {
                case 0: idx=0; return 0;
                case 1: idx=0; return 0;
                case 2: idx=0; return 0; //jdx=settings.param[0].pid; return menu_fnc_changeid(0,0x00);
                case 3: idx=0; return 0; //memcpy(tmp_str,settings.param[0].name,8); return menu_fnc_changename(0,0x00);
            }
            break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    if (jdx<0) jdx=0;
    if (jdx>PARAM_CNT-1) jdx=PARAM_CNT-1;
    if (jdx==0) {
        strcopy(&rtxt[1+0*16], "MODULE");
    } else {
        strcopy(&rtxt[1+0*16], "PARAM");
        mprint_int(&rtxt[6], jdx, 10, 2);
    }
    strcopy(&rtxt[1+1*16], "Change ID");
    strcopy(&rtxt[1+2*16], "Change Name"); // Change: ID/Name/Rate submenu: discard changes, save changes, item0...N
    strcopy(&rtxt[1+3*16], "Change Rate");
    if (idx>3) idx=3;
    if (idx<0) idx=0;
    rtxt[idx*16] = '>';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_homescreen;
}

void* menu_fnc_version(u8 zero, u8 key_input)
{
    switch(key_input) {
        case KP_ROT_PUSH:
            return menu_fnc_homescreen(0,0x00);
            break;
    }
    ecan_tx_console(settings.param[0].pid,
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
        case KP_ROT_HOLD_DEC: idx++; if (idx>1) idx=1; break;
        case KP_ROT_HOLD_INC: idx--; if (idx<0) idx=0; break;
        case KP_ROT_INC:
            switch (idx) {
                case 0: jdx+=1; break;
                case 1: jdx+=16; break;
            }
            break;
        case KP_ROT_DEC:
            switch (idx) {
                case 0: jdx-=1; break;
                case 1: jdx-=16; break;
            }
            break;
        case KP_ROT_PUSH:
            // TODO
            //PARAM_LIST[0].pid = jdx; // apply new ID
            return 0;
        case KP_ROT_HOLD:
            idx=0; return menu_fnc_homescreen(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1], "Change ID from");
    mprint_int(&rtxt[2+1*16], settings.param[0].pid, 16, 4);
    rtxt[7+1*16] = 't';
    rtxt[8+1*16] = 'o';
    mprint_int(&rtxt[10+1*16], jdx, 16, 4);
    rtxt[13-idx+2*16] = '^';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changeid;
}

void* menu_fnc_changename(u8 zero, u8 key_input)
{
    switch(key_input) {
        case KP_ROT_HOLD_INC: idx++; if (idx>7) idx=7; break;
        case KP_ROT_HOLD_DEC: idx--; if (idx<0) idx=0; break;
        case KP_ROT_INC: tmp_str[idx]++; if (tmp_str[idx]>'z') tmp_str[idx]='z'; break;
        case KP_ROT_DEC: tmp_str[idx]--; if (tmp_str[idx]<' ') tmp_str[idx]=' '; break;
        case KP_ROT_PUSH:
            idx++; if (idx<=7) break; 
            // TODO
            //memcpy(PARAM_LIST[0].name,tmp_str,8); // apply new name
            // intentionally no break
        case KP_ROT_HOLD:
            idx=0; return menu_fnc_homescreen(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[0], "Change      Name");
    mprint_int(&rtxt[7], settings.param[0].pid, 16, 4);
    rtxt[3+1*16] = 0x22;
    rtxt[12+1*16] = 0x22;
    memcpy(&rtxt[4+1*16],tmp_str,8);
    rtxt[4+idx+2*16] = '^';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changename;
}
