#ifndef HMC5883_DRIV_H
#define HMC5883_DRIV_H

#include "i2c.h"

#define HMC5883_I2C_ADDR             0x3C

#define HMC5883_REG_CONFIG_A         0x00
#define HMC5883_REG_CONFIG_B         0x01
#define HMC5883_REG_MODE             0x02

void hmc5883_writereg(u8 addr, u8 val);
u8 hmc5883_readreg(u8 addr);

void hmc5883_process(void);
void hmc5883_init(void);
void* hmc5883_homescreen(u8 idx, u8 key_input);

#endif
