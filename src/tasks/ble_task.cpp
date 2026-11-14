#include "../../include/patient_monitor/ble_task.h"

namespace patient_monitor {

BleTask::BleTask()
    : connected_(false), transmit_counter_(0) {}

void BleTask::step() {
}

bool BleTask::transmitStatus(const StatusSnapshot& /*snapshot*/) {
    if (!connected_) {
        return false;
    }
    transmit_counter_++;
    return true;
}

void startBleTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
