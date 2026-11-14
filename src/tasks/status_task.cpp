#include "../../include/patient_monitor/status_task.h"

namespace patient_monitor {

StatusTask::StatusTask()
    : current_snapshot_(),
      last_ecg_update_us_(0),
      last_spo2_update_us_(0) {}

void StatusTask::updateEcgMetric(const EcgFrame& frame) {
    if (frame.envelope.quality_flags & QualityFlags::VALID) {
        current_snapshot_.heart_rate_valid = true;
        current_snapshot_.heart_rate_bpm = 72;
    } else {
        current_snapshot_.heart_rate_valid = false;
    }
    last_ecg_update_us_ = frame.envelope.monotonic_timestamp_us;
}

void StatusTask::updateSpo2Metric(const Spo2Sample& sample) {
    if ((sample.envelope.quality_flags & QualityFlags::VALID) && sample.confidence_percent >= 70) {
        current_snapshot_.spo2_valid = true;
        current_snapshot_.spo2_percent = 98;
    } else {
        current_snapshot_.spo2_valid = false;
    }
    last_spo2_update_us_ = sample.envelope.monotonic_timestamp_us;
}

void StatusTask::updateBpMetric(const BpRecord& record) {
    if (record.error_code == 0) {
        current_snapshot_.bp_valid = true;
        current_snapshot_.last_systolic_mmhg = record.systolic_mmhg;
        current_snapshot_.last_diastolic_mmhg = record.diastolic_mmhg;
    } else {
        current_snapshot_.bp_valid = false;
    }
}

void StatusTask::updateBatteryMetric(uint16_t mv, uint8_t pct) {
    current_snapshot_.battery_millivolts = mv;
    current_snapshot_.battery_percent = pct;
    current_snapshot_.battery_critical = (pct < 10);
}

void StatusTask::step() {
}

void startStatusTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
