#pragma once

#include <stdint.h>
#include "system_mode.h"
#include "events.h"
#include "status_snapshot.h"
#include "spi_bus.h"
#include "bp.h"

namespace patient_monitor {

class ModeCoordinator {
public:
    ModeCoordinator();

    SystemMode getCurrentMode() const { return current_mode_; }

    // Request mode transition. Returns true if permitted.
    bool requestMode(SystemMode requested_mode);

    // Force system into error/shutdown mode
    void tripError(const char* reason = nullptr);

    // Check if system is currently in BP exclusive mode
    bool isBpExclusive() const {
        return current_mode_ == SystemMode::BP_PREPARE ||
               current_mode_ == SystemMode::BP_ACTIVE  ||
               current_mode_ == SystemMode::BP_RECOVER;
    }

private:
    SystemMode current_mode_;
};

// Global application interface
bool initialize();
ModeCoordinator& getModeCoordinator();
ISpiBus& getSharedSpiBus();
IBpHardware& getBpHardware();

// Hardware ISR callbacks (strictly minimal: signal edge or task notification, no lengthy work)
void handleBpButtonIsr();
void handleProfileButtonIsr();
void handleEcgDrdyIsr();

} // namespace patient_monitor
