#include "ublox_gps.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"

volatile u8 rx_buff[256];

volatile u16 heading_true;
volatile u8 time_hour;
volatile u8 time_min;
volatile u8 time_sec;
volatile u16 fly_min;
volatile u32 fly_sec;
volatile u16 altitude_feet;
volatile u16 ground_speed_mph;
volatile u8 vpos_acc;
volatile u8 hpos_acc;
volatile u32 lat;
volatile u32 lon;
volatile u8 fix;
volatile u8 speed_acc;
volatile u16 course_acc;
volatile u8 year;
volatile u8 month;
volatile u8 day;
volatile u8 accuracy;

volatile u8 gps_flags = 0;
#define gps_pkt0_flag   0b00000001
#define gps_pkt1_flag   0b00000010

extern const s_param_settings PARAM_LIST[PARAM_CNT];

void ublox_gps_rx_irq(u8 rx_byte) // entire IRQ < 312 instruction cycles
{
    static u8 rx_pos = 0;
    static u8 rx_end = 255;

    rx_buff[rx_pos] = rx_byte;
    switch(rx_pos) {
        case 0: // sync char 1
        case 128:
        {
            if (rx_byte==UBX_SYNC1)
                rx_pos++;
            break;
        }
        case 1: // sync char 2
        case 1+128:
        {
            if (rx_byte==UBX_SYNC2) {
                rx_pos++;
            } else {
                if (rx_byte!=UBX_SYNC1)
                    rx_pos--;
            }
            break;
        }
        case 5: // end of header
        case 5+128:
        {
            if (rx_byte==0x00) // MSB of length: expect 0x00
                rx_end = UBX_PKT_LEN(rx_buff[rx_pos-1]); // length + header
            rx_pos++;
            break;
        }
        default:
        {
            rx_pos++;
            break;
        }
    }
    if (rx_pos == rx_end) {
        gps_flags |= gps_pkt0_flag;
        rx_pos = 128; // next packet
        rx_end = 255;
    } else if (rx_pos == rx_end+128) {
        gps_flags |= gps_pkt1_flag;
        rx_pos = 0; // next packet
        rx_end = 255;
    }
}

void ublox_gps_process(void)
{
    ubx_packet *rx_pkt = 0;
    u8 i;
    if (gps_flags & gps_pkt0_flag)
    {
        gps_flags &= ~gps_pkt0_flag; // clear flag
        rx_pkt = (ubx_packet *)&rx_buff[0];
    }
    if (gps_flags & gps_pkt1_flag)
    {
        gps_flags &= ~gps_pkt1_flag; // clear flag
        rx_pkt = (ubx_packet *)&rx_buff[128];
    }
    if (rx_pkt)
    {
        u8 gps_ck_a = 0; // calculate checksum
        u8 gps_ck_b = 0;
        for (i=2;i<rx_pkt->_len+6;i++) // start:_class, end:before _ck
        {
            gps_ck_a += rx_pkt->data[i];
            gps_ck_b += gps_ck_a;
        }
        if (  (gps_ck_a == rx_pkt->data[rx_pkt->_len+6])&& // correct checksum
                (gps_ck_b == rx_pkt->data[rx_pkt->_len+7])  )
        {
            if (rx_pkt->_class == UBX_CLASS_NAV)
            switch(rx_pkt->_id)
            {
                case UBX_ID_NAV_POSECEF:
                {
                    if (rx_pkt->nav_posecef.pAcc > 25500)
                        accuracy = 255;
                    else accuracy = rx_pkt->nav_posecef.pAcc / 100;
                    break;
                }
                case UBX_ID_NAV_POSLLH:
                {
                    if (rx_pkt->nav_posllh.vAcc > 255000)
                        vpos_acc = 255;
                    else vpos_acc = rx_pkt->nav_posllh.vAcc / 1000;
                    if (rx_pkt->nav_posllh.hAcc > 255000)
                        hpos_acc = 255;
                    else hpos_acc = rx_pkt->nav_posllh.hAcc / 1000;
                    if (fix)
                    {
                        altitude_feet = (u16)(rx_pkt->nav_posllh.hMSL / 304.8);
                        lat = rx_pkt->nav_posllh.lat;
                        lon = rx_pkt->nav_posllh.lon;
                    } else {
                        altitude_feet = 0;
                        lat = 0;
                        lon = 0;
                    }
                    break;
                }
                case UBX_ID_NAV_STATUS:
                {
                    fix = rx_pkt->nav_status.gpsFix;
                    fly_sec = (rx_pkt->nav_status.msss / 1000);
                    break;
                }
                case UBX_ID_NAV_TIMEUTC:
                {
                    if (rx_pkt->nav_timeutc.valid & 0b111)
                    {
                        time_hour = rx_pkt->nav_timeutc.hour;
                        time_min = rx_pkt->nav_timeutc.min;
                        time_sec = rx_pkt->nav_timeutc.sec;
                        year = rx_pkt->nav_timeutc.year - 2000;
                        month = rx_pkt->nav_timeutc.month;
                        day = rx_pkt->nav_timeutc.day;
                    }
                    break;
                }
                case UBX_ID_NAV_VELNED:
                {
                    speed_acc = (u16)(rx_pkt->nav_velned.sAcc / 44.70400);
                    course_acc = rx_pkt->nav_velned.cAcc / 100000;
                    if (fix)
                    {
                        heading_true = rx_pkt->nav_velned.heading / 100000;
                        ground_speed_mph = (u16)(rx_pkt->nav_velned.gSpeed / 44.70400);
                    } else {
                        heading_true = 0;
                        ground_speed_mph = 0;
                    }
                    break;
                }
            }
        }
    }
}

void ublox_gps_init(void)
{
    UART_RXt = INPUT_PIN;
    _U1RXR = 36; // UART1_RX (input)  RB4 == RP36 -> U1RX (TABLE 11-2 in DS70000657H)

    U1BRG = (F_CY / (4 * 38400)) + 0.5; // Set BAUD rate: 38400 bps
    U1MODE = 0b1000000000001000; // UART Enable, 8N1 BRGH
    U1STA = 0b0000000000000000; // UART TX pin disabled; controlled by PORT
}


void* gps_menu(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case KP_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';

    strcopy(&rtxt[1+0*16], "Fix:");
    mprint_int(&rtxt[5+0*16], fix, 16, 2);

    strcopy(&rtxt[1+1*16], "Acc:");
    mprint_int(&rtxt[5+1*16], accuracy, 10, 0);

    strcopy(&rtxt[1+2*16], "TMR:");
    mprint_int(&rtxt[5+2*16], fly_sec, 10, 0);

    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &gps_menu;
}

void gps_vel_sendval(u8 idx)
{
    float mph = (float)ground_speed_mph;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, mph); // Send value
}

void gps_alt_sendval(u8 idx)
{
    float alt = (float)altitude_feet;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, alt); // Send value
}

void gps_hdg_sendval(u8 idx)
{
    float hdg = (float)heading_true;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, hdg); // Send value
}

void gps_time_sendval(u8 idx)
{
    float tmp;
    u8* tmp_ptr = (u8*)&tmp;
    tmp_ptr[0] = time_sec;
    tmp_ptr[1] = time_min;
    tmp_ptr[2] = time_hour;
    tmp_ptr[3] = 0;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp); // Send value
}

void gps_date_sendval(u8 idx)
{
    float tmp;
    u8* tmp_ptr = (u8*)&tmp;
    tmp_ptr[0] = day;
    tmp_ptr[1] = month;
    tmp_ptr[2] = year;
    tmp_ptr[3] = 0;
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp); // Send value
}
