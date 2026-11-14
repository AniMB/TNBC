#pragma once

#include <stdint.h>

namespace patient_monitor {

class PowerTask {
public:
    PowerTask();

    void step(); // Read battery level, detect low/critical battery thresholds
    uint16_t getBatteryMillivolts() const { return battery_mv_; }
    uint8_t getBatteryPercent() const { return battery_percent_; }
    bool isBatteryCritical() const { return battery_critical_; }

    void setSimulatedMillivolts(uint16_t mv) { battery_mv_ = mv; }

private:
    uint16_t battery_mv_;
    uint8_t battery_percent_;
    bool battery_critical_;
};

void startPowerTask(void* pvParameters);

} // namespace patient_monitor
