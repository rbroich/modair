#include "fuelflow.h"

u16 ff_accum = 0;
u16 ff_inst = 0;

void fuelflow_cn_irq(void)
{
    static u8 old_val = 1;
    u16 curr_val = FF_IRQ;
    if (curr_val ^ old_val) {
        ff_accum++;
        ff_inst++;
    }
    old_val = FF_IRQ;
}

void fuelflow_init(void)
{
    FF_IRQt = INPUT_PIN;
    FF_IRQcn = ENABLE;
}

u16 fuelflow_read(u8 inst)
{
    if (inst != 0)
        return ff_accum;
    else {
        u16 tmp = ff_inst;
        ff_inst = 0;
        return tmp;
    }
}
