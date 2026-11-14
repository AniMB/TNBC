#pragma once

#include <stdint.h>
#include <stddef.h>

namespace patient_monitor {

/**
 * Interface for SPI Bus Arbitration and safe shared bus access.
 * Enforces single-owner serialization across ECG, Display, and SD Card.
 */
class ISpiBus {
public:
    virtual ~ISpiBus() = default;

    // Acquire exclusive access to the shared SPI bus (blocks up to timeout_ms)
    virtual bool lock(uint32_t timeout_ms = 100) = 0;

    // Release exclusive access to the shared SPI bus
    virtual void unlock() = 0;

    // Transfer buffer over SPI with specified chip select
    virtual bool transfer(uint8_t cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len) = 0;
};

// No-op / mock SPI bus for host tests and safe unconfigured hardware state
class MockSpiBus : public ISpiBus {
public:
    MockSpiBus() : locked_(false), transfer_count_(0) {}

    bool lock(uint32_t /*timeout_ms*/ = 100) override {
        if (locked_) return false; // Re-entrancy guard
        locked_ = true;
        return true;
    }

    void unlock() override {
        locked_ = false;
    }

    bool transfer(uint8_t /*cs_pin*/, const uint8_t* tx_data, uint8_t* rx_data, size_t len) override {
        if (!locked_) return false;
        transfer_count_++;
        if (rx_data && tx_data) {
            for (size_t i = 0; i < len; ++i) {
                rx_data[i] = tx_data[i];
            }
        }
        return true;
    }

    bool isLocked() const { return locked_; }
    size_t getTransferCount() const { return transfer_count_; }

private:
    bool locked_;
    size_t transfer_count_;
};

} // namespace patient_monitor
