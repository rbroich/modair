#include "rot_enc.h"

volatile u8 rotpb_wait = 0;
volatile u8 rotpb_cnt = 0;
volatile u8 rot_flags = 0;

void rot_enc_irq(void)
{
    static u16 PORTB_old = 0xFFFF;
    u16 PORTB_val = PORTB;
    if ((PORTB_val ^ PORTB_old)&0b1100) // encoder moved
    {
        static s8 pos_cnt = 0;
        if (((PORTB_old&0b0100)>>2) ^ ((PORTB_val&0b1000)>>3)){ // XOR bit old.ENC_A and new.ENC_B
            pos_cnt++;
        } else {
            pos_cnt--;
        }
        if ((PORTB_val&0b1100)==0b1100) // detent position
        { // pos_cnt should have moved by 4
            if (PORTB_val&0b0010) // pushbutton not held
            {
                if (pos_cnt > 2) // rotate left
                    rot_flags |= rot_dec_flag;
                if (pos_cnt < -2) // rotate right
                    rot_flags |= rot_inc_flag;
            } else { // pushbutton held down
                rotpb_wait = 0;
                if (pos_cnt > 2) // rotate left
                    rot_flags |= rot_shift_dec_flag;
                if (pos_cnt < -2) // rotate right
                    rot_flags |= rot_shift_inc_flag;
            }
            pos_cnt = 0;
            if (rot_flags & (rot_dec_flag|rot_inc_flag|rot_shift_dec_flag|rot_shift_inc_flag))
                rot_flags |= rot_mod_flag;
        }
    }
    if ((PORTB_val ^ PORTB_old)&0b0010) // push button changed
    {
        if (PORTB_val&0b0010) { // push button released
            rotpb_cnt = 0;
            if (rotpb_wait) {
                rotpb_wait = 0;
                rot_flags |= rot_push_flag;
                rot_flags |= rot_mod_flag;
            }
        } else { // push button engaged
            rotpb_cnt = 60; // start timer: 60 x 20ms = 1.2 sec for rot_longpush_flag
            rotpb_wait = 1;
        }
    }
    PORTB_old = PORTB_val;
}

void rot_enc_tmr(void) // called every 20 ms == 50 Hz
{
    if (rotpb_cnt) {
        rotpb_cnt--;
        if ((rotpb_cnt==0)&&(rotpb_wait)) {
            rotpb_wait = 0;
            rot_flags |= rot_longpush_flag;
            rot_flags |= rot_mod_flag;
        }
    }
}

void rot_enc_init(void)
{
    PB0a = DIGITAL_PIN;
    ROT0a = DIGITAL_PIN;
    ROT1a = DIGITAL_PIN;
    
    PB0t = INPUT_PIN;
    ROT0t = INPUT_PIN;
    ROT1t = INPUT_PIN;

    PB0pu = ENABLE; // pull up
    ROT0pu = ENABLE;
    ROT1pu = ENABLE;

    PB0c = ENABLE; // change notification
    ROT0c = ENABLE;
    ROT1c = ENABLE;
}
