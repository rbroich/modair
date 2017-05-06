#ifndef MODAIR_BUS_H
#define MODAIR_BUS_H

// MODAIR CAN-ID defines:
#define MA_PKT_TYPE_MASK            0x00000003 // Packet Frame Type (see FT_* defines)
#define MA_PARAM_ID_MASK            0x0003FFFC // Source Parameter ID: uniquely identifies the module, sensor, or output
#define MA_MSGTYPE_MASK             0x03FC0000 // Message Type (see MT_* defines)

// FRAME defines:
#define FT_PKT_SINGLE               0x0  // We only expect a single data packet
#define FT_PKT_START                0x1  // Start of Frame data, expect continued data next
#define FT_PKT_CONT                 0x2  // Continued data
#define FT_PKT_END                  0x3  // End of Frame data: this was the last data packet

// MSGTYPE defines: Message Types
#define MT_BROADCAST_ALARM          0x01 // Sensor alarm broadcasts
#define MT_REMOTE_CMD               0x02 // D<0:1> DESTINATION PARAM_ID (or DPI_* defines), D<2> CMD_CODE (see RC_* defines)
#define MT_BROADCAST_VALUE          0x03 // Sensor broadcasts (periodic, requested, or as a response to SET_VALUE)
#define MT_BROADCAST_MSG_ERROR      0x04 // Error message
#define MT_BROADCAST_MSG_WARNING    0x05 // Warning message
#define MT_BROADCAST_MSG_INFO       0x06 // Information message
#define MT_BROADCAST_NAME           0x07 // Broadcast name (after powerup or requested)
#define MT_CONSOLE_TEXT             0x20 // Text terminal / console / menu
#define MT_REMOTE_ACK               0x2E // D<0:1> DESTINATION PARAM_ID (or DPI_* defines), D<2> ACK_CODE (see AK_* defines)
#define MT_DATA_CHANNEL             0x2F // Data channel for Firmware / Configuration upload and download

// DESTINATION PARAM_ID (unique ID for each module, sensor, and output), or one of these defines:
#define DPI_ALL_PARAMETERS          0xFFFF // Addresses all parameters (virtual nodes hosted on a physical node)
#define DPI_ALL_MODULES             0xFFFE // Addresses all modules (physical node)

// MT_REMOTE_CMD defines: Remote Commands Codes (CMD_CODE)
#define RC_GET_VALUE                0x00
#define RC_GET_NAME                 0x01
#define RC_SET_VALUE                0x02 // D<4:7> new value
#define RC_CONSOLE_KEY              0x03 // Remote terminal / console: <D3> KeyPress (see KP_* defines)
#define RC_GET_VALUE_DTYPE          0x04 // Data type: s8,u8,s16,u16,s32,u32,float,double,time,date,char
#define RC_GET_VALUE_RMAX           0x05 // Range
#define RC_GET_VALUE_RMIN           0x06
#define RC_GET_VALUE_WMAX           0x07 // Warning
#define RC_GET_VALUE_WMIN           0x08
#define RC_GET_VALUE_AMAX           0x09 // Alarm
#define RC_GET_VALUE_AMIN           0x0A
#define RC_REBOOT                   0x10 // Reboot module
#define RC_FACTORY_RESET            0x11 // Restore default settings for module
#define RC_FIRMWARE_READ            0x12 // Read current firmware
#define RC_FIRMWARE_WRITE           0x13 // Program new firmware
#define RC_CONFIG_READ              0x14 // Read current configuration data
#define RC_CONFIG_WRITE             0x15 // Write new configuration data
#define RC_LOG_READ                 0x16
#define RC_LOG_CLEAR                0x17

// MT_REMOTE_ACK defines: Remote Acknowledge Codes (ACK_CODE)
#define AK_OKAY                     0x01
#define AK_DONE                     0x02
#define AK_ERROR                    0xFF

// RC_CONSOLE_KEY defines: Key Press codes
#define KP_ASCII                    0x00 // 0x00 to 0x7F: standard ASCII chars
#define KP_ROT_INC                  0x81
#define KP_ROT_DEC                  0x82
#define KP_ROT_HOLD_INC             0x83
#define KP_ROT_HOLD_DEC             0x84
#define KP_ROT_PUSH                 0x85
#define KP_ROT_HOLD                 0x86
#define KP_ROT_LONGHOLD             0x87
#define KP_ROT_EXTRALONGHOLD        0x88

#endif
