#ifndef RPM_H
#define RPM_H

#include "common.h"

#define RPM_IRQcn                _CNIEB14
#define RPM_IRQt                 _TRISB14
#define RPM_IRQ                  _RB14

void rpm_irq(void);
void rpm_init(void);
u16 rpm_read(void);

#endif
