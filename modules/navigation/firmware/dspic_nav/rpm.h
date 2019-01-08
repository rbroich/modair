#ifndef RPM_H
#define RPM_H

#include "common.h"

#define RPM_IRQcn                _CNIEB14
#define RPM_IRQt                 _TRISB14
#define RPM_IRQ                  _RB14

void rpm_cn_irq(void);
void rpm_tmr_irq(void);

void rpm_cntdwn(u8 idx);

void rpm_init(void);

#endif
