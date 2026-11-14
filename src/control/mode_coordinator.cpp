#include "../../include/patient_monitor/app.h"

namespace patient_monitor {

static ModeCoordinator s_global_mode_coordinator;

ModeCoordinator& getModeCoordinator() {
    return s_global_mode_coordinator;
}

ModeCoordinator::ModeCoordinator()
    : current_mode_(SystemMode::BOOT) {}

bool ModeCoordinator::requestMode(SystemMode requested_mode) {
    if (current_mode_ == requested_mode) {
        return true;
    }

    if (current_mode_ == SystemMode::ERROR && requested_mode != SystemMode::BOOT && requested_mode != SystemMode::SHUTDOWN) {
        return false;
    }

    bool transition_allowed = false;

    switch (current_mode_) {
        case SystemMode::BOOT:
            if (requested_mode == SystemMode::IDLE || requested_mode == SystemMode::ERROR) {
                transition_allowed = true;
            }
            break;

        case SystemMode::IDLE:
            if (requested_mode == SystemMode::MONITORING ||
                requested_mode == SystemMode::SHUTDOWN   ||
                requested_mode == SystemMode::ERROR) {
                transition_allowed = true;
            }
            break;

        case SystemMode::MONITORING:
            if (requested_mode == SystemMode::BP_PREPARE ||
                requested_mode == SystemMode::IDLE       ||
                requested_mode == SystemMode::ERROR      ||
                requested_mode == SystemMode::SHUTDOWN) {
                transition_allowed = true;
            }
            break;

        case SystemMode::BP_PREPARE:
            if (requested_mode == SystemMode::BP_ACTIVE  ||
                requested_mode == SystemMode::BP_RECOVER ||
                requested_mode == SystemMode::ERROR) {
                transition_allowed = true;
            }
            break;

        case SystemMode::BP_ACTIVE:
            if (requested_mode == SystemMode::BP_RECOVER ||
                requested_mode == SystemMode::ERROR) {
                transition_allowed = true;
            }
            break;

        case SystemMode::BP_RECOVER:
            if (requested_mode == SystemMode::MONITORING ||
                requested_mode == SystemMode::IDLE       ||
                requested_mode == SystemMode::ERROR) {
                transition_allowed = true;
            }
            break;

        case SystemMode::ERROR:
        case SystemMode::SHUTDOWN:
            if (requested_mode == SystemMode::BOOT || requested_mode == SystemMode::SHUTDOWN) {
                transition_allowed = true;
            }
            break;

        default:
            transition_allowed = false;
            break;
    }

    if (transition_allowed) {
        current_mode_ = requested_mode;
        return true;
    }

    return false;
}

void ModeCoordinator::tripError(const char* /*reason*/) {
    current_mode_ = SystemMode::ERROR;
    getBpHardware().emergencyDeflate();
}

} // namespace patient_monitor
