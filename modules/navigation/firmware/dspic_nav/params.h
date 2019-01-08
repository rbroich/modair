#ifndef PARAMS_H
#define PARAMS_H

#include "common.h"
#include "fuellevel.h"
#include "watertemp.h"

#define PARAM_CNT      25 // index 0: module parameters

typedef struct {
    u16 pid;
    char name[8];
    u16 rate;
} s_param_settings;

typedef struct {
    void (*canrx_fnc_ptr)(u8,u16,u16*,u8,u8,u8);
    void (*sendval_fnc_ptr)(u8);
    void* (*menu_fnc_ptr)(u8,u8);
} s_param_fptr;

typedef union {
    int words[_FLASH_PAGE]; // ensure that sizeof(s_settings)<=_FLASH_PAGE*2
    struct {
        s_param_settings param[PARAM_CNT];
        s_watertemp watertemp_rom;
        s_fuelcal fuellevel_rom;
        u16 rmp_mul;
        float engine_hobbs;
        u8 maintainance_date[3];
        u16 maintainance_date_pid;
    };
} s_settings;

#endif
