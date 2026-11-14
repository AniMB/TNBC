#pragma once

#include <stdint.h>
#include "status_snapshot.h"

namespace patient_monitor {

class BleTask {
public:
    BleTask();

    void step();
    bool isConnected() const { return connected_; }
    void setConnected(bool conn) { connected_ = conn; }

    bool transmitStatus(const StatusSnapshot& snapshot);

private:
    bool connected_;
    uint32_t transmit_counter_;
};

void startBleTask(void* pvParameters);

} // namespace patient_monitor
