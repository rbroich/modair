#ifndef MENU_FUNCTIONS_H
#define MENU_FUNCTIONS_H

#include "common.h"

void* menu_fnc_homescreen(u8 key_input);
    void* menu_fnc_main(u8 key_input);
        void* menu_fnc_viewnotes(u8 key_input);
    void* menu_fnc_config(u8 key_input);
        void* menu_fnc_editsensors(u8 key_input);
        void* menu_fnc_edithome(u8 key_input);
        void* menu_fnc_editmenu(u8 key_input);
        void* menu_fnc_editnotes(u8 key_input);
        void* menu_fnc_debugbus(u8 key_input);


#endif
