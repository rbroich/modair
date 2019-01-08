#ifndef ONLOGGING_H
#define ONLOGGING_H

#include "common.h"

void enginehours_tmr_irq(void);
void enginehours_vinlost_irq(void);

void engineon_sendval(u8 idx);
void enginehours_cntdwn(u8 idx);
void enginehours_init(void);

void* enginehours_fnc_homescreen(u8 idx, u8 key_input);

#endif
