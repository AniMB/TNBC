#pragma once

#include <stdint.h>
#include "system_mode.h"
#include "profile_map.h"

namespace patient_monitor {

struct StatusSnapshot {
    uint32_t patient_id;
    uint32_t session_id;
    uint64_t snapshot_timestamp_us;
    SystemMode current_mode;

    // Physiological prototype estimates (Nonclinical / Unvalidated prototype metrics)
    uint16_t heart_rate_bpm;
    uint16_t spo2_percent;
    uint16_t last_systolic_mmhg;
    uint16_t last_diastolic_mmhg;
    uint16_t battery_millivolts;
    uint8_t  battery_percent;

    // Validity flags
    bool heart_rate_valid;
    bool spo2_valid;
    bool bp_valid;
    bool battery_critical;
    bool storage_ok;

    // Current metadata
    DemographicProfile active_profile;

    constexpr StatusSnapshot()
        : patient_id(0),
          session_id(0),
          snapshot_timestamp_us(0),
          current_mode(SystemMode::BOOT),
          heart_rate_bpm(0),
          spo2_percent(0),
          last_systolic_mmhg(0),
          last_diastolic_mmhg(0),
          battery_millivolts(0),
          battery_percent(0),
          heart_rate_valid(false),
          spo2_valid(false),
          bp_valid(false),
          battery_critical(false),
          storage_ok(false),
          active_profile() {}
};

} // namespace patient_monitor
