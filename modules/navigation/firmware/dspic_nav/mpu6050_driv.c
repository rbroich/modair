#include "mpu6050_driv.h"
#include "math.h"
#include "ecan_mod.h"
#include "params.h"
#include "modair_bus.h"

extern const s_settings settings;

void mpu6050_writereg(u8 addr, u8 val)
{
    i2c_write_bytes(MPU6050_I2C_ADDR, addr, &val, 1);
}

u8 mpu6050_readreg(u8 addr)
{
    u8 val;
    i2c_read_bytes(MPU6050_I2C_ADDR, addr, &val, 1);
    return val;
}

void mpu6050_process(void)
{

}

void mpu6050_init(void)
{
    // power-on-reset values: 0x00 (all except PWR_MGMT_1 and WHO_AM_I registers)
    mpu6050_writereg(MPU6050_REG_PWR_MGMT_1, 0x01); // Take out of sleep, CLKSEL = PLL with X axis gyroscope reference
    mpu6050_writereg(MPU6050_REG_GYRO_CONFIG, 0x00); // Gyro Full Scale Range = +-250 deg/s (most sensitive)
    mpu6050_writereg(MPU6050_REG_ACCEL_CONFIG, 0x00); // Acc Full Scale Range = +-2g (most sensitive)
    mpu6050_writereg(MPU6050_REG_INT_PIN_CFG, 0x02); // I2C Bypass: allow direct access to AUX_I2C devices from host
}

void* mpu6050_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0; break;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "MPU6050");

    u8 buf[14];
    i2c_read_bytes(MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_XOUT_H, buf, 14);
    mprint_int(&rtxt[1*16 + 0],buf[0],16,2); // ax
    mprint_int(&rtxt[1*16 + 2],buf[1],16,2);
    mprint_int(&rtxt[1*16 + 5],buf[2],16,2); // ay
    mprint_int(&rtxt[1*16 + 7],buf[3],16,2);
    mprint_int(&rtxt[1*16 + 10],buf[4],16,2); // az
    mprint_int(&rtxt[1*16 + 12],buf[5],16,2);

    mprint_int(&rtxt[2*16 + 0],buf[6],16,2); // temp: int16val / 340 + 36.53 degrees C
    mprint_int(&rtxt[2*16 + 2],buf[7],16,2);

    mprint_int(&rtxt[3*16 + 0],buf[8],16,2); // gx
    mprint_int(&rtxt[3*16 + 2],buf[9],16,2);
    mprint_int(&rtxt[3*16 + 5],buf[10],16,2); // gy
    mprint_int(&rtxt[3*16 + 7],buf[11],16,2);
    mprint_int(&rtxt[3*16 + 10],buf[12],16,2); // gz
    mprint_int(&rtxt[3*16 + 12],buf[13],16,2);

    ecan_tx_console(settings.param[idx].pid, rtxt);
    return &mpu6050_homescreen;
}
