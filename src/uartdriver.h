/*! \file uartdriver.h */
#pragma once
#include <string>
#include "uart.h"

#define UART_PARITY_ERROR       0x80
#define UART_RX_OVERRUN_ERROR   0x40
#define UART_HAS_RX             0x01

/**
 * @class cUartDriver
 * @brief Low-level UART driver using direct hardware register access.
 *
 * This class provides a minimal hardware abstraction for a UART device
 * (typically a 16550-compatible controller) by accessing the UART registers
 * directly through the I/O base address obtained from the system.
 *
 * The driver resolves the UART base address associated with a Linux device
 * name (e.g. "/dev/ttyS0") by scanning `/proc/tty/driver/serial`. Once the
 * address is obtained, the class performs direct register manipulation to
 * configure and operate the UART.
 *
 * @note
 * This driver assumes a system exposing traditional serial ports through
 * `/dev/ttyS*` backed by a 16550-compatible UART. It is not intended for
 * USB serial adapters (ttyUSB*) or ACM devices.
 *
 * @warning
 * Direct hardware access may require elevated privileges depending on the
 * platform and I/O permissions.
 *
 * @author luiz@cressoni.com.br
 *
 * @note
 * Apache 2.0 license
 */

class cUartDriver {
    int mDeviceAddress = -1;
    int getSerialPortAdddress(const std::string& device);
public:
    explicit cUartDriver(const std::string& device);
    bool init();
    void release();

    void setBaudrate(int baudrate);
    void setParity(enumUartParity parity);
    void setStopbits(int stopbits);
    void setWordlength(int length);

    bool testTransmit();
    uint8_t testReceive();

    uint8_t readByte();
    bool writeByte(uint8_t ch);
};



//eof uartdriver.h