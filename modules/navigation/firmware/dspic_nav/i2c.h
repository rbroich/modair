#ifndef I2C_H
#define I2C_H

#include "common.h"


void i2c_init(void);
u8 i2c_write(u8 data);
u8 i2c_read(void);
void i2c_ack(void);
void i2c_nack(void);
void i2c_stop(void);
void i2c_start(void);
void i2c_restart(void);

#endif
