#ifndef MODAIR_BUS_H
#define MODAIR_BUS_H

// MODAIR CAN-ID defines:
#define MA_PARAM_ID_MASK            0x0000FFFF // Source Parameter ID: uniquely identifies the module, sensor, or output
#define MA_PKT_nSOF_MASK            0x00010000 // Not Start of Frame flag (see MF_* defines)
#define MA_PKT_nEOF_MASK            0x00020000 // Not End of Frame flag
#define MA_MSGTYPE_MASK             0x0FF00000 // Message Type (see MT_* defines)

// FRAME defines:
#define MF_PKT_SINGLE               0x00 // SOF and EOF at the same time: we only expect a single data packet
#define MF_PKT_START                0x02 // Start of Frame data, expect continued data next
#define MF_PKT_CONT                 0x03 // Neither SOF nor EOF: continued data
#define MF_PKT_END                  0x01 // End of Frame data: this was the last data packet

// MSGTYPE defines: Message Types
#define MT_BROADCAST_VALUE          0x01 // Periodic sensor broadcasts
#define MT_BROADCAST_NAME           0x02 // ASCII name broadcast after powerup
#define MT_BROADCAST_ERROR_MSG      0x03 // ASCII error message
#define MT_BROADCAST_WARNING_MSG    0x04 // ASCII warning message
#define MT_BROADCAST_INFO_MSG       0x05 // ASCII information message
#define MT_TERMINAL_TXT             0x06 // ASCII text terminal / console / menu
#define MT_DATA_CH                  0x07 // Data channel for Firmware / Configuration upload and download
#define MT_REMOTE_CMD               0x11 // D<0:1> DESTINATION PARAM_ID (or DPI_* defines), D<2> CMD_CODE (see MC_* defines)
#define MT_REMOTE_ACK               0x12 // D<0:1> DESTINATION PARAM_ID (or DPI_* defines), D<2> ACK_CODE (see MK_* defines)

// DESTINATION PARAM_ID (unique ID for each module, sensor, and output), or one of these defines:
#define DPI_ALL_PARAMETERS          0xFFFF // Addresses all parameters (virtual nodes hosted on a physical node)
#define DPI_ALL_MODULES             0xFFFE // Addresses all modules (physical node)

// REMOTE_CMD defines: Remote Commands Codes (CMD_CODE)
#define MC_GET_VALUE                0x00
#define MC_GET_NAME                 0x01
#define MC_SET_VALUE                0x02 // D<4:7> new value
#define MC_TERMINAL_KEY             0x03 // Remote text terminal / console / menu: keystroke
#define MC_GET_VALUE_DTYPE          0x04 // Data type: s8,u8,s16,u16,s32,u32,float,double,time,date,char
#define MC_GET_VALUE_RMAX           0x05 // Range
#define MC_GET_VALUE_RMIN           0x06
#define MC_GET_VALUE_WMAX           0x07 // Warning
#define MC_GET_VALUE_WMIN           0x08
#define MC_GET_VALUE_AMAX           0x09 // Alarm
#define MC_GET_VALUE_AMIN           0x0A
#define MC_REBOOT                   0x10 // Reboot module
#define MC_FACTORY_RESET            0x11 // Restore default settings for module
#define MC_FIRMWARE_READ            0x12 // Read current firmware
#define MC_FIRMWARE_WRITE           0x13 // Program new firmware
#define MC_CONFIG_READ              0x14 // Read current configuration data
#define MC_CONFIG_WRITE             0x15 // Write new configuration data
#define MC_LOG_READ                 0x16
#define MC_LOG_CLEAR                0x17

// REMOTE_ACK defines: Remote Acknowledge Codes (ACK_CODE)
#define MK_OKAY                     0x01
#define MK_DONE                     0x02
#define MK_ERROR                    0xFF

// MC_TERMINAL_KEY defines: Rotary Encoder Keycodes
#define TK_ROT_INC                  0x81
#define TK_ROT_DEC                  0x82
#define TK_ROT_HOLD_INC             0x83
#define TK_ROT_HOLD_DEC             0x84
#define TK_ROT_PUSH                 0x85
#define TK_ROT_HOLD                 0x86
#define TK_ROT_LONGHOLD             0x87
#define TK_ROT_EXTRALONGHOLD        0x88

#endif
