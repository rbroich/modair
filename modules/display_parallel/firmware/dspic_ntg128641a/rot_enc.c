#include "rot_enc.h"

volatile u8 rotpb_cnt = 0;
extern volatile u8 rot_enc_input;

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
                    rot_enc_input = C_ROT_DEC;
                if (pos_cnt < -2) // rotate right
                    rot_enc_input = C_ROT_INC;
            } else { // rotated while pushbutton held down
                rotpb_cnt = 0; // don't trigger C_ROT_HOLD or C_ROT_LONGHOLD
                if (pos_cnt > 2) // rotate left
                    rot_enc_input = C_ROT_HOLD_DEC;
                if (pos_cnt < -2) // rotate right
                    rot_enc_input = C_ROT_HOLD_INC;
            }
            pos_cnt = 0;
        }
    }
    if ((PORTB_val ^ PORTB_old)&0b0010) // push button changed
    {
        if (PORTB_val&0b0010) { // push button released
            if (rotpb_cnt > (255-60)) // button held less than 1.2 sec for PUSH
                rot_enc_input = C_ROT_PUSH;
            rotpb_cnt = 0; // stop countdown in timer function
        } else { // push button engaged
            rotpb_cnt = 255; // start countdown in timer function
        }
    }
    PORTB_old = PORTB_val;
}

void rot_enc_tmr(void) // called every 20 ms == 50 Hz
{
    if (rotpb_cnt) {
        rotpb_cnt--;
        if (rotpb_cnt==(255-60)) { // 60 x 20ms = 1.2 sec for HOLD
            rot_enc_input = C_ROT_HOLD;
        }
        if (rotpb_cnt==(255-120)) { // 120 x 20ms = 2.4 sec for LONG HOLD
            rot_enc_input = C_ROT_LONGHOLD;
        }
        if (rotpb_cnt==(255-254)) { // 240 x 20ms = 4.8 sec for EXTRA LONG HOLD
            rot_enc_input = C_ROT_EXTRALONGHOLD;
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
