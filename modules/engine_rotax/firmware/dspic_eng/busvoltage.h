#ifndef BUSVOLTAGE_H
#define BUSVOLTAGE_H

#include "common.h"

void busvoltage_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);
void busvoltage_cntdwn(u8 idx);

void busvoltage_fnc_homescreen(u8 idx, u8 key_input);

#endif
