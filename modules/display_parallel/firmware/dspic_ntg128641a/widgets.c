#include <string.h>

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
    
    pid_vals_cnt = 17;
    pid_vals[0].pid  = 0x0010; // BUS VOLT
    pid_vals[1].pid  = 0x0011; // EGT 1   
    pid_vals[2].pid  = 0x0012; // EGT 2   
    pid_vals[3].pid  = 0x0013; // RPM     
    pid_vals[4].pid  = 0x0014; // ENG HRS 
    pid_vals[5].pid  = 0x0015; // ENG ON  
    pid_vals[6].pid  = 0x0016; // MAINTAIN
    pid_vals[7].pid  = 0x0017; // FUEL LVL
    pid_vals[8].pid  = 0x0018; // H2O TEMP
    pid_vals[9].pid  = 0x0019; // RELAY   
    pid_vals[10].pid = 0x001A; // OD1 OUT 
    pid_vals[11].pid = 0x001B; // OD2 OUT 
    pid_vals[12].pid = 0x001C; // FF INST 
    pid_vals[13].pid = 0x001D; // FF AVE  
    pid_vals[14].pid = 0x001E; // FUEL END
    pid_vals[15].pid = 0x001F; // FUEL RNG
    pid_vals[16].pid = 0x0020; // FUEL USE
}

void draw_widgets(u8 home_screen)
{
    char tmp_str[16];
    char *ptr;

    if (home_screen == 0) {
        // bus voltage: bottom right
        strcpy(tmp_str,"Bus ");
        ptr = mprint_float(&tmp_str[4], pid_vals[0].val, 2, 1); // xx.x
        strcpy(ptr,"V");
        LCD_string(tmp_str, 3, 2, font_rot);

        // h20 temp: bottom left (1 line up)
        ptr = mprint_float(tmp_str, pid_vals[8].val, 0, 1); // xx.x
        strcpy(ptr," C");
        LCD_string(tmp_str, 3+8, 2, font_rot);

        // engine on time: bottom left (2 lines up)
        ptr = mprint_float(tmp_str, pid_vals[5].val, 1, 3); // 0.012h
        strcpy(ptr,"h");
        LCD_string(tmp_str, 3+2*8, 2, font_rot);

        // rpm: middle centre
        mprint_float(tmp_str, pid_vals[3].val, 4, 0); // 2240 rpm
        LCD_string(tmp_str, 60, 23, font_rot);
        LCD_string("rpm", 60-8, 23+3, font_rot);

        // fuel level: top bar + text
        strcpy(tmp_str,"Fuel ");
        ptr = mprint_float(&tmp_str[5], pid_vals[7].val, 2, 2);
        strcpy(ptr,"l");
        LCD_string(tmp_str, 124, 2, font_rot);
        LCD_rect(119-7,0,119,63,LCD_BLACK,0);
        s16 fuel_level = pid_vals[7].val * 63.0 / 50.0;
        if (fuel_level > 63) fuel_level = 63;
        if (fuel_level < 0) fuel_level = 0;
        LCD_rect(119-6,0,119-1,fuel_level,LCD_BLACK,1); // fill bar
    } else {
        
    }
}