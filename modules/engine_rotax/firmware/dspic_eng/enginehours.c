#include "enginehours.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"

u8 enginehours_flash_erased = 0;
volatile u16 engine_on_sec = 0; // on-time in seconds (u16 is enough for 18h)
float curr_engine_hobbs;

extern const s_param_settings PARAM_LIST[PARAM_CNT];
extern const float engine_hobbs;
extern u16 rpm_inst;

void enginehours_tmr_irq(void)
{
    static u16 tmr_cnt = 0;
    if (rpm_inst > 0) {
        tmr_cnt++; //start counting time
        if (tmr_cnt >= 1000/20) { // 1 second
            tmr_cnt = 0;
            engine_on_sec++;
        }
    }
    // Erase page if engine has been on for more than 2 minutes
    // (erase takes 23.1 ms, so we need to do this before vin is lost)
    if ((engine_on_sec > 2*60)&&(!enginehours_flash_erased)) {
        // copy page to RAM
        // write FF's to the Engine Hour variable
        // disable interrupts...
        // erase flash page
        // write RAM to flash
        enginehours_flash_erased = 1;
    }
}

void enginehours_vinlost_irq(void)
{
    // Bus power has been lost;
    // onboard capacitors will only supply the CPU with power for another ~10ms

    if (enginehours_flash_erased) {
        // write Engine Hours to FLASH memory
        // writing a single word takes 54 us
    }
}

void engineon_sendval(u8 idx)
{
    float hrs = (float)engine_on_sec/(60.0*60.0);
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, hrs); // Send value
}

void enginehours_cntdwn(u8 idx)
{
    float hrs = curr_engine_hobbs + (float)engine_on_sec/(60*60);
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, hrs); // Send value
}

void* enginehours_fnc_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "Counter:");
    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &enginehours_fnc_homescreen;
}

void enginehours_init(void)
{
    enginehours_flash_erased = 0;
    curr_engine_hobbs = engine_hobbs; // read Engine Hours from FLASH memory
}
