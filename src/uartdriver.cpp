/*! \file uartdriver.cpp */
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/io.h>
#include <termios.h>
#include "uartdriver.h"

// ---------------------------- UART Direct Register Access ----------------------------
// Here there's direct manipulation of UART hardware registers, bypassing the OS
// This is bare-metal territory: changes here affect the device at signal level.
// Proceed only if you understand the 16550 register model and side effects.
// If you need to go deeper than this, get a soldering iron.

#define UART_IER_DEFAULT    0x00

 #define UART_FCR_DEFAULT   (UART_FCR_ENABLE   | \
                            UART_FCR_RX_RESET | \
                            UART_FCR_TX_RESET | \
                            UART_FCR_TRIG_1)

#define UART_MCR_DEFAULT  (UART_MCR_DTR | \
                           UART_MCR_RTS | \
                           UART_MCR_OUT2)

int getRealBaudrate(tcflag_t baudrate){
    static const std::pair<tcflag_t, int> table[] = {
        {B1200,   1200},
        {B1800,   1800},
        {B2400,   2400},
        {B4800,   4800},
        {B9600,   9600},
        {B19200,  19200},
        {B38400,  38400},
        {B57600,  57600},
        {B115200, 115200}
    };

    for (const auto& entry : table){
        if (entry.first == baudrate)
            return entry.second;
    }
    return 115200;
}


cUartDriver::cUartDriver(const std::string& device){
    mDeviceAddress = getSerialPortAdddress(device);
}

int cUartDriver::getSerialPortAdddress(const std::string& device) {
    size_t pos = device.find("ttyS");
    if (pos == std::string::npos) {
        return -1;
    }
    int deviceNumber;
    try {
        deviceNumber = std::stoi(device.substr(pos + 4));
    } catch (...) {
        return -1;
    }
    std::ifstream file("/proc/tty/driver/serial");
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    while (std::getline(file, line)){
        std::istringstream iss(line);
        int number;
        char colon;
        if (!(iss >> number >> colon))
            continue;
        std::string uartField;
        std::string portField;
        std::string irqField;

        if (number != deviceNumber)
            continue;

        iss >> uartField >> portField >> irqField;
        if (uartField.rfind("uart:", 0) != 0)
            continue;
        std::string uart = uartField.substr(5);
        std::string portStr = portField.substr(5);

        if (uart == "unknown"  || uart == "none"){
            return -1;
        }
        int portAddress = std::stoi(portStr, nullptr, 16);
        return portAddress;
    }
    return -1;
}

bool cUartDriver::init() {
    if (mDeviceAddress == -1)
        return false;
    if (ioperm (mDeviceAddress, 8, 1) != 0)
        return false;

    outb (UART_IER_DEFAULT, mDeviceAddress + UART_IER);
    outb (UART_FCR_DEFAULT, mDeviceAddress + UART_FCR);
    outb (UART_MCR_DEFAULT, mDeviceAddress + UART_MCR);

    inb(mDeviceAddress + UART_LSR); //bus sync
    uint8_t iir = inb(mDeviceAddress + UART_IIR);   //read to confirm UART is compatible
    bool ok = ((iir & 0xC0) == 0xC0);
    if (!ok)
        release();
    return ok;
}

void cUartDriver::release() {
    if (mDeviceAddress != -1)
        ioperm (mDeviceAddress, 8, 0);
}

void cUartDriver::setBaudrate(int _baudrate) {
    if (mDeviceAddress == -1)
        return;
    uint8_t lcr;
    _baudrate = getRealBaudrate(_baudrate);
    uint16_t divider = 115200 / _baudrate;
    lcr = inb (mDeviceAddress + UART_LCR);
    outb (lcr | 0x80, mDeviceAddress + UART_LCR);
    outb (divider & 0x00ff, mDeviceAddress + UART_DLL);
    outb (divider >> 8, mDeviceAddress + UART_DLM);
    outb (lcr & 0x7f, mDeviceAddress + UART_LCR);
}

void cUartDriver::setParity(enumUartParity _parity) {
    if (mDeviceAddress == -1)
        return;
    uint8_t lcr;
    lcr  = inb (mDeviceAddress + UART_LCR) & 0xc7;
    lcr |= _parity << 3;
    outb (lcr, mDeviceAddress + UART_LCR);
}

void cUartDriver::setStopbits(int _stopbits) {
    if (mDeviceAddress == -1)
        return;
    if (_stopbits != 1 && _stopbits != 2)
        return;
    uint8_t lcr = inb(mDeviceAddress + UART_LCR) & 0xfb;
    lcr |= (_stopbits - 1) << 2;
    outb (lcr, mDeviceAddress + UART_LCR);
}

void cUartDriver::setWordlength(int _length) {
    if (mDeviceAddress == -1)
        return;
    if (_length < 5 || _length > 8)
        return;
    uint8_t lcr = inb(mDeviceAddress + UART_LCR) & 0xfc;
    lcr |= (_length - 5);
    outb (lcr, mDeviceAddress + UART_LCR);
}

bool cUartDriver::testTransmit() {
    if (mDeviceAddress == -1)
        return false;
    return (inb(mDeviceAddress + UART_LSR) & UART_LSR_THRE) != 0;
}

uint8_t cUartDriver::testReceive() {
    if (mDeviceAddress == -1)
        return 0;
    uint8_t ret = 0;
    uint8_t aux = inb (mDeviceAddress + UART_LSR);
    if (aux & UART_LSR_OE)
        return UART_RX_OVERRUN_ERROR;
    if (aux  & UART_LSR_DR)
        ret |= UART_HAS_RX;
    if (aux & UART_LSR_PE)
        ret |= UART_PARITY_ERROR;
    return ret;
}

uint8_t cUartDriver::readByte () {
    if (mDeviceAddress == -1)
        return 0;
    return inb (mDeviceAddress + UART_RBR);
}

bool cUartDriver::writeByte (uint8_t ch) {
    if (mDeviceAddress == -1)
        return false;
    outb (ch, mDeviceAddress + UART_THR);
    printf(">> %02x\n", ch);
    return true;
}

//eof uartdriver.cpp