#include "../../include/patient_monitor/ecg_task.h"

namespace patient_monitor {

EcgRingBuffer::EcgRingBuffer(size_t capacity)
    : capacity_(capacity > 0 ? capacity : 256),
      head_(0),
      tail_(0),
      overrun_count_(0) {
    buffer_ = new EcgFrame[capacity_]();
}

EcgRingBuffer::~EcgRingBuffer() {
    delete[] buffer_;
}

bool EcgRingBuffer::push(const EcgFrame& frame) {
    size_t next_head = (head_ + 1) % capacity_;
    if (next_head == tail_) {
        overrun_count_++;
        return false;
    }
    buffer_[head_] = frame;
    head_ = next_head;
    return true;
}

bool EcgRingBuffer::pop(EcgFrame& frame) {
    if (head_ == tail_) {
        return false;
    }
    frame = buffer_[tail_];
    tail_ = (tail_ + 1) % capacity_;
    return true;
}

size_t EcgRingBuffer::size() const {
    if (head_ >= tail_) {
        return head_ - tail_;
    }
    return capacity_ - (tail_ - head_);
}

void EcgRingBuffer::reset() {
    head_ = 0;
    tail_ = 0;
    overrun_count_ = 0;
}

EcgTask::EcgTask(ISpiBus& spi_bus, EcgRingBuffer& ring_buffer)
    : spi_bus_(spi_bus),
      ring_buffer_(ring_buffer),
      sequence_(0),
      drdy_signaled_(false) {}

void EcgTask::onDataReady() {
    drdy_signaled_ = true;
}

void EcgTask::step() {
    if (!drdy_signaled_) {
        return;
    }
    drdy_signaled_ = false;

    if (spi_bus_.lock(10)) {
        uint8_t tx[6] = {0};
        uint8_t rx[6] = {0};
        spi_bus_.transfer(0xFF, tx, rx, sizeof(tx));
        spi_bus_.unlock();

        EcgFrame frame;
        frame.envelope = SampleEnvelope(1, 1, 1000 * sequence_, sequence_, QualityFlags::VALID | QualityFlags::SYNTHETIC_MOCK);
        frame.channel1_raw = (static_cast<int32_t>(rx[0]) << 16) | (static_cast<int32_t>(rx[1]) << 8) | rx[2];
        frame.channel2_raw = (static_cast<int32_t>(rx[3]) << 16) | (static_cast<int32_t>(rx[4]) << 8) | rx[5];
        frame.status_word = 0;

        ring_buffer_.push(frame);
        sequence_++;
    }
}

void startEcgTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
