#include "../../include/patient_monitor/bp.h"
#include "../../include/patient_monitor/app.h"
#include "../../include/patient_monitor/events.h"
#include <stdint.h>

namespace patient_monitor {

bool checkAndClearBpButtonTrigger();

enum class BpState : uint8_t {
    IDLE = 0,
    CHECK_SAFETY,
    PREPARING,
    INFLATING_MOCK,
    MEASURING_MOCK,
    DEFLATING,
    COMPLETE,
    FAULT
};

class BpWorker {
public:
    explicit BpWorker(IBpHardware& hardware, ModeCoordinator& coordinator)
        : hardware_(hardware), coordinator_(coordinator), state_(BpState::IDLE), step_counter_(0) {}

    BpState getState() const { return state_; }

    void step() {
        switch (state_) {
            case BpState::IDLE:
                if (checkAndClearBpButtonTrigger()) {
                    state_ = BpState::CHECK_SAFETY;
                }
                break;

            case BpState::CHECK_SAFETY:
                if (hardware_.hasHardwareFault()) {
                    hardware_.emergencyDeflate();
                    state_ = BpState::FAULT;
                    break;
                }
                if (!coordinator_.requestMode(SystemMode::BP_PREPARE)) {
                    state_ = BpState::IDLE;
                    break;
                }
                step_counter_ = 0;
                state_ = BpState::PREPARING;
                break;

            case BpState::PREPARING:
                step_counter_++;
                if (step_counter_ >= 2) {
                    if (coordinator_.requestMode(SystemMode::BP_ACTIVE)) {
                        state_ = BpState::INFLATING_MOCK;
                        step_counter_ = 0;
                    } else {
                        state_ = BpState::FAULT;
                    }
                }
                break;

            case BpState::INFLATING_MOCK:
                if (hardware_.hasHardwareFault()) {
                    hardware_.emergencyDeflate();
                    coordinator_.requestMode(SystemMode::BP_RECOVER);
                    state_ = BpState::FAULT;
                    break;
                }
                step_counter_++;
                if (step_counter_ >= 3) {
                    state_ = BpState::MEASURING_MOCK;
                    step_counter_ = 0;
                }
                break;

            case BpState::MEASURING_MOCK:
                step_counter_++;
                if (step_counter_ >= 3) {
                    state_ = BpState::DEFLATING;
                }
                break;

            case BpState::DEFLATING:
                hardware_.emergencyDeflate();
                state_ = BpState::COMPLETE;
                break;

            case BpState::COMPLETE:
                coordinator_.requestMode(SystemMode::BP_RECOVER);
                coordinator_.requestMode(SystemMode::MONITORING);
                state_ = BpState::IDLE;
                break;

            case BpState::FAULT:
                hardware_.emergencyDeflate();
                coordinator_.requestMode(SystemMode::BP_RECOVER);
                coordinator_.requestMode(SystemMode::MONITORING);
                state_ = BpState::IDLE;
                break;
        }
    }

private:
    IBpHardware& hardware_;
    ModeCoordinator& coordinator_;
    BpState state_;
    uint32_t step_counter_;
};

static MockBpHardware s_mock_bp_hardware;
static BpWorker s_bp_worker(s_mock_bp_hardware, getModeCoordinator());

IBpHardware& getBpHardware() {
    return s_mock_bp_hardware;
}

BpWorker& getBpWorker() {
    return s_bp_worker;
}

void startBpWorkerTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
