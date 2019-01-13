#include <string.h>
#include <stddef.h>
#include "rpm.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "analog.h"
#include "libpic30.h"

u8 enginehours_flash_erased = 0;
volatile u16 engine_on_sec = 0; // on-time in seconds (u16 is enough for 18h)
float curr_engine_hobbs;

u16 rpm_inst = 0;
u16 rpm_cnt = 0;

extern const s_settings settings;
extern s_settings tmp_settings;

void rpm_cn_irq(void)
{ // TODO: use Input Capture instead of counter in CN IRQ?
    static u8 old_val = 1;
    u16 curr_val = RPM_IRQ;
    if ((curr_val ^ old_val)&&(curr_val)) { // only on rising edge
        rpm_cnt++;
    }
    old_val = RPM_IRQ;
}

void rpm_tmr_irq(void)
{
    static u8 tmr_overflow_cnt = 0;
    tmr_overflow_cnt++;
    if (tmr_overflow_cnt >= 500/20) { // count IRQ's over the last 500ms
        tmr_overflow_cnt = 0;
        rpm_inst = rpm_cnt;
        rpm_cnt = 0;
    }
    
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
        tmp_settings = settings; // copy settings to RAM
        u32* engine_hobbs_ptr = (u32*)&tmp_settings.engine_hobbs;
        *engine_hobbs_ptr = 0xFFFFFFFF; // write FF's to the Engine Hour variable, such that we can program without erasing later
        flash_tmp_settings(); // erase flash memory, and write tmp_settings
        enginehours_flash_erased = 1;
    }
}

void enginehours_vinlost_irq(void) // bus power has been lost
{
    // onboard capacitors will only supply the CPU with power for another ~10ms
    if (enginehours_flash_erased) {
        float hrs = curr_engine_hobbs + (float)engine_on_sec/(60*60);
        u16 *hrs_ptr = (u16*)&hrs;
        // write engine_hobbs to flash memory (previously erased already)
        _prog_addressT p = __builtin_tbladdress(&settings.engine_hobbs);
        _write_flash_word32(p, hrs_ptr[0], hrs_ptr[1]); // writing a single word takes 54 us
        curr_engine_hobbs = hrs;
        engine_on_sec = 0;
        enginehours_flash_erased = 0;
    }
}

void rpm_init(void)
{
    RPM_IRQt = INPUT_PIN;
    RPM_IRQcn = ENABLE;
    
    enginehours_flash_erased = 0;
    curr_engine_hobbs = settings.engine_hobbs; // read Engine Hours from FLASH memory
}

void engineon_sendval(u8 idx)
{
    float hrs = (float)engine_on_sec/(60.0*60.0);
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, hrs); // Send value
}

void enginehours_sendval(u8 idx)
{
    float hrs = curr_engine_hobbs + (float)engine_on_sec/(60*60);
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, hrs); // Send value
}

void rpm_sendval(u8 idx)
{
    float rpm = rpm_inst * settings.rmp_mul;
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, rpm); // Send value
}

void* enginehours_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    char *unit_ptr;
    for (i=8;i<16*4;i++) rtxt[i] = ' ';
    memcpy(rtxt, settings.param[idx].name, 8);
    strcopy(&rtxt[1+1*16], "Hobbs:");
    float hrs = curr_engine_hobbs + (float)engine_on_sec/(60*60);
    unit_ptr = mprint_float(&rtxt[8+1*16], hrs, 0, 2);
    *(++unit_ptr) = 'h';
    strcopy(&rtxt[1+2*16], "On-Time:");
    unit_ptr = mprint_int(&rtxt[10+2*16], engine_on_sec, 10, 0);
    *(++unit_ptr) = 's';
    strcopy(&rtxt[1+3*16], "Flash:");
    unit_ptr = mprint_float(&rtxt[8+3*16], settings.engine_hobbs, 0, 2);
    *(++unit_ptr) = 'h';
    rtxt[15+3*16] = enginehours_flash_erased? '*' : ' ';
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &enginehours_menu;
}
