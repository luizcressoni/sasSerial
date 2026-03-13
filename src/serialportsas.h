/*! \file serialportsas.h */
#pragma once

#include <condition_variable>
#include <string>
#include <mutex>
#include <thread>

#include "uartdriver.h"
#include "ringbuffer.h"

#define WAKEUP_BIT          0x80

/**
 * @class cSerialPortSas
 * @brief Asynchronous serial transport layer for SAS communication.
 *
 * This class implements a higher-level serial transport built on top of
 * the low-level UART driver (`cUartDriver`). It provides buffered,
 * thread-driven communication suitable for SAS (Slot Accounting System)
 * protocols which rely on UART communication with a wake-up bit.
 *
 * The class internally manages:
 *
 * - Separate RX and TX ring buffers
 * - A worker thread responsible for UART polling
 * - Transmission scheduling
 * - Wake-up bit handling required by SAS communication
 *
 * The worker thread continuously executes RX and TX tasks:
 *
 * - **RX task** reads bytes from the UART when available and pushes them
 *   into the receive ring buffer.
 *
 * - **TX task** fetches bytes from the transmit buffer and writes them
 *   to the UART when the transmitter becomes ready.
 *
 * Communication with the application layer is fully decoupled from the
 * hardware through lock-free ring buffers, allowing producers and
 * consumers to operate asynchronously.
 *
 * The SAS protocol requires a special "wake-up bit" (9th bit in UART
 * communication) used to signal the beginning of a message frame. This
 * class allows the caller to mark transmitted bytes with this bit via
 * the `PutChar()` and `PutMsg()` methods.
 *
 * @note
 * This class assumes the underlying UART driver supports wake-up bit
 * transmission, typically implemented via 9-bit UART mode or equivalent
 * hardware mechanisms.
 *
 * @author luiz@cressoni.com.br
 *
 * @note
 * Apache 2.0 license
 */
class cSerialPortSas{
protected:
    cRingBuffer mRxBuffer, mTxBuffer;
    cUartDriver mUartDriver;

    std::atomic<bool> stopFlag{};
    std::condition_variable cv;
    std::mutex cv_m;
    std::thread workerThread;
    void    runThread(bool _start);
    void    task();
    void    rxTask();
    void    txTask();

    uint16_t mTxPendingByte{};
    bool     mTxPendingValid = false;
public:
    explicit cSerialPortSas(const std::string& _device);
    ~cSerialPortSas();
    bool        open();
    void        close();
    bool        putChar(uint8_t _c, bool _wakeupbit = false);
    bool        putMsg(const uint8_t *_buffer, uint16_t _size, bool _wakeupbit = false);
    uint8_t     getChar(uint8_t *_byte);
};

//eof sas_serial.h