#include "../../include/patient_monitor/profile_map.h"

namespace patient_monitor {

const char* ProfileSelectionMap::genderToString(GenderCode code) {
    switch (code) {
        case GenderCode::UNKNOWN:           return "Unknown";
        case GenderCode::FEMALE:            return "Female";
        case GenderCode::MALE:              return "Male";
        case GenderCode::NON_BINARY:        return "Non-Binary";
        case GenderCode::OTHER:             return "Other";
        case GenderCode::PREFER_NOT_TO_SAY: return "Prefer Not To Say";
        default:                            return "Invalid";
    }
}

const char* ProfileSelectionMap::raceToString(RaceCategoryCode code) {
    switch (code) {
        case RaceCategoryCode::UNKNOWN:                         return "Unknown";
        case RaceCategoryCode::AMERICAN_INDIAN_ALASKA_NATIVE:   return "American Indian / Alaska Native";
        case RaceCategoryCode::ASIAN:                           return "Asian";
        case RaceCategoryCode::BLACK_AFRICAN_AMERICAN:          return "Black / African American";
        case RaceCategoryCode::NATIVE_HAWAIIAN_PACIFIC_ISLANDER:return "Native Hawaiian / Pacific Islander";
        case RaceCategoryCode::WHITE:                           return "White";
        case RaceCategoryCode::MULTI_RACIAL:                    return "Multi-Racial";
        case RaceCategoryCode::OTHER:                           return "Other";
        case RaceCategoryCode::PREFER_NOT_TO_SAY:               return "Prefer Not To Say";
        default:                                                return "Invalid";
    }
}

const char* ProfileSelectionMap::ageRangeToString(AgeRangeCode code) {
    switch (code) {
        case AgeRangeCode::UNKNOWN:           return "Unknown";
        case AgeRangeCode::AGE_0_TO_17:       return "0 - 17";
        case AgeRangeCode::AGE_18_TO_29:      return "18 - 29";
        case AgeRangeCode::AGE_30_TO_44:      return "30 - 44";
        case AgeRangeCode::AGE_45_TO_64:      return "45 - 64";
        case AgeRangeCode::AGE_65_TO_79:      return "65 - 79";
        case AgeRangeCode::AGE_80_PLUS:       return "80+";
        case AgeRangeCode::PREFER_NOT_TO_SAY: return "Prefer Not To Say";
        default:                              return "Invalid";
    }
}

size_t ProfileSelectionMap::genderOptionCount() {
    return static_cast<size_t>(GenderCode::COUNT);
}

size_t ProfileSelectionMap::raceOptionCount() {
    return static_cast<size_t>(RaceCategoryCode::COUNT);
}

size_t ProfileSelectionMap::ageRangeOptionCount() {
    return static_cast<size_t>(AgeRangeCode::COUNT);
}

GenderCode ProfileSelectionMap::nextGender(GenderCode current) {
    uint8_t next = static_cast<uint8_t>(current) + 1;
    if (next >= static_cast<uint8_t>(GenderCode::COUNT)) {
        next = 0;
    }
    return static_cast<GenderCode>(next);
}

RaceCategoryCode ProfileSelectionMap::nextRace(RaceCategoryCode current) {
    uint8_t next = static_cast<uint8_t>(current) + 1;
    if (next >= static_cast<uint8_t>(RaceCategoryCode::COUNT)) {
        next = 0;
    }
    return static_cast<RaceCategoryCode>(next);
}

AgeRangeCode ProfileSelectionMap::nextAgeRange(AgeRangeCode current) {
    uint8_t next = static_cast<uint8_t>(current) + 1;
    if (next >= static_cast<uint8_t>(AgeRangeCode::COUNT)) {
        next = 0;
    }
    return static_cast<AgeRangeCode>(next);
}

} // namespace patient_monitor
