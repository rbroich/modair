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
u16 *pidptr;
s16 *rateptr;
char *nameptr;

void* menu_fnc_homescreen(u8 zero, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: idx++; break;
        case KP_ROT_DEC: idx--; break;
        case KP_ROT_PUSH:
            switch(idx) {
                case 0:
                    idx=0;
                    return 0;
                case 1:
                    idx=0;
                    u16 last_module_pid=settings.param[0].pid;
                    flash_tmp_settings();
                    if (last_module_pid != tmp_settings.param[0].pid)
                        ecan_tx_console(last_module_pid, 0); // close session on the old PID
                    return 0;
                case 2:
                    idx=0;
                    return menu_fnc_version(0,0x00); // call function to print updated console text, and then return with that pointer
            }
            jdx=idx-3; idx=0; return menu_fnc_changesettings(0,0x00);
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
            switch(idx) {
                case 0: idx=0; return menu_fnc_homescreen(0,0x00);
                case 1: idx=0; pidptr=&tmp_settings.param[jdx].pid; return menu_fnc_changeid(0,0x00);
                case 2: idx=0; nameptr=tmp_settings.param[jdx].name; return menu_fnc_changename(0,0x00);
                case 3: idx=0; rateptr=(s16*)&tmp_settings.param[jdx].rate; return menu_fnc_changerate(0,0x00);
            }
            break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    mprint_int(&rtxt[1], tmp_settings.param[jdx].pid, 16, 4);
    rtxt[6] = '-';
    memcpy(&rtxt[8], tmp_settings.param[jdx].name, 8);
    strcopy(&rtxt[1+1*16], "Change ID");
    strcopy(&rtxt[1+2*16], "Change Name");
    strcopy(&rtxt[1+3*16], "Change Rate");
    if (idx>3) idx=3;
    if (idx<0) idx=0;
    rtxt[idx*16] = '>';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changesettings;
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
        case KP_ROT_HOLD_DEC: idx++; if (idx>3) idx=3; break;
        case KP_ROT_HOLD_INC: idx--; if (idx<0) idx=0; break;
        case KP_ROT_INC:
            switch (idx) {
                case 0: *pidptr+=0x1; break;
                case 1: *pidptr+=0x10; break;
                case 2: *pidptr+=0x100; break;
                case 3: *pidptr+=0x1000; break;
            }
            break;
        case KP_ROT_DEC:
            switch (idx) {
                case 0: *pidptr-=0x1; break;
                case 1: *pidptr-=0x10; break;
                case 2: *pidptr-=0x100; break;
                case 3: *pidptr-=0x1000; break;
            }
            break;
        case KP_ROT_PUSH:
        case KP_ROT_HOLD:
            idx=0; return menu_fnc_changesettings(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1], "Change ID from");
    mprint_int(&rtxt[2+1*16], settings.param[jdx].pid, 16, 4);
    rtxt[7+1*16] = 't';
    rtxt[8+1*16] = 'o';
    mprint_int(&rtxt[10+1*16], *pidptr, 16, 4);
    rtxt[13-idx+2*16] = '^';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changeid;
}

void* menu_fnc_changename(u8 zero, u8 key_input)
{
    switch(key_input) {
        case KP_ROT_HOLD_INC: idx++; if (idx>7) idx=7; break;
        case KP_ROT_HOLD_DEC: idx--; if (idx<0) idx=0; break;
        case KP_ROT_INC: nameptr[idx]++; if (nameptr[idx]>'z') nameptr[idx]='z'; break;
        case KP_ROT_DEC: nameptr[idx]--; if (nameptr[idx]<' ') nameptr[idx]=' '; break;
        case KP_ROT_PUSH:
            idx++; if (idx<=7) break; 
            // else: intentionally no break
        case KP_ROT_HOLD:
            idx=0; return menu_fnc_changesettings(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[0], "Change      Name");
    mprint_int(&rtxt[7], tmp_settings.param[jdx].pid, 16, 4);
    rtxt[3+1*16] = 0x22;
    rtxt[12+1*16] = 0x22;
    memcpy(&rtxt[4+1*16],nameptr,8);
    rtxt[4+idx+2*16] = '^';
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changename;
}

void* menu_fnc_changerate(u8 zero, u8 key_input)
{
    switch(key_input) {
        case KP_ROT_HOLD_DEC: idx++; if (idx>2) idx=2; break;
        case KP_ROT_HOLD_INC: idx--; if (idx<0) idx=0; break;
        case KP_ROT_INC:
            switch (idx) {
                case 0: *rateptr+=1; break;
                case 1: *rateptr+=10; break;
                case 2: *rateptr+=100; break;
            }
            break;
        case KP_ROT_DEC:
            switch (idx) {
                case 0: *rateptr-=1; break;
                case 1: *rateptr-=10; break;
                case 2: *rateptr-=100; break;
            }
            break;
        case KP_ROT_PUSH:
        case KP_ROT_HOLD:
            idx=0; return menu_fnc_changesettings(0,0x00);
    }
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(rtxt, "Change Rate from");
    mprint_int(&rtxt[2+1*16], settings.param[jdx].rate, 10, 4);
    rtxt[7+1*16] = 't';
    rtxt[8+1*16] = 'o';
    if (*rateptr < 0) *rateptr=0;
    if (*rateptr > 9999) *rateptr=9999;
    mprint_int(&rtxt[10+1*16], *rateptr, 10, 4);
    rtxt[13-idx+2*16] = '^';
    float rate_s = 0.02 * (float)(*rateptr); // 20ms period
    mprint_float(&rtxt[0+3*16], rate_s, 2, 2);
    rtxt[5+3*16] = 's';
    if (*rateptr) {
        float rate_hz = 1.0 / rate_s;
        mprint_float(&rtxt[8+3*16], rate_hz, 1, 3);
        strcopy(&rtxt[13+3*16], " Hz");
    } else {
        strcopy(&rtxt[13+3*16], "OFF");
    }
    ecan_tx_console(settings.param[0].pid, rtxt);
    return &menu_fnc_changerate;
}
