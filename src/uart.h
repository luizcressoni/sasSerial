/*! \file uart.h */
#pragma once

// --- National Semiconductor UART 16550 ------------------------------
/* UART register offsets relative to base address
 DLL and DLM registers can be accessed by setting the DLAB bit.
        register            read/write access */
#define UART_THR            0 // write
#define UART_RBR            0 // read
#define UART_DLL            0 // read/write
#define UART_IER            1 // read/write
#define UART_DLM            1 // read/write
#define UART_IIR            2 // read
#define UART_FCR            2 // write
#define UART_LCR            3 // read/write
#define UART_MCR            4 // read/write
#define UART_LSR            5 // read
#define UART_MSR            6 // read
#define UART_SR             7 // read/write

// UART Line Status Register (LSR) flags
#define UART_LSR_DR          0x01  // Data Ready
#define UART_LSR_OE          0x02  // Overrun Error
#define UART_LSR_PE          0x04  // Parity Error
#define UART_LSR_FE          0x08  // Framing Error
#define UART_LSR_BI          0x10  // Break Interrupt
#define UART_LSR_THRE        0x20  // Transmitter Holding Register Empty
#define UART_LSR_TEMT        0x40  // Transmitter Empty (FIFO + Shift register)
#define UART_LSR_FIFO        0x80  // FIFO Error (16550 and later)

// UART Interrupt Enable Register (IER) flags
#define UART_IER_ERBFI       0x01  // Enable Received Data Available interrupt
#define UART_IER_ETBEI       0x02  // Enable Transmitter Holding Register Empty interrupt
#define UART_IER_ELSI        0x04  // Enable Receiver Line Status interrupt
#define UART_IER_EDSSI       0x08  // Enable Modem Status interrupt
// 16550 and later (optional, hardware dependent)
#define UART_IER_SLEEP       0x10  // Enable sleep mode
#define UART_IER_LP          0x20  // Low power mode

// UART FIFO Control Register (FCR) flags
#define UART_FCR_ENABLE      0x01  // Enable FIFO
#define UART_FCR_RX_RESET    0x02  // Clear RX FIFO
#define UART_FCR_TX_RESET    0x04  // Clear TX FIFO
#define UART_FCR_DMA_MODE    0x08  // DMA mode select
// RX trigger levels (16550)
#define UART_FCR_TRIG_1      0x00
#define UART_FCR_TRIG_4      0x40
#define UART_FCR_TRIG_8      0x80
#define UART_FCR_TRIG_14     0xC0

// UART Modem Control Register (MCR) flags
#define UART_MCR_DTR         0x01  // Data Terminal Ready
#define UART_MCR_RTS         0x02  // Request To Send
#define UART_MCR_OUT1        0x04  // Auxiliary output 1
#define UART_MCR_OUT2        0x08  // Auxiliary output 2 (often enables IRQ)
#define UART_MCR_LOOP        0x10  // Loopback mode
// 16750 and later
#define UART_MCR_AFCE        0x20  // Auto flow control enable


enum enumUartParity{
    UART_NO_PARITY   = 0,
    UART_ODD_PARITY  = 1,
    UART_EVEN_PARITY = 3,
    UART_MARK        = 5,
    UART_SPACE       = 7,
};
//eof uart_defines.h