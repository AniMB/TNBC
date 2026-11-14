#pragma once

#include <stdint.h>
#include <stddef.h>
#include "patient_session.h"
#include "spi_bus.h"

namespace patient_monitor {

class EcgRingBuffer {
public:
    explicit EcgRingBuffer(size_t capacity = 256);
    ~EcgRingBuffer();

    EcgRingBuffer(const EcgRingBuffer&) = delete;
    EcgRingBuffer& operator=(const EcgRingBuffer&) = delete;

    bool push(const EcgFrame& frame);
    bool pop(EcgFrame& frame);
    size_t size() const;
    size_t capacity() const { return capacity_; }
    uint32_t getOverrunCount() const { return overrun_count_; }
    void reset();

private:
    EcgFrame* buffer_;
    size_t capacity_;
    volatile size_t head_;
    volatile size_t tail_;
    volatile uint32_t overrun_count_;
};

class EcgTask {
public:
    EcgTask(ISpiBus& spi_bus, EcgRingBuffer& ring_buffer);

    void step();
    void onDataReady();

    uint32_t getFrameSequence() const { return sequence_; }
    uint32_t getDroppedFrames() const { return ring_buffer_.getOverrunCount(); }

private:
    ISpiBus& spi_bus_;
    EcgRingBuffer& ring_buffer_;
    uint32_t sequence_;
    volatile bool drdy_signaled_;
};

void startEcgTask(void* pvParameters);

} // namespace patient_monitor
