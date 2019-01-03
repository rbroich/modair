#include "ecan_mod.h"
#include "string.h"
#include "params.h"
#include "iopins.h"
#include "modair_bus.h"

__attribute__((aligned(NR_ECAN_BUF*16))) u16 ecan_buf[NR_ECAN_BUF][8];

extern const s_param_fptr PARAM_CONST[PARAM_CNT];

void ecan_process(void)
{
    static u8 next_rx_buff = 0;
    u8 i, j, cb;
    u8 buf_sel;
    cb = next_rx_buff; // ensure buffers are processed in their received order
    for (j=0;j<8;j++) {
        buf_sel = 8 + ((cb+j)&0x7); // first 8 are Tx, last 8 are Rx
        if (C1RXFUL1 & (1<<buf_sel)) { // receive buffer full flag
            led_toggle();
            next_rx_buff = cb+j+1;
            if (ecan_buf[buf_sel][0] & 0x0001) { // Extended Identifier (29-bit ID)
                // CID = CAN-ID = 0b000s.ssss.ssss.ssee.eeee.eeee.eeee.eeee
                // where s [bit28:18] = SID (11 bit)
                //       e [bit17:00] = EID (18 bit)
                u8 msg_type = (ecan_buf[buf_sel][0] & 0x03FC) >> 2;
                u8 len = ecan_buf[buf_sel][2]&0x000F; // Data Length Code
                u16 pid = (ecan_buf[buf_sel][2]&0xF000) >> 12;
                pid |= ecan_buf[buf_sel][1] << 4;
                u8 flags = (ecan_buf[buf_sel][2] & 0x0C00) >> 10;
                if (ecan_buf[buf_sel][2] & 0x0200) // remote frame request
                    flags |= ECAN_FLAGS_RTR;
                for (i=0;i<PARAM_CNT;i++)
                    if (PARAM_CONST[i].canrx_fnc_ptr)
                        PARAM_CONST[i].canrx_fnc_ptr(i,pid,&ecan_buf[buf_sel][3],msg_type,flags,len);
            }
            C1RXFUL1 = 0xFFFF ^ (1<<buf_sel); // Clear Rx Buffer Full Flag
        } else break;
    }
}

void ecan_tx_console(u16 pid, char *str)
{
    while ((_TXREQ0)||(_TXREQ1)||(_TXREQ2)||(_TXREQ3)||
            (_TXREQ4)||(_TXREQ5)||(_TXREQ6)||(_TXREQ7))
        ; // wait for all 8 transmit buffers to become available
    u8 i;
    if (str) {
        // Ensure packets are sent in order...

        // START packet: TX7 has highest natural priority
        ecan_buf[7][0] = (MT_CONSOLE_TEXT<<2) | 0b11;
        ecan_buf[7][1] = (pid&0xFFF0)>>4;
        ecan_buf[7][2] = ((pid&0x000F)<<12) | (FT_PKT_START<<10) | 8; // 8 data bytes
        memcpy((char*)&ecan_buf[7][3],&str[0],8);
        _TXREQ7=1; // transmit buffer 7

        // CONT packets: buffers 6 downto 1
        for (i=1;i<7;i++) {
            ecan_buf[7-i][0] = (MT_CONSOLE_TEXT<<2) | 0b11;
            ecan_buf[7-i][1] = (pid&0xFFF0)>>4;
            ecan_buf[7-i][2] = ((pid&0x000F)<<12) | (FT_PKT_CONT<<10) | 8; // 8 data bytes
            memcpy((char*)&ecan_buf[7-i][3],&str[i*8],8);
            switch (i) { // Request Transmission on current buffer
                case 1: _TXREQ1=1; break;
                case 2: _TXREQ2=1; break;
                case 3: _TXREQ3=1; break;
                case 4: _TXREQ4=1; break;
                case 5: _TXREQ5=1; break;
                case 6: _TXREQ6=1; break;
            }
        }

        // END packet: buffer 0
        ecan_buf[0][0] = (MT_CONSOLE_TEXT<<2) | 0b11;
        ecan_buf[0][1] = (pid&0xFFF0)>>4;
        ecan_buf[0][2] = ((pid&0x000F)<<12) | (FT_PKT_END<<10) | 8; // 8 data bytes
        memcpy((char*)&ecan_buf[0][3],&str[7*8],8);
        _TXREQ0=1; // transmit buffer 0
    } else {
        ecan_buf[7][0] = (MT_CONSOLE_TEXT<<2) | 0b11;
        ecan_buf[7][1] = (pid&0xFFF0)>>4;
        ecan_buf[7][2] = ((pid&0x000F)<<12) | (FT_PKT_SINGLE<<10) | 1; // 1 data byte
        ecan_buf[7][3] = 0x00; // 0x00 == terminate string
        _TXREQ7=1; // transmit buffer 7
    }
}

void ecan_tx_float(u16 pid, u16 msg_type, float val)
{
    u16 *val_ptr = (u16 *)&val;
    while (_TXREQ0) ; // wait for first transmit buffer to become available
    ecan_buf[0][0] = (msg_type<<2) | 0b11;
    ecan_buf[0][1] = (pid&0xFFF0)>>4;
    ecan_buf[0][2] = ((pid&0x000F)<<12) | (FT_PKT_SINGLE<<10) | 4;
    ecan_buf[0][3] = val_ptr[0]; // byte 0 and 1: TODO!!! check endianness
    ecan_buf[0][4] = val_ptr[1]; // byte 2 and 3
    _TXREQ0=1;
}

void ecan_tx_str(u16 pid, char* str, u16 msg_type, u8 len)
{
    while (_TXREQ0) ; // wait for first transmit buffer to become available
    ecan_buf[0][0] = (msg_type<<2) | 0b11;
    ecan_buf[0][1] = (pid&0xFFF0)>>4;
    ecan_buf[0][2] = ((pid&0x000F)<<12) | (FT_PKT_SINGLE<<10) | len;
    char* dst = (char*)&ecan_buf[0][3];
    memcpy(dst,str,len);
    _TXREQ0=1;
}

void ecan_tx(u16 pid, u16 d0, u16 d2, u16 d4, u16 d6, u16 msg_type, u8 flags, u8 len)
{
    while (_TXREQ0) ; // wait for first transmit buffer to become available
    // Always transmit using Extended Identifier format
    ecan_buf[0][0] = (msg_type<<2) | 0b11;
    ecan_buf[0][1] = (pid&0xFFF0)>>4;
    ecan_buf[0][2] = ((pid&0x000F)<<12) | ((flags&0x3)<<10) | ((flags&ECAN_FLAGS_RTR)?0x200:0) | len;
    ecan_buf[0][3] = d0; // byte 0 and 1
    ecan_buf[0][4] = d2; // byte 2 and 3
    ecan_buf[0][5] = d4; // byte 4 and 5
    ecan_buf[0][6] = d6; // byte 6 and 7
    _TXREQ0=1;
}

void ecan_init(void)
{
    // I/O Pin direction, mode, and re-mapping (PPS)
    U1RXt = INPUT_PIN;
    U1TX = 1; // recessive state by default
    U1TXt = OUTPUT_PIN;
    U1RXa = DIGITAL_PIN;
    _RP39R = 14; // CAN_TXD (output) RB7 == RP39 <- C1TX (TABLE 11-3 in DS70000657H)
    _C1RXR = 34; // CAN_RXD (input)  RB2 == RPI34 -> C1RX (TABLE 11-2 in DS70000657H)

    C1CTRL1bits.REQOP = 0b100; // enter config mode
    while(C1CTRL1bits.OPMODE != 0b100); // wait for config mode

    // Memory Map Select (0 or 1: C1CTRLx,C1VEC,C1FCTRL,C1FIFO,C1INTF,C1INTE,C1EC,C1CFGx,C1FEN1,C1FMSKSELx)

    // Set Bit Timing to 1 Mbps
    // N = 20 time quanta per bit;   FTQ = N x baud = 20 MHz
    C1CTRL1bits.CANCKS = 1; // FCAN is equal to 2*FP (==2*F_CY)
    C1CFG2bits.SEG1PH = 0x7; // Phase Segment 1 time is 8 TQ
    C1CFG2bits.SEG2PHTS = 0x1; // Phase Segment 2 time is set to be programmable
    C1CFG2bits.SEG2PH = 0x5; // Phase Segment 2 time is 6 TQ
    C1CFG2bits.PRSEG = 0x4; // Propagation Segment time is 5 TQ
    C1CFG2bits.SAM = 0x1; // Bus line is sampled three times at the sample point
    C1CFG1bits.SJW = 0x3; // Synchronization Jump Width set to 4 TQ
    C1CFG1bits.BRP = 0x2; // Baud Rate Prescaler = FCAN /(2*FTQ) -1

    // Memory Map Select (1: C1BUFPNTx,C1RXMxSID,C1RXMxEID,C1RXFxSID,C1RXFxEID)
    C1CTRL1bits.WIN = 1;

    // Configure Receive Buffers [buffers 8 to 15]
    C1FCTRLbits.DMABS = 0b100; // 16 buffers in device RAM
    C1FCTRLbits.FSA = 8; // FIFO starts at buffer 8 (rx buffers)
    C1RXF0SIDbits.SID = 0; // configure filter 0
    C1RXF0SIDbits.EXIDE = 1; // extended identifier address
    C1RXF0SIDbits.EID = 0;
	C1RXF0EIDbits.EID = 0;
    C1BUFPNT1bits.F0BP = 0b1111; // assign filter 0 to FIFO
    C1RXM0SIDbits.SID = 0; // configure mask 0 (all bits are ignored in filter comparison)
    C1RXM0EIDbits.EID = 0;
    C1FMSKSEL1bits.F0MSK = 0; // select mask 0 for filter 0
	C1FEN1 = 0x1; // enable filter 0

    // Memory Map Select (0: C1RXFULx,C1RXOVFx,C1TRxxCON,C1RXD,C1TXD)
    C1CTRL1bits.WIN = 0;

    // Configure Transmit Buffers [buffers 0 to 7]
    C1TR01CONbits.TXEN0 = ENABLE; // buffers are transmit buffers
    C1TR01CONbits.TXEN1 = ENABLE;
    C1TR23CONbits.TXEN2 = ENABLE;
    C1TR23CONbits.TXEN3 = ENABLE;
    C1TR45CONbits.TXEN4 = ENABLE;
    C1TR45CONbits.TXEN5 = ENABLE;
    C1TR67CONbits.TXEN6 = ENABLE;
    C1TR67CONbits.TXEN7 = ENABLE;

    // Setup DMA0 for TX_RAM -> CAN_TX
    DMA0CONbits.SIZE = 0x0; // word instead of byte
    DMA0CONbits.DIR = 0x1; // read from RAM address, write to peripheral address
    DMA0CONbits.AMODE = 0x2; // Peripheral Indirect Addressing mode
    DMA0CONbits.MODE = 0x0; // Continuous, Ping-Pong modes disabled
    DMA0REQ = 70; // DMA Peripheral IRQ Number Select: ECAN1 ? TX Data Request
    DMA0CNT = 7;
    DMA0PAD = (volatile u16)&C1TXD;
    DMA0STAL = (u16)&ecan_buf[0][0];
    DMA0STAH = (u16)&ecan_buf[0][0];
    DMA0CONbits.CHEN = ENABLE;

    // Setup DMA1 for CAN_RX -> RX_RAM
    DMA1CONbits.SIZE = 0x0; // word instead of byte
    DMA1CONbits.DIR = 0x0; // Read from Peripheral address, write to RAM address
    DMA1CONbits.AMODE = 0x2; // Peripheral Indirect Addressing mode
    DMA1CONbits.MODE = 0x0; // Continuous, Ping-Pong modes disabled
    DMA1REQ = 34; // DMA Peripheral IRQ Number Select: ECAN1 ? RX Data Ready
    DMA1CNT = 7;
    DMA1PAD = (volatile u16)&C1RXD;
    DMA1STAL = (u16)&ecan_buf[0][0];
    DMA1STAH = (u16)&ecan_buf[0][0];
    DMA1CONbits.CHEN = ENABLE;

    C1CTRL1bits.REQOP = 0; // enter normal mode
    while(C1CTRL1bits.OPMODE != 0); // wait for normal mode
}
