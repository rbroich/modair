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
    
    pid_vals_cnt = 21;
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
    pid_vals[17].pid = 0x0042; // GPS HDG 
    pid_vals[18].pid = 0x0040; // GPS VEL 
    pid_vals[19].pid = 0x0041; // GPS ALT 
    pid_vals[20].pid = 0x0043; // TIME    
}

void draw_compass(u8 x, u8 y, int hdg)
{
    #define circ_dots 31.0
    #define circ_tags 25.0
    u8 i;
    u8 k;
    float rot_rad;
    float sin_r, cos_r;

    float northarg = ((360-hdg)-90) / 57.29578;
    s8 northx = 0.5+17.0*cos(northarg);
    s8 northy = 0.5+17.0*sin(northarg);
    LCD_line(x+northx,y+northy,x-northx,y-northy,LCD_BLACK);
    LCD_rect(x-9,y-4,x+9,y+4,LCD_WHITE,1); // erase block

    lcd_setpixel(x, y-20, LCD_BLACK); // heading arrow
    lcd_setpixel(x-1, y-19, LCD_BLACK);
    lcd_setpixel(x, y-19, LCD_BLACK);
    lcd_setpixel(x+1, y-19, LCD_BLACK);

    for (i=0;i<36;i++) {
        rot_rad = ((360-hdg)-90+10*i) / 57.29578;
        sin_r = sin(rot_rad);
        cos_r = cos(rot_rad);

        LCD_dot(x+0.5+circ_dots*cos_r, y+0.5+circ_dots*sin_r, 0, LCD_BLACK);

        if ((i%3)==0)
        switch(i) {
        case 0: {
            u8 pos_x = 0.5+x+circ_tags*cos_r;
            u8 pos_y = 0.5+y+circ_tags*sin_r;
            LCD_char('N', pos_x-2, pos_y-3, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK);
            lcd_setpixel(pos_x+1, pos_y, LCD_BLACK); // make 'N' bold
            lcd_setpixel(pos_x, pos_y-1, LCD_BLACK);
            lcd_setpixel(pos_x-1, pos_y-2, LCD_BLACK);
            lcd_setpixel(pos_x-1, pos_y, LCD_BLACK);
            lcd_setpixel(pos_x, pos_y+1, LCD_BLACK);
            lcd_setpixel(pos_x+1, pos_y+2, LCD_BLACK);
            for (k=0;k<7;k++) {
                lcd_setpixel(pos_x-3, pos_y-3+k, LCD_BLACK);
                lcd_setpixel(pos_x+3, pos_y-3+k, LCD_BLACK);
            }
            break;
            }
        case 9: LCD_char('E', x+0.5+circ_tags*cos_r-2, y+0.5+circ_tags*sin_r-3, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK); break;
        case 18: LCD_char('S', x+0.5+circ_tags*cos_r-2, y+0.5+circ_tags*sin_r-3, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK); break;
        case 27: LCD_char('W', x+0.5+circ_tags*cos_r-2, y+0.5+circ_tags*sin_r-3, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK); break;
        default: {
            char buf[3];
            mprint_int(buf,i,10,2);
            LCD_string(buf, x+0.5+circ_tags*cos_r-3, y+0.5+circ_tags*sin_r-2, GLCD_ROTATE_0, GLCD_FONT_4x5, LCD_BLACK, 3);
            break;
            }
        }
    }

    char buf_brg[5];
    mprint_int(buf_brg,hdg,10,3); // 090
    LCD_string(buf_brg, x-8, y-3, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 5);
}

void draw_hbar(u8 x, u8 y, u8 w, u8 h, u8 percent)
{
    LCD_rect(x,y,x+w-1,y+h-1,LCD_BLACK,0); // draw outline
    u8 fill = percent * w / 100.0;
    if (fill >= w) fill = w-1;
    LCD_rect(x,y+1,x+fill,y+h-2,LCD_BLACK,1); // fill bar
}

void draw_widgets(u8 home_screen)
{
    char tmp_str[16];
    char *ptr;

    if (home_screen == 0) {
        // compass with GPS heading
        draw_compass(31,31,pid_vals[17].val);

        // fuel level: text
        LCD_string((char*)"Fuel ", 0, 117, font_sml2);
        ptr = mprint_float(tmp_str, pid_vals[7].val, 2, 2); // 48.63l
        strcpy(ptr,"l");
        LCD_string(tmp_str, LCD_X-6*4, 117, font_sml2);

        // fuel level: bottom hbar
        draw_hbar(0,123,64,5,pid_vals[7].val*100.0/50.0);

        // bus voltage: bottom right
        ptr = mprint_float(tmp_str, pid_vals[0].val, 2, 1); // xx.xV
        strcpy(ptr,"V");
        LCD_string(tmp_str, 0, 111, font_sml2);

        // h20 temp: bottom left (1 line up)
        ptr = mprint_float(tmp_str, pid_vals[8].val, 2, 1); // xx.x
        strcpy(ptr," C");
        LCD_string(tmp_str, LCD_X-24, 110, font_sml2);
        lcd_setpixel(LCD_X-6,110,LCD_BLACK); // degrees dot

        // Altitude
        ptr = mprint_float(tmp_str, pid_vals[19].val, 4, 0); // 5560 ft
        strcpy(ptr," ft");
        LCD_string(tmp_str, 32-3.5*6, 65, font_def);

        // Velocity
        ptr = mprint_float(tmp_str, pid_vals[18].val, 2, 0); // 54 mph
        strcpy(ptr," mph");
        LCD_string(tmp_str, 32-3*6, 65+9, font_def);

        // RPM
        ptr = mprint_float(tmp_str, pid_vals[3].val, 4, 0); // 2240 rpm
        strcpy(ptr," RPM");
        LCD_string(tmp_str, 31-4*4, 88, font_sml1);
        LCD_line(4,95,LCD_X-1-4,95,LCD_BLACK);
        u8 rpm_ratio = pid_vals[3].val*(LCD_X-8)/8000.0;
        LCD_line(4+rpm_ratio,97,4+rpm_ratio,94,LCD_BLACK);
        LCD_line(3+rpm_ratio,97,3+rpm_ratio,94,LCD_BLACK);

        // engine on time: bottom left (2 lines up)
        ptr = mprint_float(tmp_str, pid_vals[5].val*60.0, 3, 0); // 012m
        strcpy(ptr,"m");
        LCD_string(tmp_str, 0, 104, font_sml2);

        // Time in UTC
        u8 *fl_ptr;
        fl_ptr = (u8*)&pid_vals[20].val;
        ptr = mprint_int(tmp_str, fl_ptr[2], 10, 2); // 06:54
        *ptr++ = ':';
        mprint_int(ptr, fl_ptr[1], 10, 2);
        LCD_string(tmp_str, LCD_X-5*4, 104, GLCD_ROTATE_0,GLCD_FONT_3x5,LCD_BLACK,5);

    } else {
        
    }
}