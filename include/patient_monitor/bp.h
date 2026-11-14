#pragma once

#include <stdint.h>
#include "patient_session.h"

namespace patient_monitor {

class IBpHardware {
public:
    virtual ~IBpHardware() = default;

    virtual bool initialize() = 0;
    virtual int16_t readPressureMmhg() = 0;
    virtual bool setPump(bool enable) = 0;
    virtual bool setValve(bool open) = 0;
    virtual bool hasHardwareFault() = 0;
    virtual void emergencyDeflate() = 0;
};

class MockBpHardware : public IBpHardware {
public:
    MockBpHardware()
        : pump_on_(false), valve_open_(true), simulated_pressure_(0), fault_tripped_(false) {}

    bool initialize() override {
        emergencyDeflate();
        return true;
    }

    int16_t readPressureMmhg() override {
        return simulated_pressure_;
    }

    bool setPump(bool enable) override {
        if (fault_tripped_ && enable) {
            return false;
        }
        pump_on_ = enable;
        return true;
    }

    bool setValve(bool open) override {
        valve_open_ = open;
        return true;
    }

    bool hasHardwareFault() override {
        return fault_tripped_;
    }

    void emergencyDeflate() override {
        pump_on_ = false;
        valve_open_ = true;
    }

    void setSimulatedPressure(int16_t mmhg) { simulated_pressure_ = mmhg; }
    void tripFault(bool tripped) {
        fault_tripped_ = tripped;
        if (tripped) {
            emergencyDeflate();
        }
    }
    bool isPumpOn() const { return pump_on_; }
    bool isValveOpen() const { return valve_open_; }

private:
    bool pump_on_;
    bool valve_open_;
    int16_t simulated_pressure_;
    bool fault_tripped_;
};

} // namespace patient_monitor
