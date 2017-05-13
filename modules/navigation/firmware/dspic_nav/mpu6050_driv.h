#ifndef MPU6050_DRIV_H
#define MPU6050_DRIV_H

#include "i2c.h"

#define MPU6050_I2C_ADDR             0xD0

#define MPU6050_REG_GYRO_CONFIG      0x1B
#define MPU6050_REG_ACCEL_CONFIG     0x1C
#define MPU6050_REG_INT_PIN_CFG      0x37
#define MPU6050_REG_PWR_MGMT_1       0x6B

#define MPU6050_REG_ACCEL_XOUT_H     0x3B
#define MPU6050_REG_ACCEL_XOUT_L     0x3C
#define MPU6050_REG_ACCEL_YOUT_H     0x3D
#define MPU6050_REG_ACCEL_YOUT_L     0x3E
#define MPU6050_REG_ACCEL_ZOUT_H     0x3F
#define MPU6050_REG_ACCEL_ZOUT_L     0x40
#define MPU6050_REG_TEMP_OUT_H       0x41
#define MPU6050_REG_TEMP_OUT_L       0x42
#define MPU6050_REG_GYRO_XOUT_H      0x43
#define MPU6050_REG_GYRO_XOUT_L      0x44
#define MPU6050_REG_GYRO_YOUT_H      0x45
#define MPU6050_REG_GYRO_YOUT_L      0x46
#define MPU6050_REG_GYRO_ZOUT_H      0x47
#define MPU6050_REG_GYRO_ZOUT_L      0x48

void mpu6050_writereg(u8 addr, u8 val);
u8 mpu6050_readreg(u8 addr);

void mpu6050_process(void);
void mpu6050_init(void);
void* mpu6050_homescreen(u8 idx, u8 key_input);

#endif
