/*! \file ringbuffer.h */
#pragma once

#include <atomic>
#include <cstdint>
#include <cstddef>

/**
* @class cRingBuffer
* @brief A simple lock-free ring buffer for uint16_t elements
*/
class cRingBuffer{
    static constexpr size_t SIZE = 1024;
    static constexpr size_t MASK = SIZE - 1;
    alignas(64) std::atomic<size_t> mHead;
    alignas(64) std::atomic<size_t> mTail;
    uint16_t mBuffer[SIZE];
    std::atomic<bool> mOverrun {false};

public:
    cRingBuffer();
    bool push(uint16_t value);
    bool pop(uint16_t &value);

    size_t capacity() const;
    bool empty() const;
    bool full() const;
    bool overrun() const;
    void clear_overrun();
};

//eof ringbuffer.h