#ifndef PARAMS_H
#define PARAMS_H

#include "common.h"
#include "fuellevel.h"
#include "watertemp.h"

#define PARAM_CNT      14 // index 0: module parameters

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

typedef struct {
    s_param_settings param[PARAM_CNT];
    s_watertemp watertemp_rom;
    s_fuelcal fuellevel_rom;
} s_settings;

#endif
