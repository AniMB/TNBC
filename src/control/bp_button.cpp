#include <stdint.h>
#include "../../include/patient_monitor/events.h"
#include "../../include/patient_monitor/app.h"

namespace patient_monitor {

static volatile bool s_bp_button_pressed = false;
static volatile uint64_t s_last_bp_isr_timestamp_us = 0;

void handleBpButtonIsr() {
    s_bp_button_pressed = true;
    s_last_bp_isr_timestamp_us = 1;
}

bool checkAndClearBpButtonTrigger() {
    if (s_bp_button_pressed) {
        s_bp_button_pressed = false;
        return true;
    }
    return false;
}

} // namespace patient_monitor
