#include "watertemp.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const s_watertemp watertemp_rom;

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
