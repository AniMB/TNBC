#pragma once

#include <stdint.h>

namespace patient_monitor {

/**
 * System Modes for Patient Monitor
 * The system executes in distinct operational modes coordinated by ModeCoordinator.
 */
enum class SystemMode : uint8_t {
    BOOT = 0,        // Hardware and subsystem initialization
    IDLE,            // System initialized, waiting for session start or idle
    MONITORING,      // Normal continuous multi-sensor monitoring
    BP_PREPARE,      // Preparing for blood pressure measurement (quiescing non-critical bus activity)
    BP_ACTIVE,       // Active BP measurement; BP worker owns coordinated execution mode
    BP_RECOVER,      // Restoring regular monitoring tasks and post-BP logging
    ERROR,           // Fault condition or safety trip
    SHUTDOWN         // Safe power-down or sleep
};

inline const char* systemModeToString(SystemMode mode) {
    switch (mode) {
        case SystemMode::BOOT:       return "BOOT";
        case SystemMode::IDLE:       return "IDLE";
        case SystemMode::MONITORING: return "MONITORING";
        case SystemMode::BP_PREPARE: return "BP_PREPARE";
        case SystemMode::BP_ACTIVE:  return "BP_ACTIVE";
        case SystemMode::BP_RECOVER: return "BP_RECOVER";
        case SystemMode::ERROR:      return "ERROR";
        case SystemMode::SHUTDOWN:   return "SHUTDOWN";
        default:                     return "UNKNOWN";
    }
}

} // namespace patient_monitor
