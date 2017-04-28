#include "watertemp.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const s_watertemp watertemp_rom;

void watertemp_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8* dptr = (u8*)data;
    if ((msg_type==MT_REMOTE_CMD)&&(data[0]==PARAM_LIST[idx].pid)) {
        if (flags==0) {
            // Name request
            if ((dptr[2]==MC_GET_NAME)&&(len==3))
                ecan_tx_str(PARAM_LIST[idx].pid, (char*)PARAM_LIST[idx].name, MT_BROADCAST_NAME, 8);
            // Value request
            if ((dptr[2]==MC_GET_VALUE)&&(len==3))
                watertemp_cntdwn(idx);
            // Terminal keystroke
            if ((dptr[2]==MC_TERMINAL_KEY)&&(len==4))
                ecan_tx_console(PARAM_LIST[idx].pid, 0);
                //busvoltage_fnc_homescreen(idx, dptr[3]);
                // current_menu_fnc = (*current_menu_fnc)(dptr[3]); // can remote console function
        }
    }
}

void watertemp_cntdwn(u8 idx)
{
    u8 i;
    u16 x = analog_read_watertemp();
    u16 y = 0;
    for (i=1;i<16;i++) // 16 LUT
    if (x <= watertemp_rom.WTx[i]) {
        y = linear_interpolate(x, watertemp_rom.WTx[i-1], watertemp_rom.WTx[i],
                watertemp_rom.WTy[i-1], watertemp_rom.WTy[i]);
        break;
    }
    float tmp_f = (float)y * 0.1; // convert 0.1 deg to degrees
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}
