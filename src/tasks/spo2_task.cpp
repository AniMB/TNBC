#include "../../include/patient_monitor/spo2_task.h"

namespace patient_monitor {

Spo2Task::Spo2Task()
    : sample_count_(0), latest_sample_() {}

void Spo2Task::step() {
    latest_sample_.envelope = SampleEnvelope(1, 1, 40000 * sample_count_, sample_count_,
                                             QualityFlags::VALID | QualityFlags::SYNTHETIC_MOCK);
    latest_sample_.red_raw = 50000 + (sample_count_ % 100);
    latest_sample_.ir_raw = 75000 + (sample_count_ % 120);
    latest_sample_.confidence_percent = 95;

    sample_count_++;
}

bool Spo2Task::getLatestSample(Spo2Sample& sample) {
    if (sample_count_ == 0) {
        return false;
    }
    sample = latest_sample_;
    return true;
}

void startSpo2Task(void* /*pvParameters*/) {
}

} // namespace patient_monitor
