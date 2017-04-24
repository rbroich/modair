#ifndef PARAMS_H
#define PARAMS_H

#include "common.h"

typedef struct {
    u16 pid;
    char name[8];
    u8 rate;
    void* console_fnc_ptr;
} s_param_settings;

#endif
