#pragma once

#include <stdint.h>
#include "patient_session.h"

namespace patient_monitor {

class Spo2Task {
public:
    Spo2Task();

    void step();
    bool getLatestSample(Spo2Sample& sample);

    uint32_t getSampleCount() const { return sample_count_; }

private:
    uint32_t sample_count_;
    Spo2Sample latest_sample_;
};

void startSpo2Task(void* pvParameters);

} // namespace patient_monitor
