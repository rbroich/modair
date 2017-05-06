#include <string.h>

#include "menu_functions.h"
#include "glcd_lib.h"
#include "rot_enc.h"
#include "buzzer.h"
#include "widgets.h"
#include "modair_bus.h"
#include "ecan_mod.h"
#include "heap.h"


// TODO: make this a constant stored in flash
#define THIS_MODULE_ID 0xFF02

s16 idx = 0;
u8 tmp_cnt = 0;
extern volatile u16 heap_mem[HEAP_MEM_SIZE];
extern volatile u8 heap_item_cnt;
extern volatile u8 heap_alloc;

void* menu_fnc_homescreen(u8 key_input)
{
    void *ret = &menu_fnc_homescreen;

    heap_alloc = HEAP_ALLOC_HOME;
    // draw to screen
    draw_widgets(idx);

    if (tmp_cnt) {
        tmp_cnt--;
        if (idx)
            LCD_rect(LCD_X/2, LCD_Y-1, LCD_X-1, LCD_Y-1, LCD_BLACK, 0);
        else LCD_rect(0, LCD_Y-1, LCD_X/2-1, LCD_Y-1, LCD_BLACK, 0);
    }

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>1) idx=1; tmp_cnt=15; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; tmp_cnt=15; break;
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

    heap_alloc = HEAP_ALLOC_NONE; // deallocate heap memory
    heap_item_cnt = 0;
    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>2) idx=2; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            switch(idx) {
                case 0: ret = &menu_fnc_homescreen; break;
                case 1: ret = &menu_fnc_viewmodules; break;
                case 2: ret = &menu_fnc_viewnotes; break;
            }
            idx=0;
            break;
    }

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_string("View Modules", 10, 5+8*2, font_def);
    LCD_string("Notes", 10, 5+8*3, font_def);
    LCD_dot(3, 13+idx*8, 3, LCD_BLACK);
    
    return ret;
}

void* menu_fnc_viewmodules(u8 key_input)
{
    void *ret = &menu_fnc_viewmodules;
    LCD_string("VIEW ATTACHED MODULES", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_string("Module params", 10, 5+8*2, font_def);
    LCD_string("Module console", 10, 5+8*3, font_def);
    LCD_string("All params", 10, 5+8*4, font_def);

    heap_alloc = HEAP_ALLOC_NONE; // deallocate heap memory
    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>3) idx=3; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            switch (idx) {
                case 0: ret = &menu_fnc_main; break;
                case 1: ret = &menu_fnc_viewmoduleparams;
                    heap_alloc = HEAP_ALLOC_PIDNAME;
                    ecan_tx(THIS_MODULE_ID, DPI_ALL_MODULES, RC_GET_NAME, 0, 0, MT_REMOTE_CMD, 0, 3); // send a GET_NAME request to all modules
                    break;
                case 2: ret = &menu_fnc_accessmoduleconsole;
                    heap_alloc = HEAP_ALLOC_PIDNAME;
                    ecan_tx(THIS_MODULE_ID, DPI_ALL_MODULES, RC_GET_NAME, 0, 0, MT_REMOTE_CMD, 0, 3); // send a GET_NAME request to all modules
                    break;
                case 3: ret = &menu_fnc_viewparams;
                    heap_alloc = HEAP_ALLOC_PIDNAME;
                    ecan_tx(THIS_MODULE_ID, DPI_ALL_PARAMETERS, RC_GET_NAME, 0, 0, MT_REMOTE_CMD, 0, 3); // send a GET_NAME request to all parameters
                    break;
            }
            heap_item_cnt = 0; // reset current list counter
            idx=0;
            break;
    }
    LCD_dot(3, 13+(idx%7)*8, 3, LCD_BLACK);

    return ret;
}

void* menu_fnc_viewmoduleparams(u8 key_input)
{
    void *ret = &menu_fnc_viewmoduleparams;
    LCD_string("VIEW MODULE PARAMS", 2, 3, font_def);

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>heap_item_cnt) idx=heap_item_cnt; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            if (idx==0) {
                ret = &menu_fnc_viewmodules;
            } else {
                ret = &menu_fnc_viewparams;
                // send a GET_NAME request to the selected module
                s_pid_name* pid_names = (s_pid_name*)&heap_mem[0];
                ecan_tx(THIS_MODULE_ID, pid_names[idx-1].pid, RC_GET_NAME, 0, 0, MT_REMOTE_CMD, 0, 3);
                heap_item_cnt = 0; // reset current list counter
            }
            idx=0;
            break;
    }
    print_pid_names();
    return ret;
}

void* menu_fnc_accessmoduleconsole(u8 key_input)
{
    void *ret = &menu_fnc_accessmoduleconsole;
    LCD_string("ACCESS MODULE CONSOLE", 2, 3, font_def);

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>heap_item_cnt) idx=heap_item_cnt; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            if (idx==0) {
                ret = &menu_fnc_viewmodules;
            } else {
                ret = &menu_fnc_accessconsole;
                s_pid_name* pid_names = (s_pid_name*)&heap_mem[0];
                s_console_txt* console_txt = (s_console_txt*)&heap_mem[0];
                console_txt->pid = pid_names[idx-1].pid;
                memset((char*)console_txt->txt, ' ', 16*4);
                // send a TERMINAL_KEY request to the selected module (with KEYSTROKE==0x00) to bring up the menu
                ecan_tx(THIS_MODULE_ID, console_txt->pid, RC_CONSOLE_KEY | (0x00<<8), 0, 0, MT_REMOTE_CMD, 0, 4);
                heap_item_cnt = 0; // reset current list counter
                heap_alloc = HEAP_ALLOC_CONSOLETXT; // allocate heap memory
            }
            idx=0;
            break;
    }
    print_pid_names();
    return ret;
}

void* menu_fnc_accessconsole(u8 key_input)
{
    s_console_txt* console_txt = (s_console_txt*)&heap_mem[0];
    LCD_string("CONSOLE", 2, 3, font_def);
    if (heap_alloc != HEAP_ALLOC_CONSOLETXT)
        return 0;
    char tmp_str[16];
    mprint_int(tmp_str, console_txt->pid, 16, 4);
    LCD_string(tmp_str, 10+7*6, 3, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,4);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);
    LCD_rect(6, 11, 8+16*6, 13+4*8, LCD_BLACK, 0);

    if (console_txt->txt[0]) {
        // print the remote console
        LCD_string(&console_txt->txt[0], 10, 8+8*1, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,16);
        LCD_string(&console_txt->txt[16], 10, 8+8*2, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,16);
        LCD_string(&console_txt->txt[32], 10, 8+8*3, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,16);
        LCD_string(&console_txt->txt[48], 10, 8+8*4, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,16);
    } else return &menu_fnc_homescreen; // exit to home screen

    if (key_input) { // send the KEYCODE to the selected module
        ecan_tx(THIS_MODULE_ID, console_txt->pid, RC_CONSOLE_KEY | (key_input<<8), 0, 0, MT_REMOTE_CMD, 0, 4);
    }
    return &menu_fnc_accessconsole;
}

void* menu_fnc_viewparams(u8 key_input)
{
    void *ret = &menu_fnc_viewparams;
    LCD_string("VIEW PARAMETERS", 2, 3, font_def);

    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>heap_item_cnt) idx=heap_item_cnt; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            if (idx==0) {
                ret = &menu_fnc_viewmodules;
            } else {
                ret = &menu_fnc_accessconsole;
                s_pid_name* pid_names = (s_pid_name*)&heap_mem[0];
                s_console_txt* console_txt = (s_console_txt*)&heap_mem[0];
                console_txt->pid = pid_names[idx-1].pid;
                memset((char*)console_txt->txt, ' ', 16*4);
                // send a TERMINAL_KEY request to the selected parameter (with KEYSTROKE==0x00) to bring up the menu
                ecan_tx(THIS_MODULE_ID, console_txt->pid, RC_CONSOLE_KEY | (0x00<<8), 0, 0, MT_REMOTE_CMD, 0, 4);
                heap_item_cnt = 0; // reset current list counter
                heap_alloc = HEAP_ALLOC_CONSOLETXT; // allocate heap memory
            }
            idx=0;
            break;
    }
    print_pid_names();
    return ret;
}

void print_pid_names(void)
{
    s_pid_name* pid_names = (s_pid_name*)&heap_mem[0];
    u8 i=0, j=1;
    char tmp_str[16];
    u8 page = idx / 7;
    if (page==0) {
        LCD_string("Exit", 10, 3+8*1, font_def);
    } else { i=page*7-1; j=0; }
    for (; (i<heap_item_cnt)&&(j<7); i++, j++) {
        LCD_string((char*)&pid_names[i].u.name[0], 10, 3+8+8*j, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,8);
        LCD_char('(', 10+14*6, 3+8+8*j, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK);
        mprint_int(tmp_str, pid_names[i].pid, 16, 4);
        LCD_string(tmp_str, 10+15*6, 3+8+8*j, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,4);
        LCD_char(')', 10+19*6, 3+8+8*j, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK);
    }
    LCD_dot(3, 11+(idx%7)*8, 3, LCD_BLACK);
}

void* menu_fnc_config(u8 key_input)
{
    void *ret = &menu_fnc_config;
    LCD_string("SETTINGS", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_string("Home Setup", 10, 5+8*2, font_def);
    LCD_string("Menu Setup", 10, 5+8*3, font_def);
    LCD_string("Notes Editor", 10, 5+8*4, font_def);
    LCD_string("Debug Bus", 10, 5+8*5, font_def);
    LCD_dot(3, 13+idx*8, 3, LCD_BLACK);

    heap_alloc = HEAP_ALLOC_NONE; // deallocate heap
    switch(key_input) {
        case C_ROT_INC: idx++; if (idx>4) idx=4; break;
        case C_ROT_DEC: idx--; if (idx<0) idx=0; break;
        case C_ROT_PUSH:
            switch(idx) {
                case 0: ret = &menu_fnc_homescreen; break;
                case 1: ret = &menu_fnc_edithome; break;
                case 2: ret = &menu_fnc_editmenu; break;
                case 3: ret = &menu_fnc_editnotes; break;
                case 4: ret = &menu_fnc_debugbus;
                    heap_alloc = HEAP_ALLOC_CANDEBUG;
                    heap_item_cnt = 0;
                    break;
            }
            idx=0;
            break;
    }
    return ret;
}

void* menu_fnc_viewnotes(u8 key_input)
{
    void *ret = &menu_fnc_viewnotes;
    LCD_string("VIEW NOTES", 2, 3, font_def);
    LCD_line(0, 8, LCD_X-1, 8, LCD_BLACK);

    switch(key_input) {
        case C_ROT_PUSH:
            ret = &menu_fnc_main;
            idx=0;
            break;
    }

    LCD_string("Exit", 10, 5+8*1, font_def);
    LCD_dot(3, 13+idx*8, 3, LCD_BLACK);

    return ret;
}

void* menu_fnc_edithome(u8 key_input)
{
    void *ret = &menu_fnc_edithome;
    LCD_string("HOME SETUP", 2, 3, font_def);
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
    LCD_string("MENU SETUP", 2, 3, font_def);
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

    buzzer_toggle();

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
    u8 i,j;
    char tmp_str[20];
    s_can_debug* can_dbgs = (s_can_debug*)&heap_mem[0];
    for (i=0;i<heap_item_cnt;i++) {
        mprint_int(tmp_str, can_dbgs[i].pid, 16, 4);
        LCD_string(tmp_str, 2, 3+8*i, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,4);
        LCD_char('/', 2+4*6, 3+8*i, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK);
        mprint_int(tmp_str, can_dbgs[i].msg_type, 16, 2);
        LCD_string(tmp_str, 2+5*6, 3+8*i, GLCD_ROTATE_0,GLCD_FONT_5x7,LCD_BLACK,2);

        mprint_int(&tmp_str[0], can_dbgs[i].d0, 16, 4);
        mprint_int(&tmp_str[4], can_dbgs[i].d2, 16, 4);
        mprint_int(&tmp_str[8], can_dbgs[i].d4, 16, 4);
        mprint_int(&tmp_str[12], can_dbgs[i].d6, 16, 4);
        for (j=0;j<can_dbgs[i].len;j++) {
            LCD_string(&tmp_str[j*2], 2+7*6+j*9, 3+8*i, GLCD_ROTATE_0,GLCD_FONT_4x5,LCD_BLACK,2);
        }
        lcd_setpixel(119,2+8*i,LCD_BLACK);
        lcd_setpixel(119,3+8*i,LCD_BLACK);
        lcd_setpixel(119,4+8*i,LCD_BLACK);
        if (!(can_dbgs[i].flags & 0x1))
            lcd_setpixel(120,3+8*i,LCD_BLACK);
        if (!(can_dbgs[i].flags & 0x2))
            lcd_setpixel(121,3+8*i,LCD_BLACK);
        lcd_setpixel(122,2+8*i,LCD_BLACK);
        lcd_setpixel(122,3+8*i,LCD_BLACK);
        lcd_setpixel(122,4+8*i,LCD_BLACK);
        if (can_dbgs[i].flags & ECAN_FLAGS_RTR)
            lcd_setpixel(125,3+8*i,LCD_BLACK);
    }
    if ((idx)&&(heap_item_cnt==8)) // auto-reset
        heap_item_cnt = 0;
    switch(key_input) {
        case C_ROT_INC:
            idx=1;
            break;
        case C_ROT_DEC:
            idx=0;
            heap_item_cnt = 0;
            break;
        case C_ROT_PUSH:
            idx=0;
            return &menu_fnc_config;
            break;
    }
    return &menu_fnc_debugbus;
}
