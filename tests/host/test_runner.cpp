#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstring>

#include "../../include/patient_monitor/system_mode.h"
#include "../../include/patient_monitor/events.h"
#include "../../include/patient_monitor/patient_session.h"
#include "../../include/patient_monitor/profile_map.h"
#include "../../include/patient_monitor/status_snapshot.h"
#include "../../include/patient_monitor/spi_bus.h"
#include "../../include/patient_monitor/bp.h"
#include "../../include/patient_monitor/app.h"
#include "../../include/patient_monitor/ecg_task.h"
#include "../../include/patient_monitor/spo2_task.h"
#include "../../include/patient_monitor/status_task.h"
#include "../../include/patient_monitor/power_task.h"
#include "../../include/patient_monitor/sd_task.h"

using namespace patient_monitor;

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] ASSERTION FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            return false; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        g_tests_run++; \
        std::cout << "[RUN ] " << #fn << " ... "; \
        if (fn()) { \
            std::cout << "PASS" << std::endl; \
            g_tests_passed++; \
        } else { \
            std::cout << "FAIL" << std::endl; \
        } \
    } while (0)

bool test_mode_coordinator() {
    ModeCoordinator coord;
    TEST_ASSERT(coord.getCurrentMode() == SystemMode::BOOT, "Initial mode must be BOOT");
    TEST_ASSERT(!coord.requestMode(SystemMode::BP_ACTIVE), "BOOT -> BP_ACTIVE must be rejected");
    TEST_ASSERT(coord.requestMode(SystemMode::IDLE), "BOOT -> IDLE must succeed");
    TEST_ASSERT(coord.requestMode(SystemMode::MONITORING), "IDLE -> MONITORING must succeed");
    TEST_ASSERT(coord.getCurrentMode() == SystemMode::MONITORING, "Current mode must be MONITORING");

    TEST_ASSERT(coord.requestMode(SystemMode::BP_PREPARE), "MONITORING -> BP_PREPARE must succeed");
    TEST_ASSERT(coord.isBpExclusive(), "BP_PREPARE must be reported as BP exclusive");
    TEST_ASSERT(coord.requestMode(SystemMode::BP_ACTIVE), "BP_PREPARE -> BP_ACTIVE must succeed");
    TEST_ASSERT(coord.isBpExclusive(), "BP_ACTIVE must be reported as BP exclusive");
    TEST_ASSERT(coord.requestMode(SystemMode::BP_RECOVER), "BP_ACTIVE -> BP_RECOVER must succeed");
    TEST_ASSERT(coord.requestMode(SystemMode::MONITORING), "BP_RECOVER -> MONITORING must succeed");
    TEST_ASSERT(!coord.isBpExclusive(), "MONITORING must not be BP exclusive");

    coord.tripError("Test fault");
    TEST_ASSERT(coord.getCurrentMode() == SystemMode::ERROR, "tripError must force ERROR mode");
    TEST_ASSERT(!coord.requestMode(SystemMode::MONITORING), "ERROR -> MONITORING must be rejected");

    return true;
}

bool test_ecg_ring_buffer() {
    constexpr size_t TEST_CAP = 8;
    EcgRingBuffer ring(TEST_CAP);

    TEST_ASSERT(ring.capacity() == TEST_CAP, "Capacity check");
    TEST_ASSERT(ring.size() == 0, "Initial size must be 0");
    TEST_ASSERT(ring.getOverrunCount() == 0, "Initial overrun count must be 0");

    for (size_t i = 0; i < TEST_CAP - 1; ++i) {
        EcgFrame f;
        f.envelope = SampleEnvelope(1, 1, 1000 * i, static_cast<uint32_t>(i));
        f.channel1_raw = static_cast<int32_t>(100 + i);
        TEST_ASSERT(ring.push(f), "Push should succeed within capacity");
    }

    TEST_ASSERT(ring.size() == TEST_CAP - 1, "Size should match items pushed");

    EcgFrame overflow_frame;
    overflow_frame.envelope = SampleEnvelope(1, 1, 99999, 999);
    TEST_ASSERT(!ring.push(overflow_frame), "Pushing to full ring buffer must fail");
    TEST_ASSERT(ring.getOverrunCount() == 1, "Overrun count must increment on dropped frame");

    for (size_t i = 0; i < TEST_CAP - 1; ++i) {
        EcgFrame out;
        TEST_ASSERT(ring.pop(out), "Pop should succeed");
        TEST_ASSERT(out.envelope.sequence == i, "FIFO sequence check");
        TEST_ASSERT(out.channel1_raw == static_cast<int32_t>(100 + i), "Data integrity check");
    }

    TEST_ASSERT(ring.size() == 0, "Size should be 0 after popping all");
    EcgFrame empty_pop;
    TEST_ASSERT(!ring.pop(empty_pop), "Pop on empty ring buffer must return false");

    return true;
}

bool test_spo2_task() {
    Spo2Task spo2;
    TEST_ASSERT(spo2.getSampleCount() == 0, "Initial sample count must be 0");

    spo2.step();
    TEST_ASSERT(spo2.getSampleCount() == 1, "Sample count must be 1 after one step");

    Spo2Sample sample;
    TEST_ASSERT(spo2.getLatestSample(sample), "getLatestSample should succeed");
    TEST_ASSERT(sample.envelope.quality_flags & QualityFlags::SYNTHETIC_MOCK, "Must have SYNTHETIC_MOCK flag");
    TEST_ASSERT(sample.confidence_percent >= 90, "Confidence check");

    return true;
}

bool test_status_task() {
    StatusTask status;
    const StatusSnapshot& snap1 = status.getSnapshot();
    TEST_ASSERT(!snap1.heart_rate_valid, "Heart rate should be invalid initially");
    TEST_ASSERT(!snap1.spo2_valid, "SpO2 should be invalid initially");

    EcgFrame f;
    f.envelope = SampleEnvelope(1, 1, 1000, 1, QualityFlags::VALID);
    status.updateEcgMetric(f);
    TEST_ASSERT(status.getSnapshot().heart_rate_valid, "Heart rate should be valid after valid ECG frame");

    Spo2Sample s_invalid;
    s_invalid.envelope = SampleEnvelope(1, 1, 1000, 1, QualityFlags::VALID);
    s_invalid.confidence_percent = 50;
    status.updateSpo2Metric(s_invalid);
    TEST_ASSERT(!status.getSnapshot().spo2_valid, "SpO2 with <70% confidence must be invalid");

    Spo2Sample s_valid;
    s_valid.envelope = SampleEnvelope(1, 1, 2000, 2, QualityFlags::VALID);
    s_valid.confidence_percent = 95;
    status.updateSpo2Metric(s_valid);
    TEST_ASSERT(status.getSnapshot().spo2_valid, "SpO2 with >=70% confidence must be valid");

    return true;
}

bool test_bp_safety_invariants() {
    MockBpHardware bp;
    bp.initialize();
    TEST_ASSERT(!bp.isPumpOn(), "Pump must be OFF after init");
    TEST_ASSERT(bp.isValveOpen(), "Valve must be OPEN (deflated) after init");

    bp.tripFault(true);
    TEST_ASSERT(!bp.setPump(true), "Cannot turn on pump while hardware fault is tripped");
    TEST_ASSERT(!bp.isPumpOn(), "Pump must remain OFF");
    TEST_ASSERT(bp.isValveOpen(), "Valve must remain OPEN");

    bp.tripFault(false);
    TEST_ASSERT(bp.setPump(true), "Set pump ON should succeed when no fault");
    TEST_ASSERT(bp.isPumpOn(), "Pump is ON");

    bp.emergencyDeflate();
    TEST_ASSERT(!bp.isPumpOn(), "Pump must be OFF after emergencyDeflate");
    TEST_ASSERT(bp.isValveOpen(), "Valve must be OPEN after emergencyDeflate");

    return true;
}

bool test_profile_map() {
    TEST_ASSERT(ProfileSelectionMap::MAP_SCHEMA_VERSION == 1, "Schema version must be 1");
    TEST_ASSERT(std::string(ProfileSelectionMap::genderToString(GenderCode::UNKNOWN)) == "Unknown", "Gender UNKNOWN string");
    TEST_ASSERT(std::string(ProfileSelectionMap::genderToString(GenderCode::PREFER_NOT_TO_SAY)) == "Prefer Not To Say", "Gender PREFER_NOT_TO_SAY string");
    TEST_ASSERT(std::string(ProfileSelectionMap::raceToString(RaceCategoryCode::UNKNOWN)) == "Unknown", "Race UNKNOWN string");
    TEST_ASSERT(std::string(ProfileSelectionMap::raceToString(RaceCategoryCode::PREFER_NOT_TO_SAY)) == "Prefer Not To Say", "Race PREFER_NOT_TO_SAY string");
    TEST_ASSERT(std::string(ProfileSelectionMap::ageRangeToString(AgeRangeCode::AGE_18_TO_29)) == "18 - 29", "AgeRange string");
    TEST_ASSERT(std::string(ProfileSelectionMap::ageRangeToString(AgeRangeCode::PREFER_NOT_TO_SAY)) == "Prefer Not To Say", "Age PREFER_NOT_TO_SAY string");

    GenderCode g = GenderCode::UNKNOWN;
    for (size_t i = 0; i < ProfileSelectionMap::genderOptionCount(); ++i) {
        g = ProfileSelectionMap::nextGender(g);
    }
    TEST_ASSERT(g == GenderCode::UNKNOWN, "Cycling through all genders should wrap around to UNKNOWN");

    return true;
}

bool test_profile_button_controller() {
    ProfileButtonController ctrl;
    TEST_ASSERT(ctrl.getState() == UiProfileState::VIEW, "Initial state VIEW");

    // Short press moves to EDIT_FIELD
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getState() == UiProfileState::EDIT_FIELD, "State EDIT_FIELD");
    TEST_ASSERT(ctrl.getActiveField() == ActiveEditField::GENDER, "Active field GENDER");

    // Short press cycles to RACE
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getActiveField() == ActiveEditField::RACE, "Active field RACE");

    // Short press cycles to AGE_RANGE
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getActiveField() == ActiveEditField::AGE_RANGE, "Active field AGE_RANGE");

    // Short press cycles back to GENDER
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getActiveField() == ActiveEditField::GENDER, "Active field back to GENDER");

    // Long press enters SELECT_OPTION for GENDER
    ctrl.onLongPress();
    TEST_ASSERT(ctrl.getState() == UiProfileState::SELECT_OPTION, "State SELECT_OPTION");

    // Short press changes gender from UNKNOWN to FEMALE
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getPendingProfile().gender == GenderCode::FEMALE, "Gender changed to FEMALE");

    // Long press advances to CONFIRM
    ctrl.onLongPress();
    TEST_ASSERT(ctrl.getState() == UiProfileState::CONFIRM, "State CONFIRM");

    // Short press cancels and reverts
    ctrl.onShortPress();
    TEST_ASSERT(ctrl.getState() == UiProfileState::VIEW, "Cancel returns to VIEW");
    TEST_ASSERT(ctrl.getProfile().gender == GenderCode::UNKNOWN, "Profile unchanged after cancel");

    // Edit and confirm:
    ctrl.onShortPress(); // -> EDIT_FIELD
    ctrl.onLongPress();  // -> SELECT_OPTION
    ctrl.onShortPress(); // GENDER -> FEMALE
    ctrl.onLongPress();  // -> CONFIRM
    ctrl.onLongPress();  // Commit! -> VIEW
    TEST_ASSERT(ctrl.getState() == UiProfileState::VIEW, "Committed returns to VIEW");
    TEST_ASSERT(ctrl.getProfile().gender == GenderCode::FEMALE, "Profile updated to FEMALE");
    TEST_ASSERT(ctrl.getProfile().version == 2, "Profile version incremented to 2");

    return true;
}

class TestSdStorageBackend : public ISdStorageBackend {
public:
    bool isCardPresent() override { return true; }
    bool openSessionDirectory(uint32_t /*patient_id*/, uint32_t /*session_id*/) override { return true; }
    bool appendRecord(StreamType /*stream*/, const uint8_t* /*data*/, size_t /*length*/) override {
        records_written++;
        return true;
    }
    bool sync() override { return true; }
    size_t getCurrentFileOffset(StreamType /*stream*/) override { return 0; }

    size_t records_written = 0;
};

bool test_sd_task_session_isolation() {
    TestSdStorageBackend backend;
    SdTask sd(backend);

    TEST_ASSERT(sd.getActivePatientId() == 1, "Initial patient ID should be 1");
    TEST_ASSERT(sd.getActiveSessionId() == 1, "Initial session ID should be 1");

    EcgFrame valid_frame;
    valid_frame.envelope = SampleEnvelope(1, 1, 1000, 1, QualityFlags::VALID);
    valid_frame.channel1_raw = 12345;
    TEST_ASSERT(sd.writeEcgBatch(&valid_frame, 1), "Writing matching session batch must succeed");
    TEST_ASSERT(sd.getWriteSuccessCount() == 1, "Write success count should be 1");
    TEST_ASSERT(sd.getMisattributionDrops() == 0, "No drops for matching session");

    EcgFrame rogue_frame;
    rogue_frame.envelope = SampleEnvelope(2, 1, 2000, 2, QualityFlags::VALID);
    TEST_ASSERT(!sd.writeEcgBatch(&rogue_frame, 1), "Writing mismatched patient ID must be strictly rejected");
    TEST_ASSERT(sd.getMisattributionDrops() == 1, "Misattribution drop counter must increment");

    TEST_ASSERT(sd.switchSession(2, 5), "Switching session should succeed");
    TEST_ASSERT(sd.getActivePatientId() == 2, "Active patient ID should be 2");
    TEST_ASSERT(sd.getActiveSessionId() == 5, "Active session ID should be 5");

    TEST_ASSERT(!sd.writeEcgBatch(&rogue_frame, 1), "Session ID mismatch must be rejected");
    TEST_ASSERT(sd.getMisattributionDrops() == 2, "Misattribution drop counter should be 2");

    EcgFrame new_session_frame;
    new_session_frame.envelope = SampleEnvelope(2, 5, 3000, 1, QualityFlags::VALID);
    TEST_ASSERT(sd.writeEcgBatch(&new_session_frame, 1), "Writing matching new session batch must succeed");
    TEST_ASSERT(sd.getWriteSuccessCount() == 2, "Write success count should be 2");

    return true;
}

bool test_power_task() {
    PowerTask power;
    power.setSimulatedMillivolts(3800);
    power.step();
    TEST_ASSERT(!power.isBatteryCritical(), "3800mV is not critical");
    TEST_ASSERT(power.getBatteryPercent() == 85, "Percent check");

    power.setSimulatedMillivolts(3200);
    power.step();
    TEST_ASSERT(power.isBatteryCritical(), "3200mV must trigger critical battery");
    TEST_ASSERT(power.getBatteryPercent() == 5, "Percent should reflect critical low");

    return true;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Running Patient Monitor Host Unit Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    RUN_TEST(test_mode_coordinator);
    RUN_TEST(test_ecg_ring_buffer);
    RUN_TEST(test_spo2_task);
    RUN_TEST(test_status_task);
    RUN_TEST(test_bp_safety_invariants);
    RUN_TEST(test_profile_map);
    RUN_TEST(test_profile_button_controller);
    RUN_TEST(test_sd_task_session_isolation);
    RUN_TEST(test_power_task);

    std::cout << "==========================================================" << std::endl;
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " tests passed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
