#include "../../include/patient_monitor/power_task.h"

namespace patient_monitor {

PowerTask::PowerTask()
    : battery_mv_(3700),
      battery_percent_(85),
      battery_critical_(false) {}

void PowerTask::step() {
    if (battery_mv_ < 3300) {
        battery_critical_ = true;
        battery_percent_ = 5;
    } else if (battery_mv_ < 3600) {
        battery_critical_ = false;
        battery_percent_ = 25;
    } else {
        battery_critical_ = false;
        battery_percent_ = 85;
    }
}

void startPowerTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
