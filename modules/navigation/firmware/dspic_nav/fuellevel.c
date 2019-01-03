#include <string.h>

#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_settings settings;
extern s_settings tmp_settings;

s8 fl_ptr;
u8 fl_entrymode;
s16 fl_idx = 0;

void fuellevel_cntdwn(u8 idx)
{
    u16 x = analog_read_fuellevel();
    float tmp_f = fuellevel_interpolate(x);
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}

float fuellevel_interpolate(u16 adc_val)
{
    u8 i;
    u16 y = 0;
    for (i=1;i<16;i++) // 16 LUT
    if (adc_val <= settings.fuellevel_rom.FLx[i]) {
        y = linear_interpolate(adc_val, settings.fuellevel_rom.FLx[i-1], settings.fuellevel_rom.FLx[i],
                settings.fuellevel_rom.FLy[i-1], settings.fuellevel_rom.FLy[i]);
        break;
    }
    return (float)y*0.01; // convert to liters
}

void* fuellevel_menu(u8 idx, u8 key_input)
{
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';

    // first process key input
    switch(key_input) {
        case KP_ROT_INC: fl_idx++; if (fl_idx>3) fl_idx=3; break;
        case KP_ROT_DEC: fl_idx--; if (fl_idx<0) fl_idx=0; break;
        case KP_ROT_PUSH:
            switch(fl_idx) {
                case 0: fl_idx=0; return 0;
                case 1: fl_idx=0; return fuellevel_info(idx,0x00);
                case 2: fl_idx=0; flash_tmp_settings(); return 0;
                case 3: fl_idx=0; return fuellevel_edit(idx,0x00);
            }
    }
    // then print updated console text
    strcopy(&rtxt[1+0*16], "Exit & Discard");
    strcopy(&rtxt[1+1*16], "Status Info");
    strcopy(&rtxt[1+2*16], "Save Changes");
    strcopy(&rtxt[1+3*16], "Edit Cal Table");
    rtxt[(fl_idx&0x3)*16] = '>';
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &fuellevel_menu;
}

void* fuellevel_info(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return fuellevel_menu(idx,0x00);
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=8;i<16*4;i++) rtxt[i] = ' ';
    memcpy(rtxt, settings.param[idx].name, 8);
    strcopy(&rtxt[1+1*16], "ADC:");
    u16 x = analog_read_fuellevel();
    rtxt[6+1*16] = '0';
    rtxt[7+1*16] = 'x';
    mprint_int(&rtxt[8+1*16], x, 16, 4);
    
    float tmp_f = fuellevel_interpolate(x);
    strcopy(&rtxt[1+2*16], "Value:");
    mprint_float(&rtxt[8+2*16], tmp_f, 0, 2); // can actually be any unit (litres/gallons/etc)
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &fuellevel_info;
}

void* fuellevel_edit(u8 idx, u8 key_input)
{
    int i;
    switch(key_input) {
        case KP_ROT_INC: fl_idx++; if (fl_idx>17) fl_idx=17; break;
        case KP_ROT_DEC: fl_idx--; if (fl_idx<0) fl_idx=0; break;
        case KP_ROT_PUSH:
            switch(fl_idx) {
                case 0: return fuellevel_menu(idx,0x00);
                case 1:
                    for (i=0;i<16;i++) {
                        tmp_settings.fuellevel_rom.FLx[i] = 0xFFF; // max value
                        tmp_settings.fuellevel_rom.FLy[i] = 0;
                    }
                    tmp_settings.fuellevel_rom.FLx[0] = 0;
                    break;
                default:
                    fl_idx-=2;
                    fl_ptr=0;
                    fl_entrymode=0;
                    if (tmp_settings.fuellevel_rom.FLx[fl_idx]==0xFFF) {
                        u16 x = analog_read_fuellevel(); // current ADC reading on entry if not set
                        tmp_settings.fuellevel_rom.FLx[fl_idx] = x;
                    }
                    return fuellevel_entry(idx,0x00);
            }
    }
    
    char rtxt[16*4];
    int lineoffs = 0;
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    if (fl_idx<4) {
        strcopy(&rtxt[1+0*16], "Back");
        strcopy(&rtxt[1+1*16], "Clear Cal Table");
        lineoffs = 2*16;
        for (i=0;i<2;i++) { // e.g. " 1-2A3: 1234.4  "
            mprint_int(&rtxt[1+lineoffs], i, 16, 1);
            rtxt[2+lineoffs] = '-';
            mprint_int(&rtxt[3+lineoffs], tmp_settings.fuellevel_rom.FLx[i], 16, 3);
            rtxt[6+lineoffs] = ':';
            mprint_float(&rtxt[8+lineoffs], (float)tmp_settings.fuellevel_rom.FLy[i]*0.01, 0, 2);
            lineoffs += 16;
        }
    } else {
        int fl_pg = fl_idx/4;
        int sta = fl_pg*4 - 2; // start index
        int sto = sta+4; // stop index
        if (sto>16) sto=16;
        for (i=sta;i<sto;i++) { // e.g. " 1-2A3: 1234.4  "
            mprint_int(&rtxt[1+lineoffs], i, 16, 1);
            rtxt[2+lineoffs] = '-';
            mprint_int(&rtxt[3+lineoffs], tmp_settings.fuellevel_rom.FLx[i], 16, 3);
            rtxt[6+lineoffs] = ':';
            mprint_float(&rtxt[8+lineoffs], (float)tmp_settings.fuellevel_rom.FLy[i]*0.01, 0, 2);
            lineoffs += 16;
        }
    }
    rtxt[(fl_idx&0x3)*16] = '>';
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &fuellevel_edit;
}

void* fuellevel_entry(u8 idx, u8 key_input)
{
    s16 FLx = tmp_settings.fuellevel_rom.FLx[fl_idx];
    switch(key_input) {
        case KP_ROT_HOLD_DEC: fl_ptr++; if (fl_ptr>2) fl_ptr=2; break;
        case KP_ROT_HOLD_INC: fl_ptr--; if (fl_ptr<0) fl_ptr=0; break;
        case KP_ROT_INC:
            if (fl_entrymode==0) { // edit FLx
                switch (fl_ptr) {
                    case 0: FLx+=0x1; break;
                    case 1: FLx+=0x10; break;
                    case 2: FLx+=0x100; break;
                }
            } else { // edit FLy
                switch (fl_ptr) {
                    case 0: tmp_settings.fuellevel_rom.FLy[fl_idx]+=1; break;
                    case 1: tmp_settings.fuellevel_rom.FLy[fl_idx]+=10; break;
                    case 2: tmp_settings.fuellevel_rom.FLy[fl_idx]+=100; break;
                }
            }
            break;
        case KP_ROT_DEC:
            if (fl_entrymode==0) { // edit FLx
                switch (fl_ptr) {
                    case 0: FLx-=0x1; break;
                    case 1: FLx-=0x10; break;
                    case 2: FLx-=0x100; break;
                }
            } else { // edit FLy
                switch (fl_ptr) {
                    case 0: tmp_settings.fuellevel_rom.FLy[fl_idx]-=1; break;
                    case 1: tmp_settings.fuellevel_rom.FLy[fl_idx]-=10; break;
                    case 2: tmp_settings.fuellevel_rom.FLy[fl_idx]-=100; break;
                }
            }
            break;
        case KP_ROT_PUSH:
            fl_ptr=0;
            fl_entrymode++;
            if (fl_entrymode<2)
                break;
            // else: no break to return
        case KP_ROT_HOLD: fl_idx += 2; return fuellevel_edit(idx,0x00);
    }

    if (FLx<0) FLx=0; // saturate to 12-bit unsigned
    if (FLx>0xFFF) FLx=0xFFF;
    if (fl_idx) { // limit min-value based on CalTable below
        if (FLx < tmp_settings.fuellevel_rom.FLx[fl_idx-1])
            FLx = tmp_settings.fuellevel_rom.FLx[fl_idx-1];
    }
    if (fl_idx<15) { // limit max-value based on CalTable above
        if (FLx > tmp_settings.fuellevel_rom.FLx[fl_idx+1])
            FLx = tmp_settings.fuellevel_rom.FLx[fl_idx+1];
    }
    tmp_settings.fuellevel_rom.FLx[fl_idx] = FLx;

    u16 x = analog_read_fuellevel();
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(rtxt, "Edit Cal Entry");
    mprint_int(&rtxt[1+1*16], fl_idx, 16, 1);
    rtxt[2+1*16] = '-';
    mprint_int(&rtxt[3+1*16], tmp_settings.fuellevel_rom.FLx[fl_idx], 16, 3);
    rtxt[6+1*16] = ':';
    char *float_ptr = mprint_float(&rtxt[8+1*16], (float)tmp_settings.fuellevel_rom.FLy[fl_idx]*0.01, 0, 2);
    if (fl_entrymode==0)
        rtxt[5-fl_ptr+2*16] = '^';
    else {
        float_ptr = float_ptr+16-fl_ptr-1;
        if (fl_ptr>1) float_ptr--;
        *float_ptr = '^';
    }
    strcopy(&rtxt[3*16], "ADC Sample:");
    mprint_int(&rtxt[12+3*16], x, 16, 3);
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &fuellevel_entry;
}
