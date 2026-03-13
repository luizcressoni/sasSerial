/*! \file ringbuffer.cpp */
#include "ringbuffer.h"


cRingBuffer::cRingBuffer()
: mHead(0), mTail(0), mBuffer{} {}

bool cRingBuffer::push(uint16_t value){
        size_t head = mHead.load(std::memory_order_relaxed);
        size_t next = (head + 1) & MASK;
        if (next == mTail.load(std::memory_order_acquire))        {
                mOverrun.store(true, std::memory_order_relaxed);
                return false;
        }
        mBuffer[head] = value;
        mHead.store(next, std::memory_order_release);
        return true;
}

bool cRingBuffer::pop(uint16_t &value){
        size_t tail = mTail.load(std::memory_order_relaxed);
        if (tail == mHead.load(std::memory_order_acquire))
                return false;
        value = mBuffer[tail];
        mTail.store((tail + 1) & MASK, std::memory_order_release);
        return true;
}

bool cRingBuffer::empty() const {
        return mHead.load(std::memory_order_acquire) ==
               mTail.load(std::memory_order_acquire);
}

bool cRingBuffer::full() const{
        size_t head = mHead.load(std::memory_order_acquire);
        size_t next = (head + 1) & MASK;
        return next == mTail.load(std::memory_order_acquire);
}

bool cRingBuffer::overrun() const{
        return mOverrun.load(std::memory_order_relaxed);
}

void cRingBuffer::clear_overrun(){
        mOverrun.store(false, std::memory_order_relaxed);
}

size_t cRingBuffer::capacity() const{
        return SIZE - 1;
}


//eof ringbuffer.cpp
