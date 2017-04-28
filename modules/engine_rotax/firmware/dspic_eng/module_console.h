#ifndef MODULE_CONSOLE_H
#define MODULE_CONSOLE_H

#include "common.h"

void module_init(void);
void module_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);

void* menu_fnc_homescreen(u8 key_input);
    void* menu_fnc_version(u8 key_input);
    void* menu_fnc_changeid(u8 key_input);
    void* menu_fnc_changename(u8 key_input);

#endif
