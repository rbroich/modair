#include "ecan_mod.h"

__attribute__((aligned(NR_ECAN_BUF*16))) u16 ecan_buf[NR_ECAN_BUF][8];

extern void ecan_rx(u32 cid, u8 len, u8 rtr, u16 *data);

void ecan_irq(void)
{
    u8 buf_sel = C1VECbits.ICODE;

    // CID = CAN-ID = 0b000s.ssss.ssss.ssee.eeee.eeee.eeee.eeee
    // where s [bit28:18] = SID (11 bit)
    //       e [bit17:00] = EID (18 bit)
    u32 cid = (u32)(ecan_buf[buf_sel][0]&0x1FFC) << (18-2);
    u8 len = ecan_buf[buf_sel][2]&0x000F; // Data Length Code
    u8 rtr = 0;
    if (ecan_buf[buf_sel][0] & 0x0001) { // Extended Identifier (29-bit ID)
        cid |= ((u32)ecan_buf[buf_sel][1] << 6);
        cid |= (ecan_buf[buf_sel][2] >> 10);
        if (ecan_buf[buf_sel][2] & 0x0200)
            rtr = 1; // remote frame request
    } else { // Standard Identifier (11-bit ID) Message
        if (ecan_buf[buf_sel][0] & 0x0002)
            rtr = 1; // remote frame request
    }
    ecan_rx(cid, len, rtr, &ecan_buf[buf_sel][3]);
    C1RXFUL1 = 0xFFFF ^ (1<<buf_sel); // Clear Rx Buffer Full Flag
    C1INTFbits.TBIF = 0; // Clear TX Buffer Interrupt Flag
    C1INTFbits.RBIF = 0; // Clear RX Buffer Interrupt Flag
}

u8 ecan_tx(u32 cid, u8 len, u8 rtr, u16 *data)
{
    u8 use_tx_buf; // find the first empty transmit buffer
    if (_TXREQ0 == 0)
        use_tx_buf = 0;
    else if (_TXREQ1 == 0)
        use_tx_buf = 1;
    else if (_TXREQ2 == 0)
        use_tx_buf = 2;
    else if (_TXREQ3 == 0)
        use_tx_buf = 3;
    else if (_TXREQ4 == 0)
        use_tx_buf = 4;
    else if (_TXREQ5 == 0)
        use_tx_buf = 5;
    else if (_TXREQ6 == 0)
        use_tx_buf = 6;
    else if (_TXREQ7 == 0)
        use_tx_buf = 7;
    else return -1;

    // Always transmit using Extended Identifier format
    ecan_buf[use_tx_buf][0] = ((cid&0x1FFC0000)>>16) | 0b11;
    ecan_buf[use_tx_buf][1] = ((cid&0x0003FFC0)>>6);
    ecan_buf[use_tx_buf][2] = ((cid&0x0000003F)<<10) | (rtr?0x200:0) | len;
    u8 i;
    for (i=0;i<(len+1)/sizeof(u16);i++) // copy data to transmit buffer
        ecan_buf[use_tx_buf][3+i] = data[i];

    switch (use_tx_buf) { // Request Transmission on current buffer
        case 0: _TXREQ0=1; break;
        case 1: _TXREQ1=1; break;
        case 2: _TXREQ2=1; break;
        case 3: _TXREQ3=1; break;
        case 4: _TXREQ4=1; break;
        case 5: _TXREQ5=1; break;
        case 6: _TXREQ6=1; break;
        case 7: _TXREQ7=1; break;
    }
    return use_tx_buf;
}

void ecan_init(void)
{
    // I/O Pin direction, mode, and re-mapping (PPS)
    U1RXt = INPUT_PIN;
    U1TX = 1; // recessive state by default
    U1TXt = OUTPUT_PIN;
    U1RXa = DIGITAL_PIN;
    _RP39R = 14; // CAN_TXD (output) RB7 == RP39 <- C1TX (TABLE 11-3 in DS70000657H)
    _C1RXR = 40; // CAN_RXD (input)  RB8 == RP40 -> C1RX (TABLE 11-2 in DS70000657H)

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

    C1INTEbits.RBIE = ENABLE; // Receive Buffer Interrupt Enable
}
