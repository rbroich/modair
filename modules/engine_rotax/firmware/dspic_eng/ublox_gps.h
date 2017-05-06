#ifndef UBLOX_GPS_H
#define UBLOX_GPS_H

#include "common.h"

#define UART_RXt                    TRISBbits.TRISB4
#define UART_RX                     PORTBbits.RB4 // RP36

#define UBX_SYNC1                   0xB5
#define UBX_SYNC2                   0x62
#define UBX_CLASS_NAV               0x01

#define UBX_ID_NAV_POSECEF          0x01
#define UBX_ID_NAV_POSLLH           0x02
#define UBX_ID_NAV_STATUS           0x03
#define UBX_ID_NAV_TIMEUTC          0x21
#define UBX_ID_NAV_VELNED           0x12

#define UBX_PKT_LEN(len_payload)    (len_payload + 8)

typedef union {
    u8 data[128];              // [header_len==6], [payload], [checksum==2]
    struct {
        u8 _sync1;             // [0xB5]
        u8 _sync2;             // [0x62]
        u8 _class;             // [NAV==0x01]
        u8 _id;
        u16 _len;              // length of payload [excluding checksum]
        union {
            struct {
                u32 iTOW;      // [ms] GPS Millisecond Time of Week
                s32 ecefX;      // [cm] ECEF X coordinate
                s32 ecefY;      // [cm] ECEF Y coordinate
                s32 ecefZ;      // [cm] ECEF Z coordinate
                u32 pAcc;      // [cm] Position Accuracy Estimate
            } nav_posecef;          // [_id==0x01] Position Solution in ECEF
            struct {
                u32 iTOW;      // [ms] GPS Millisecond Time of Week
                s32 lon;        // [deg/1e7] Longitude
                s32 lat;        // [deg/1e7] Latitude
                s32 height;     // [mm] Height above Ellipsoid
                s32 hMSL;       // [mm] Height above mean sea level
                u32 hAcc;      // [mm] Horizontal Accuracy Estimate
                u32 vAcc;      // [mm] Vertical Accuracy Estimate
            } nav_posllh;           // [_id==0x02] Geodetic Position Solution
            struct {
                u32 iTOW;      // [ms] GPS Millisecond Time of Week
                u8 gpsFix;     // [0x00==no fix,0x02==2D fix,0x03==3D fix]
                u8 flags;      // Navigation Status Flags
                u8 fixStat;    // Fix Status Information
                u8 flags2;     // further information about navigation output
                u32 ttff;      // Time to first fix (millisecond time tag)
                u32 msss;      // Milliseconds since Startup / Reset
            } nav_status;           // [_id==0x03] Receiver Navigation Status
            struct {
                u32 iTOW;      // [ms] GPS Millisecond Time of Week
                s32 velN;       // [cm/s] NED north velocity
                s32 velE;       // [cm/s] NED east velocity
                s32 velD;       // [cm/s] NED down velocity
                u32 speed;     // [cm/s] Speed (3-D)
                u32 gSpeed;    // [cm/s] Ground Speed (2-D)
                s32 heading;    // [deg/1e5] Heading of motion 2-D
                u32 sAcc;      // [cm/s] Speed Accuracy Estimate
                u32 cAcc;      // [deg/1e5] Course / Heading Accuracy Estimate
            } nav_velned;           // [_id==0x12] Velocity Solution in NED
            struct {
                u32 iTOW;      // [ms] GPS Millisecond Time of Week
                u32 tAcc;      // [ns] Time Accuracy Estimate
                s32 nano;       // Nanoseconds of second, range -1e9..1e9
                u16 year;      // Year, range 1999..2099 (UTC)
                u8 month;      // Month, range 1..12 (UTC)
                u8 day;        // Day of Month, range 1..31 (UTC)
                u8 hour;       // Hour of Day, range 0..23 (UTC)
                u8 min;        // Minute of Hour, range 0..59 (UTC)
                u8 sec;        // Seconds of Minute, range 0..59 (UTC)
                u8 valid;      // Validity Flags [b2:UTC,b1:WeekNr,b0:ToW]
            } nav_timeutc;          // [_id==0x21] UTC Time Solution
        };
    };
} ubx_packet;

void ublox_gps_rx_irq(u8 rx_byte);
void ublox_gps_process(void);
void ublox_gps_init(void);

void* gps_menu(u8 idx, u8 key_input);
void gps_vel_sendval(u8 idx);
void gps_alt_sendval(u8 idx);
void gps_hdg_sendval(u8 idx);
void gps_time_sendval(u8 idx);
void gps_date_sendval(u8 idx);

#endif
