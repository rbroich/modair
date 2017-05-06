#ifndef HEAP_H
#define HEAP_H

#include "common.h"

#define HEAP_MEM_SIZE           1024

#define HEAP_ALLOC_NONE         0
#define HEAP_ALLOC_PIDNAME      1
#define HEAP_ALLOC_CANDEBUG     2
#define HEAP_ALLOC_CONSOLETXT   3
#define HEAP_ALLOC_HOME         4

typedef struct {
    u16 pid;
    float val;
} s_pid_val;

typedef struct {
    u16 pid;
    union {
        u16 nval[4];
        char name[8];
    } u;
} s_pid_name;
#define MAX_PID_NAME_ITEMS    (HEAP_MEM_SIZE / sizeof(s_pid_name))

typedef struct {
    u16 pid;
    u16 d0;
    u16 d2;
    u16 d4;
    u16 d6;
    u16 msg_type;
    u16 len;
    u16 flags;
} s_can_debug;

typedef struct {
    u16 pid;
    char txt[16*4];
} s_console_txt;

#endif
