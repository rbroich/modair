#ifndef RPM_H
#define RPM_H

#include "common.h"

#define RPM_IRQcn                _CNIEB14
#define RPM_IRQt                 _TRISB14
#define RPM_IRQ                  _RB14

void rpm_cn_irq(void);
void rpm_tmr_irq(void);
void rpm_sendval(u8 idx);
void rpm_init(void);

void enginehours_vinlost_irq(void);
void engineon_sendval(u8 idx);
void enginehours_sendval(u8 idx);
void* enginehours_menu(u8 idx, u8 key_input);

#endif
