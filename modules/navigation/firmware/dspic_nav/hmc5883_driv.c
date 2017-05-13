#include "hmc5883_driv.h"
#include "math.h"
#include "ecan_mod.h"
#include "params.h"
#include "modair_bus.h"
#include "led.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];

void hmc5883_writereg(u8 addr, u8 val)
{
    i2c_write_bytes(HMC5883_I2C_ADDR, addr, &val, 1);
}

u8 hmc5883_readreg(u8 addr)
{
    u8 val;
    i2c_read_bytes(HMC5883_I2C_ADDR, addr, &val, 1);
    return val;
}

void hmc5883_process(void)
{

}

void hmc5883_init(void)
{
    hmc5883_writereg(HMC5883_REG_CONFIG_A, 0x78); // 8 sample averaging, 75 Hz sample rate, no biasing
    //hmc5883_writereg(HMC5883_REG_CONFIG_B, 0x40); // scaling: 1090 LSB / Gauss 
    hmc5883_writereg(HMC5883_REG_MODE, 0x00); // continuous measurement mode
}

void* hmc5883_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "HMC5883L");

    u8 buf[6];
    i2c_read_bytes(HMC5883_I2C_ADDR, 0x03, buf, 6);
    mprint_int(&rtxt[1*16 + 0],buf[0],16,2); // mx
    mprint_int(&rtxt[1*16 + 2],buf[1],16,2);
    mprint_int(&rtxt[1*16 + 5],buf[2],16,2); // mz
    mprint_int(&rtxt[1*16 + 7],buf[3],16,2);
    mprint_int(&rtxt[1*16 + 10],buf[4],16,2); // my
    mprint_int(&rtxt[1*16 + 12],buf[5],16,2);
    
    s16 mx = (buf[0]<<8) | buf[1];
    s16 my = (buf[4]<<8) | buf[5];
    float heading = atan2(my,mx);
    if (heading<0) heading += 2*3.1416;
    float headingDegrees = heading * 180.0/3.1416;
    
    mprint_float(&rtxt[2*16+1], headingDegrees, 0, 2);

    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &hmc5883_homescreen;
}
