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
