#include "bmp180_driv.h"
#include "math.h"
#include "ecan_mod.h"
#include "params.h"
#include "modair_bus.h"

s32 user_QNH = 102400; // in Pa
u8 oversampling;
s32 X1, PR;
float OAT;
s16 ac1, ac2, ac3, b1, b2, mc, md;
u16 ac4, ac5, ac6;
volatile u8 bmp_state = 0;

extern const s_settings settings;

void bmp180_tmr_irq(void) // called every 1 ms
{
    static u8 cnt = 0;
    switch(cnt) {
        case 1: bmp_state = 1; break; // start temperature conversion (max 4.5ms)
        case 6: bmp_state = 2; break; // read temperature
        case 11: bmp_state = 3; break; // start pressure conversion (max 25.5ms)
        case 37: bmp_state = 4; break; // read pressure
        case 127: cnt=0; break;
    }
    cnt++;
}

void bmp180_process(void)
{
    if (bmp_state) {
        u8 tmp = bmp_state;
        bmp_state = 0;
        switch(tmp) {
            case 1: bmp180_init_raw_temperature(); break; // start temperature conversion
            case 2: bmp180_read_temperature(); break; // read temperature
            case 3: bmp180_init_raw_pressure(); break; // start pressure conversion
            case 4: bmp180_read_pressure(); break; // read and convert pressure
        }
    }
}

void bmp180_write(u8 addr, u8 data)
{
    i2c_write_bytes(BMP180_I2C_ADDR,addr,&data,1);
}

u16 bmp180_read_u16(u8 addr)
{
    u8 data[2];
    i2c_read_bytes(BMP180_I2C_ADDR,addr,data,2);
    return ((data[0]<<8) | data[1]);
}

u8 bmp180_read_u8(u8 addr)
{
    u8 data;
    i2c_read_bytes(BMP180_I2C_ADDR,addr,&data,2);
    return data;
}

void bmp180_init(u8 mode)
{
    delay_ms(10); // BMP180 start-up time before first communication

    oversampling = mode;

    if (bmp180_read_u8(BMP180_ID) != 0x55) // check that ID matches
        return;

    ac1 = bmp180_read_u16(BMP180_CAL_AC1);
    ac2 = bmp180_read_u16(BMP180_CAL_AC2);
    ac3 = bmp180_read_u16(BMP180_CAL_AC3);
    ac4 = bmp180_read_u16(BMP180_CAL_AC4);
    ac5 = bmp180_read_u16(BMP180_CAL_AC5);
    ac6 = bmp180_read_u16(BMP180_CAL_AC6);

    b1 = bmp180_read_u16(BMP180_CAL_B1);
    b2 = bmp180_read_u16(BMP180_CAL_B2);

//    mb = bmp180_read_u16(BMP180_CAL_MB);
    mc = bmp180_read_u16(BMP180_CAL_MC);
    md = bmp180_read_u16(BMP180_CAL_MD);
}

void bmp180_init_raw_temperature(void)
{
    bmp180_write(BMP180_CONTROL, BMP180_READTEMPCMD);
}

void bmp180_read_temperature(void)
{
    s32 B5, X2, UT;
    UT = bmp180_read_u16(BMP180_TEMPDATA);
    X1 = (UT - (s32)ac6) * ((s32)ac5) >> 15;
    X2 = ((s32)mc << 11) / (X1+(s32)md);
    B5 = X1 + X2;
    OAT = (0.1 / 16.0) * (B5+8); // resolution = 0.1 deg C
}

void bmp180_init_raw_pressure(void)
{
    bmp180_write(BMP180_CONTROL, BMP180_READPRESSURECMD + (oversampling << 6));
}

void bmp180_read_pressure(void)
{
    s32 B3, B5, B6, XL, X2, X3, UP, p;
    u32 B4, B7, raw;

    raw = bmp180_read_u16(BMP180_PRESSUREDATA);
    raw <<= 8;
    raw |= bmp180_read_u8(BMP180_PRESSUREDATA+2);
    raw >>= (8 - oversampling);
    UP = raw;

    X2 = ((s32)mc << 11) / (X1+(s32)md);
    B5 = X1 + X2;
    B6 = B5 - 4000;
    XL = ((s32)b2 * ( (B6 * B6)>>12 )) >> 11;
    X2 = ((s32)ac2 * B6) >> 11;
    X3 = XL + X2;
    B3 = (((((s32)ac1*4) + X3) << oversampling) + 2) / 4;

    XL = ((s32)ac3 * B6) >> 13;
    X2 = ((s32)b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((XL + X2) + 2) >> 2;
    B4 = ((u32)ac4 * (u32)(X3 + 32768UL)) >> 15;
    B7 = ((u32)UP - B3) * (u32)(50000UL >> oversampling);

    if (B7 < 0x80000000) {
        p = (B7 * 2) / B4;
    } else {
        p = (B7 / B4) * 2;
    }
    XL = (p >> 8) * (p >> 8);
    XL = (XL * 3038) >> 16;
    X3 = p * (-7357);
    X2 = X3 >> 16;

    p = p + ((XL + X2 + (s32)3791)>>4);
    PR = p; // Pressure in Pascal
}

float bmp180_get_altitude(s32 QNH) // qnh in Pa i.e. 101325 Pa
{
    float temp = (float)PR / QNH;
    temp = 1.0 - pow(temp, 0.19029);
    return (44330.0 * 3.28084 * temp); // in ft
}

void bmp180_sendaltfl(u8 idx)
{
    float alt_fl = bmp180_get_altitude(101325);
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, alt_fl); // Send value
}

void bmp180_sendqnh(u8 idx)
{
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, (float)user_QNH/100.0); // Send value
}

void bmp180_sendaltqnh(u8 idx)
{
    float alt_ft = bmp180_get_altitude(user_QNH);
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, alt_ft); // Send value
}

void bmp180_sendpres(u8 idx)
{
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, (float)PR); // Send value
}

void bmp180_sendtemp(u8 idx)
{
    ecan_tx_float(settings.param[idx].pid, MT_BROADCAST_VALUE, OAT); // Send value
}

void* bmp180_editqnh(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_INC: user_QNH += 100; break;
        case KP_ROT_DEC: user_QNH -= 100; break;
        case KP_ROT_HOLD: user_QNH = 101300; break;
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[0*16], "QNH:");
    mprint_int(&rtxt[0*16+5], user_QNH/100, 10, 4);
    strcopy(&rtxt[1*16], "Alt:");
    float alt_ft = bmp180_get_altitude(user_QNH);
    char* ptr = mprint_float(&rtxt[5+1*16], alt_ft, 0, 0);
    ptr[1] = 'f';
    ptr[2] = 't';
    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &bmp180_editqnh;
}

void* bmp180_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "Temperature");
    mprint_float(&rtxt[2+1*16], OAT, 0, 2);
    float alt_fl = bmp180_get_altitude(101325);
    mprint_float(&rtxt[2+2*16], alt_fl, 0, 1);
    float alt_ft = bmp180_get_altitude(user_QNH);
    mprint_float(&rtxt[2+3*16], alt_ft, 0, 1);

    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &bmp180_homescreen;
}
