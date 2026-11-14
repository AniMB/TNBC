#pragma once

#include <stdint.h>
#include <stddef.h>

namespace patient_monitor {

namespace QualityFlags {
    constexpr uint16_t VALID             = 0x0001;
    constexpr uint16_t CLIPPED           = 0x0002;
    constexpr uint16_t NOISY             = 0x0004;
    constexpr uint16_t LEAD_OFF          = 0x0008;
    constexpr uint16_t MOTION_ARTIFACT   = 0x0010;
    constexpr uint16_t OVERRUN_LOSS      = 0x0020;
    constexpr uint16_t SYNTHETIC_MOCK    = 0x8000;
}

struct SampleEnvelope {
    uint32_t patient_id{0};
    uint32_t session_id{0};
    uint64_t monotonic_timestamp_us{0};
    uint32_t sequence{0};
    uint16_t quality_flags{0};
    uint16_t reserved{0};

    constexpr SampleEnvelope() = default;

    constexpr SampleEnvelope(uint32_t pid, uint32_t sid, uint64_t ts, uint32_t seq, uint16_t qf = QualityFlags::VALID)
        : patient_id(pid), session_id(sid), monotonic_timestamp_us(ts),
          sequence(seq), quality_flags(qf), reserved(0) {}
};

struct EcgFrame {
    SampleEnvelope envelope{};
    int32_t channel1_raw{0};
    int32_t channel2_raw{0};
    uint32_t status_word{0};
};

struct Spo2Sample {
    SampleEnvelope envelope{};
    uint32_t red_raw{0};
    uint32_t ir_raw{0};
    uint16_t confidence_percent{0};
    uint16_t reserved{0};
};

struct BpRecord {
    SampleEnvelope envelope{};
    uint16_t systolic_mmhg{0};
    uint16_t diastolic_mmhg{0};
    uint16_t mean_arterial_pressure_mmhg{0};
    uint16_t pulse_bpm{0};
    uint8_t  error_code{0};
    uint8_t  cuff_cycles{0};
    uint16_t reserved{0};
};

} // namespace patient_monitor
