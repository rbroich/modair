#ifndef MENU_FUNCTIONS_H
#define MENU_FUNCTIONS_H

#include "common.h"

void* menu_fnc_homescreen(u8 key_input);
    void* menu_fnc_main(u8 key_input);
        void* menu_fnc_viewmodules(u8 key_input);
            void* menu_fnc_viewmoduleparams(u8 key_input);
                void* menu_fnc_viewparams(u8 key_input);
            void* menu_fnc_accessmoduleconsole(u8 key_input);
                void* menu_fnc_accessconsole(u8 key_input);
            //void* menu_fnc_viewparams(u8 key_input);
                //void* menu_fnc_accessconsole(u8 key_input);
        void* menu_fnc_viewnotes(u8 key_input);
    void* menu_fnc_config(u8 key_input);
        void* menu_fnc_edithome(u8 key_input);
        void* menu_fnc_editmenu(u8 key_input);
        void* menu_fnc_editnotes(u8 key_input);
        void* menu_fnc_debugbus(u8 key_input);


void print_pid_names(void);

#endif
