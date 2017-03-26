#include "rpm.h"

u16 rpm_inst = 0;

void rpm_irq(void)
{
    static u8 old_val = 1;
    u16 curr_val = RPM_IRQ;
    if (curr_val ^ old_val) {
        rpm_inst++;
    }
    old_val = RPM_IRQ;
}

void rpm_init(void)
{
    RPM_IRQt = INPUT_PIN;
    RPM_IRQcn = ENABLE;
}

u16 rpm_read(void)
{
    return rpm_inst;
}
