#include "i2c.h"

void i2c_init(void)
{
    I2C1CON = 0; // enable slew rate, 7-bit address
    I2C1BRG = 141; // 400 kHz ((1/FSCL - 120ns)*F_CY) - 2
    I2C1STAT = 0; // clear status bits
    I2C1CONbits.I2CEN = 1;
    u16 temp = I2C1RCV; // read buffer to clear buffer full
    i2c_stop(); // set bus to idle
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

u8 i2c_read(void)
{
    while (I2C1CON & 0b11111) // ensure that the master logic is inactive
        continue;
    I2C1CONbits.RCEN = 1;
    while (I2C1CONbits.RCEN)
        continue;
    while (!I2C1STATbits.RBF)
        continue;
    return (I2C1RCV);
}

u8 i2c_write(u8 data)
{
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
