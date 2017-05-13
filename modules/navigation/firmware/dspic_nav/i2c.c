#include "i2c.h"

void i2c_init(void)
{
    I2C1CON = 0; // enable slew rate, 7-bit address
    I2C1BRG = 141; // 400 kHz ((1/FSCL - 120ns)*F_CY) - 2
    I2C1STAT = 0; // clear status bits
    I2C1CONbits.I2CEN = 1;
    I2C1RCV; // read buffer to clear buffer full
    i2c_stop(); // set bus to idle
}

void i2c_wait_idle(void)
{
    while ((I2C1CON & 0b11111)||(I2C1STATbits.TRSTAT))
        continue; // ensure that the master logic is inactive
}

void i2c_start(void)
{
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN)
        continue;
}

void i2c_restart(void)
{
    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN)
        continue;
}

void i2c_stop(void)
{
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN)
        continue;
}

void i2c_ack(void)
{
    I2C1CONbits.ACKDT = 0; // ack
    I2C1CONbits.ACKEN = 1;
    while (I2C1CONbits.ACKEN)
        continue;
}

void i2c_nack(void)
{
    I2C1CONbits.ACKDT = 1; // nack
    I2C1CONbits.ACKEN = 1;
    while (I2C1CONbits.ACKEN)
        continue;
}

u8 i2c_receive(void)
{
    i2c_wait_idle();
    I2C1CONbits.RCEN = 1;
    while (I2C1CONbits.RCEN)
        continue;
    while (!I2C1STATbits.RBF)
        continue;
    return (I2C1RCV);
}

u8 i2c_send(u8 data)
{
    i2c_wait_idle();
    I2C1TRN = data;
    if (I2C1STATbits.IWCOL) { // Write Collision Detect
        I2C1STATbits.IWCOL = 0;
        return (-1);
    }
    while (I2C1STATbits.TBF)
        continue;
    while (I2C1STATbits.TRSTAT) // wait for the ACK
        continue;
    if (I2C1STATbits.ACKSTAT) // test for ACK condition received
        return (-2); // return NACK
    else return (0); //return ACK
}

void i2c_read_bytes(u8 i2c_addr, u8 addr, u8 *data, u8 len)
{
    u8 i;
    i2c_wait_idle();
    i2c_start();
    i2c_send(i2c_addr | 0x00); // write mode
    i2c_send(addr); // set address to read from
    i2c_restart();
    i2c_send(i2c_addr | 0x01); // start read mode
    for (i=0;i<len;i++) {
        data[i] = i2c_receive();
        if (i==len-1)
            i2c_nack(); // nack the last one
        else
            i2c_ack(); // ack all other reads
    }
    i2c_stop();
}

void i2c_write_bytes(u8 i2c_addr, u8 addr, u8 *data, u8 len)
{
    u8 i;
    i2c_wait_idle();
    i2c_start();
    i2c_send(i2c_addr | 0x00); // write mode
    i2c_send(addr);
    for (i=0;i<len;i++) {
        i2c_send(data[i]);
    }
    i2c_stop();
}
