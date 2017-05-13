#ifndef BMP180_DRIV_H
#define BMP180_DRIV_H

#include "i2c.h"

#define BMP180_READ_CMD             0xEF
#define BMP180_WRITE_CMD            0xEE

#define BMP180_ULTRALOWPOWER        0
#define BMP180_STANDARD             1
#define BMP180_HIGHRES              2
#define BMP180_ULTRAHIGHRES         3
#define BMP180_CAL_AC1              0xAA  // R   Calibration data (16 bits)
#define BMP180_CAL_AC2              0xAC  // R   Calibration data (16 bits)
#define BMP180_CAL_AC3              0xAE  // R   Calibration data (16 bits)
#define BMP180_CAL_AC4              0xB0  // R   Calibration data (16 bits)
#define BMP180_CAL_AC5              0xB2  // R   Calibration data (16 bits)
#define BMP180_CAL_AC6              0xB4  // R   Calibration data (16 bits)
#define BMP180_CAL_B1               0xB6  // R   Calibration data (16 bits)
#define BMP180_CAL_B2               0xB8  // R   Calibration data (16 bits)
#define BMP180_CAL_MB               0xBA  // R   Calibration data (16 bits)
#define BMP180_CAL_MC               0xBC  // R   Calibration data (16 bits)
#define BMP180_CAL_MD               0xBE  // R   Calibration data (16 bits)

#define BMP180_ID                   0xD0
#define BMP180_CONTROL              0xF4
#define BMP180_TEMPDATA             0xF6
#define BMP180_PRESSUREDATA         0xF6
#define BMP180_READTEMPCMD          0x2E
#define BMP180_READPRESSURECMD      0x34

void bmp180_tmr_irq(void);
void bmp180_process(void);
u8 bmp180_read_u8(u8 addr);
u16 bmp180_read_u16(u8 addr);
void bmp180_write(u8 addr, u8 data);

void bmp180_init(u8 mode);
void bmp180_init_raw_temperature(void);
void bmp180_read_temperature(void);
void bmp180_init_raw_pressure(void);
void bmp180_read_pressure(void);
s32 bmp180_read_QNH(float altitude_meters);
s32 bmp180_get_temperature(void);
float bmp180_get_altitude(s32 QNH);

void bmp180_sendpres(u8 idx);
void bmp180_sendtemp(u8 idx);
void bmp180_sendqnh(u8 idx);
void bmp180_sendaltqnh(u8 idx);
void bmp180_sendaltfl(u8 idx);
void* bmp180_editqnh(u8 idx, u8 key_input);
void* bmp180_homescreen(u8 idx, u8 key_input);

#endif
