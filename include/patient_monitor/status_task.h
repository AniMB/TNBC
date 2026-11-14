#pragma once

#include <stdint.h>
#include "status_snapshot.h"
#include "patient_session.h"

namespace patient_monitor {

class StatusTask {
public:
    StatusTask();

    void updateEcgMetric(const EcgFrame& frame);
    void updateSpo2Metric(const Spo2Sample& sample);
    void updateBpMetric(const BpRecord& record);
    void updateBatteryMetric(uint16_t mv, uint8_t pct);

    void step();
    const StatusSnapshot& getSnapshot() const { return current_snapshot_; }

private:
    StatusSnapshot current_snapshot_;
    uint64_t last_ecg_update_us_;
    uint64_t last_spo2_update_us_;
};

void startStatusTask(void* pvParameters);

} // namespace patient_monitor
