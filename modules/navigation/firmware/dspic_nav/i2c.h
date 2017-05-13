#ifndef I2C_H
#define I2C_H

#include "common.h"


void i2c_init(void);
void i2c_wait_idle(void);
void i2c_start(void);
void i2c_restart(void);
void i2c_stop(void);
void i2c_ack(void);
void i2c_nack(void);

u8 i2c_receive(void);
u8 i2c_send(u8 data);

void i2c_read_bytes(u8 i2c_addr, u8 addr, u8 *data, u8 len);
void i2c_write_bytes(u8 i2c_addr, u8 addr, u8 *data, u8 len);

#endif
