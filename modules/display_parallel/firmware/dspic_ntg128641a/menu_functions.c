#include "menu_functions.h"
#include "glcd_lib.h"
#include "rot_enc.h"

s16 idx = 0;

void* menu_fnc_homescreen(u8 key_input)
{
    void *ret = &menu_fnc_homescreen;

    // draw to screen
    LCD_rect(0, 0, LCD_X-1, LCD_Y-1, LCD_BLACK, 0);
    LCD_string("ModAir Home Screen", 10+idx, 10, font_def);

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>40) idx=40; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH: ret = &menu_fnc_main; idx=0; break;
        case C_ROT_HOLD: ret = &menu_fnc_config; idx=0; break;
    }
    return ret;
}

void* menu_fnc_main(u8 key_input)
{
    void *ret = &menu_fnc_main;
    LCD_string("MAIN MENU", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);
    
    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>1) idx=1; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            switch(idx) {
                case 0: ret = &menu_fnc_homescreen; break;
                case 1: ret = &menu_fnc_viewnotes; break;
            }
            idx=0;
            break;
    }

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_string("Notes", 10, 5+8*2, font_def);
    LCD_dot(3, 13+idx*8, 3, LCD_BLACK);
    
    return ret;
}

void* menu_fnc_config(u8 key_input)
{
    void *ret = &menu_fnc_config;
    LCD_string("CONFIG MENU", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>5) idx=5; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            switch(idx) {
                case 0: ret = &menu_fnc_homescreen; break;
                case 1: ret = &menu_fnc_editsensors; break;
                case 2: ret = &menu_fnc_edithome; break;
                case 3: ret = &menu_fnc_editmenu; break;
                case 4: ret = &menu_fnc_editnotes; break;
                case 5: ret = &menu_fnc_debugbus; break;
            }
            idx=0;
            break;
    }

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_string("Sensor Settings", 10, 5+8*2, font_def);
    LCD_string("Home Screen Setup", 10, 5+8*3, font_def);
    LCD_string("Main Menu Setup", 10, 5+8*4, font_def);
    LCD_string("Notes Editor", 10, 5+8*5, font_def);
    LCD_string("Debug CAN Bus", 10, 5+8*6, font_def);
    LCD_dot(3, 13+idx*8, 3, LCD_BLACK);

    return ret;
}

void* menu_fnc_viewnotes(u8 key_input)
{
    void *ret = &menu_fnc_viewnotes;
    LCD_string("NOTE VIEWER", 2, 3, font_def);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_main;
            idx=0;
            break;
    }

    return ret;
}

void* menu_fnc_editsensors(u8 key_input)
{
    void *ret = &menu_fnc_editsensors;
    LCD_string("SENSOR SETTINGS", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_config;
            idx=0;
            break;
    }
    return ret;
}

void* menu_fnc_edithome(u8 key_input)
{
    void *ret = &menu_fnc_edithome;
    LCD_string("HOME SCREEN SETUP", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_config;
            idx=0;
            break;
    }
    return ret;
}

void* menu_fnc_editmenu(u8 key_input)
{
    void *ret = &menu_fnc_editmenu;
    LCD_string("MAIN MENU SETUP", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_config;
            idx=0;
            break;
    }
    return ret;
}

void* menu_fnc_editnotes(u8 key_input)
{
    void *ret = &menu_fnc_editnotes;
    LCD_string("EDIT NOTES", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_config;
            idx=0;
            break;
    }
    return ret;
}

void* menu_fnc_debugbus(u8 key_input)
{
    void *ret = &menu_fnc_debugbus;
    LCD_string("DEBUG CAN BUS", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_config;
            idx=0;
            break;
    }
    return ret;
}
