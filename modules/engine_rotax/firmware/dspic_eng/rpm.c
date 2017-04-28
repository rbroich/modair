#include "rpm.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"

u16 rpm_inst = 0;
u16 rpm_cnt = 0;

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const u16 rmp_mul;

void rpm_cn_irq(void)
{
    static u8 old_val = 1;
    u16 curr_val = RPM_IRQ;
    if (curr_val ^ old_val) {
        rpm_cnt++;
    }
    old_val = RPM_IRQ;
}

void rpm_tmr_irq(void)
{
    static u8 tmr_overflow_cnt = 0;
    tmr_overflow_cnt++;
    if (tmr_overflow_cnt >= 500/20) { // count IRQ's over the last 500ms
        tmr_overflow_cnt = 0;
        rpm_inst = rpm_cnt;
        rpm_cnt = 0;
    }
}

void rpm_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8* dptr = (u8*)data;
    if ((msg_type==MT_REMOTE_CMD)&&(data[0]==PARAM_LIST[idx].pid)) {
        if (flags==0) {
            // Name request
            if ((dptr[2]==MC_GET_NAME)&&(len==3))
                ecan_tx_str(PARAM_LIST[idx].pid, (char*)PARAM_LIST[idx].name, MT_BROADCAST_NAME, 8);
            // Value request
            if ((dptr[2]==MC_GET_VALUE)&&(len==3))
                rpm_cntdwn(idx);
            // Terminal keystroke
            if ((dptr[2]==MC_TERMINAL_KEY)&&(len==4))
                ecan_tx_console(PARAM_LIST[idx].pid, 0); // send EXIT
                //rpm_fnc_homescreen(idx, dptr[3]);
                // current_menu_fnc = (*current_menu_fnc)(dptr[3]); // can remote console function
        }
    }
}

void rpm_cntdwn(u8 idx)
{
    float rpm = rpm_inst * rmp_mul;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, rpm); // Send value
}

void rpm_init(void)
{
    RPM_IRQt = INPUT_PIN;
    RPM_IRQcn = ENABLE;
}
