#pragma once

#include <stdint.h>

namespace patient_monitor {

struct DeviceEvent {
    enum class Type : uint8_t {
        NONE = 0,
        BP_BUTTON,
        PROFILE_BUTTON,
        LOW_BATTERY,
        SENSOR_ERROR,
        STORAGE_ERROR,
        SESSION_CHANGE,
        BP_COMPLETE,
        BP_FAULT,
        FAULT_OVERPRESSURE
    } type;

    uint64_t timestamp_us;
    uint32_t param1;
    uint32_t param2;

    constexpr DeviceEvent()
        : type(Type::NONE), timestamp_us(0), param1(0), param2(0) {}

    constexpr DeviceEvent(Type t, uint64_t ts, uint32_t p1 = 0, uint32_t p2 = 0)
        : type(t), timestamp_us(ts), param1(p1), param2(p2) {}
};

inline const char* deviceEventTypeToString(DeviceEvent::Type type) {
    switch (type) {
        case DeviceEvent::Type::NONE:               return "NONE";
        case DeviceEvent::Type::BP_BUTTON:          return "BP_BUTTON";
        case DeviceEvent::Type::PROFILE_BUTTON:     return "PROFILE_BUTTON";
        case DeviceEvent::Type::LOW_BATTERY:        return "LOW_BATTERY";
        case DeviceEvent::Type::SENSOR_ERROR:       return "SENSOR_ERROR";
        case DeviceEvent::Type::STORAGE_ERROR:      return "STORAGE_ERROR";
        case DeviceEvent::Type::SESSION_CHANGE:     return "SESSION_CHANGE";
        case DeviceEvent::Type::BP_COMPLETE:        return "BP_COMPLETE";
        case DeviceEvent::Type::BP_FAULT:           return "BP_FAULT";
        case DeviceEvent::Type::FAULT_OVERPRESSURE: return "FAULT_OVERPRESSURE";
        default:                                    return "UNKNOWN";
    }
}

} // namespace patient_monitor
