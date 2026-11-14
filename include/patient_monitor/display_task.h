#pragma once

#include <stdint.h>
#include "spi_bus.h"
#include "status_snapshot.h"

namespace patient_monitor {

class DisplayTask {
public:
    explicit DisplayTask(ISpiBus& spi_bus);

    void step(const StatusSnapshot& snapshot, bool bp_exclusive_active);
    uint32_t getRenderCount() const { return render_count_; }
    uint32_t getDeferredCount() const { return deferred_count_; }

private:
    ISpiBus& spi_bus_;
    uint32_t render_count_;
    uint32_t deferred_count_;
};

void startDisplayTask(void* pvParameters);

} // namespace patient_monitor
