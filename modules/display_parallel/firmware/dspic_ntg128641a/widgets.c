#include "widgets.h"
#include "rtspapi.h"

//==============================================================================
//--------------------STORED WIDGET CONFIG IN FLASH MEMORY----------------------
//==============================================================================
__attribute__((aligned(FLASH_PAGE_SIZE))) const widget rWIDGETS[FLASH_PAGE_SIZE/sizeof(widget)] = {
    {.page=0, .posX=15, .posY=20, .type=WT_FIXED_TEXT},
};
//==============================================================================
//--------------------VARIABLES-------------------------------------------------
//==============================================================================
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
}

