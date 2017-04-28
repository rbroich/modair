#ifndef FUELFLOW_H
#define FUELFLOW_H

#include "common.h"

#define FF_IRQcn                _CNIEA2
#define FF_IRQt                 _TRISA2
#define FF_IRQ                  _RA2

void fuelflow_cn_irq(void);
void fuelflow_init(void);
u16 fuelflow_read(u8 inst);

#endif
