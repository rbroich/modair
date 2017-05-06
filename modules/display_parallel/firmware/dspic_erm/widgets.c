#include <string.h>
#include <math.h>

#include "widgets.h"
#include "rtspapi.h"
#include "heap.h"
#include "glcd_lib.h"

//==============================================================================
//--------------------STORED WIDGET CONFIG IN FLASH MEMORY----------------------
//==============================================================================
__attribute__((aligned(FLASH_PAGE_SIZE))) const widget rWIDGETS[FLASH_PAGE_SIZE/sizeof(widget)] = {
    {.page=0, .posX=15, .posY=20, .type=WT_FIXED_TEXT},
};
//==============================================================================
//--------------------VARIABLES-------------------------------------------------
//==============================================================================
extern volatile s_pid_val pid_vals[64];
extern volatile u16 pid_vals_cnt;

__attribute__((far))widget widgets[FLASH_PAGE_SIZE/sizeof(widget)];
u8 nr_home_screens;
u8 widgets_count;

//==============================================================================
//==============================================================================
void read_widgets(void)
{
    u8 i;
    nr_home_screens = 0;
    widgets_count = 0;
    FlashPageRead(__builtin_tblpage(rWIDGETS), __builtin_tbloffset(rWIDGETS), (s16*)&widgets, sizeof(widgets)/sizeof(u16));
    for (i=0;i<FLASH_PAGE_SIZE/sizeof(widget);i++)
    {
        if (widgets[i].page > nr_home_screens)
            nr_home_screens = widgets[i].page;
        if (widgets[i].type == WT_NONE) {
            widgets_count = i;
            break;
        }
    }

    pid_vals_cnt = 5;
    pid_vals[0].pid  = 0x0040; // GPS VEL 
    pid_vals[1].pid  = 0x0041; // GPS ALT 
    pid_vals[2].pid  = 0x0042; // GPS HDG 
    pid_vals[3].pid  = 0x0043; // TIME    
    pid_vals[4].pid  = 0x0044; // DATE    
}

void draw_widgets(u8 home_screen)
{
    char tmp_str[16];
    char *ptr;

    // Altitude
    ptr = mprint_float(tmp_str, pid_vals[1].val, 4, 0); // 5560 ft
    strcpy(ptr," ft");
    LCD_string(tmp_str, 120, 10, font_def);
    LCD_rect(255-6,0,255,63,LCD_BLACK,0);
    float alt_delta = (pid_vals[1].val - 4000.0) / 100.0;
    u8 alt_graph = alt_delta; // min=4000, max=10300 (delta 6300 ft)
    alt_graph = alt_graph&63;
    LCD_rect(255-5,63-alt_graph,254,63,LCD_BLACK,1);

    // Velocity
    ptr = mprint_float(tmp_str, pid_vals[0].val, 2, 0); // 54 mph
    strcpy(ptr," mph");
    LCD_string(tmp_str, 123, 30, font_def);

    // Heading
    mprint_float(tmp_str, pid_vals[2].val, 3, 0); // 240 deg
    LCD_string(tmp_str, 23-6, 23, font_def);
    LCD_circle(23,23,23,LCD_BLACK);
    LCD_char('N',23,4,GLCD_ROTATE_0,GLCD_FONT_4x5,LCD_BLACK);
    LCD_char('E',23+20,23,GLCD_ROTATE_0,GLCD_FONT_3x5,LCD_BLACK);
    LCD_char('S',23,23+23-4,GLCD_ROTATE_0,GLCD_FONT_3x5,LCD_BLACK);
    LCD_char('W',3,23,GLCD_ROTATE_0,GLCD_FONT_3x5,LCD_BLACK);
    float trigarg = (float)(pid_vals[2].val)*(3.14/180.0);
    LCD_dot((23+(s8)(16*sin(trigarg))),(23+(s8)(-16*cos(trigarg))),3,LCD_BLACK);
    
    // North pointer
    float northarg = (float)(360.0 - pid_vals[2].val)*(3.14/180.0);
    s8 northx = 28.0*sin(northarg);
    s8 northy = -28.0*cos(northarg);
    LCD_dot(208+northx,31+northy,2,LCD_BLACK);
    LCD_line(208+northx,31+northy,208-northx,31-northy,LCD_BLACK);
    LCD_rect(208-16-3,31-4,208+16+3,31+4,LCD_WHITE,1); // erase block
    LCD_string("North", 208-16+3, 31, font_def);

    // Time in hh:mm:ss
    u8 *fl_ptr;
    fl_ptr = (u8*)&pid_vals[3].val;
    mprint_int(&tmp_str[0], fl_ptr[2], 10, 2); // hrs
    tmp_str[2] = ':';
    mprint_int(&tmp_str[3], fl_ptr[1], 10, 2); // min
    tmp_str[5] = ':';
    mprint_int(&tmp_str[6], fl_ptr[0], 10, 2); // sec
    LCD_string(tmp_str, 2, 60-8, font_def);

    // Date in dd/mm/20yy
    fl_ptr = (u8*)&pid_vals[4].val;
    mprint_int(&tmp_str[0], fl_ptr[0], 10, 2); // dd
    tmp_str[2] = '/';
    mprint_int(&tmp_str[3], fl_ptr[1], 10, 2); // mm
    strcpy(&tmp_str[5],"/20");
    mprint_int(&tmp_str[8], fl_ptr[2], 10, 2); // yr
    LCD_string(tmp_str, 2, 60, font_def);
}