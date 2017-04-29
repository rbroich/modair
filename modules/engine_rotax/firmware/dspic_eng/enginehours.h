#ifndef ONLOGGING_H
#define ONLOGGING_H

#include "common.h"

void enginehours_tmr_irq(void);
void enginehours_vinlost_irq(void);

void enginehours_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);
void enginehours_cntdwn(u8 idx);
void enginehours_init(void);

void* enginehours_fnc_homescreen(u8 idx, u8 key_input);

#endif
