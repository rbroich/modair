#ifndef MODULE_CONSOLE_H
#define MODULE_CONSOLE_H

#include "common.h"

void module_console_process(void);
void module_console_canrx(u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);

void* menu_fnc_homescreen(u8 key_input);
    void* menu_fnc_version(u8 key_input);

#endif
