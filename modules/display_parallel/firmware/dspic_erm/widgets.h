#ifndef WIDGETS_H
#define WIDGETS_H

#include "common.h"

typedef struct {
    u8 page;
    u8 type;
    u8 posX;
    u8 posY;
    u8 opts[12];
} widget;

#define WT_BLANK           0
#define WT_EMPTY           0
#define WT_NONE            0
#define WT_FIXED_TEXT      1
#define WT_DYNAMIC_TEXT    2
#define WT_DRAW_PRIMITIVE  3
#define WT_BAR             4
#define WT_DIAL            5
#define WT_COMPASS         6

void read_widgets(void);
void draw_widgets(u8 home_screen);

#endif
