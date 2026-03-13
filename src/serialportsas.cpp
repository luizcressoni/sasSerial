/*! \file serialportsas.cpp */

#include "serialportsas.h"
#include <unistd.h>
#include <termios.h>

#define UART_THREAD_US      200
#define WAKEUP_BIT_MASK     0x100

cSerialPortSas::cSerialPortSas(const std::string& _device)
    : mUartDriver(_device){}

cSerialPortSas::~cSerialPortSas() {
    close();
}

bool    cSerialPortSas::open() {
    if (!mUartDriver.init()) {
        return false;
    }

    mUartDriver.setBaudrate(B19200);
    mUartDriver.setParity(UART_SPACE);
    mUartDriver.setStopbits(1);
    mUartDriver.setWordlength(8);

    runThread(true);
    return true;
}

void    cSerialPortSas::close() {
    runThread(false);
    mUartDriver.release();
}

bool    cSerialPortSas::putChar(uint8_t _c, bool _wakeupbit) {
    uint16_t value = _c;
    if (_wakeupbit)
        value |= WAKEUP_BIT_MASK;
    return mTxBuffer.push(value);
}

bool    cSerialPortSas::putMsg(const uint8_t *_buffer, uint16_t _size, bool _wakeupbit) {
    if (_size == 0 || _buffer == nullptr)
        return false;
    for (auto i = 0; i < _size; i++) {
        uint16_t aux = _buffer[i];
        if (i ==0 && _wakeupbit)
            aux |= WAKEUP_BIT_MASK;
        if (!mTxBuffer.push(aux)) {
            return false;
        }
    }
    return true;
}

uint8_t cSerialPortSas::getChar(uint8_t *_byte) {
    if (mRxBuffer.empty() || _byte == nullptr)
        return 0;
    uint16_t value;
    uint8_t ret = 0;
    if (mRxBuffer.pop(value)) {
        *_byte = value & 0xff;
        ret = UART_HAS_RX;
        if (value & WAKEUP_BIT_MASK) {
            ret |= WAKEUP_BIT;
        }
    }
    return ret;
}

void cSerialPortSas::runThread(bool _start) {
    if(_start){
        stopFlag.store(false);
        workerThread = std::thread(&cSerialPortSas::task, this);
    }
    else{
        stopFlag.store(true);
        cv.notify_all();
        if (workerThread.joinable())
            workerThread.join();
    }
}

void cSerialPortSas::task() {
    std::unique_lock<std::mutex> lock(cv_m);
    while (!stopFlag.load()) {
        rxTask();
        txTask();
        if (cv.wait_for(lock, std::chrono::microseconds(UART_THREAD_US), [this] { return stopFlag.load(); })) {
            break;
        }
    }
}

void cSerialPortSas::rxTask() {
    uint8_t lsr = mUartDriver.testReceive();
    while (lsr & UART_HAS_RX) {
        uint16_t byte = mUartDriver.readByte();
        if (lsr & UART_PARITY_ERROR) {
            byte |= WAKEUP_BIT_MASK;
        }
        mRxBuffer.push(byte);
        lsr = mUartDriver.testReceive();
    }
}

void cSerialPortSas::txTask() {
    while (true) {
        if (!mTxPendingValid) {
            uint16_t v;
            if (!mTxBuffer.pop(v))
                return;
            mTxPendingValid = true;
            mTxPendingByte = v;
        }

        if (!mUartDriver.testTransmit()) {
            usleep(5);
            return;
        }

        if (mTxPendingByte & WAKEUP_BIT_MASK) {
            mUartDriver.setParity(UART_MARK);
        }
        mUartDriver.writeByte(mTxPendingByte & 0xff);
        if (mTxPendingByte & WAKEUP_BIT_MASK)
            mUartDriver.setParity(UART_SPACE);
        mTxPendingValid = false;
    }
}

//eof serialport.cpp