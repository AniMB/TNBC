#include "../../include/patient_monitor/display_task.h"

namespace patient_monitor {

DisplayTask::DisplayTask(ISpiBus& spi_bus)
    : spi_bus_(spi_bus), render_count_(0), deferred_count_(0) {}

void DisplayTask::step(const StatusSnapshot& /*snapshot*/, bool bp_exclusive_active) {
    if (bp_exclusive_active) {
        deferred_count_++;
        return;
    }

    if (spi_bus_.lock(20)) {
        uint8_t dummy_tx[4] = {0x00, 0x01, 0x02, 0x03};
        spi_bus_.transfer(0xFF, dummy_tx, nullptr, sizeof(dummy_tx));
        spi_bus_.unlock();
        render_count_++;
    }
}

void startDisplayTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
