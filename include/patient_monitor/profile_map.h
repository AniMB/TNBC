#pragma once

#include <stdint.h>
#include <stddef.h>

namespace patient_monitor {

enum class GenderCode : uint8_t {
    UNKNOWN = 0,
    FEMALE,
    MALE,
    NON_BINARY,
    OTHER,
    PREFER_NOT_TO_SAY,
    COUNT
};

enum class RaceCategoryCode : uint8_t {
    UNKNOWN = 0,
    AMERICAN_INDIAN_ALASKA_NATIVE,
    ASIAN,
    BLACK_AFRICAN_AMERICAN,
    NATIVE_HAWAIIAN_PACIFIC_ISLANDER,
    WHITE,
    MULTI_RACIAL,
    OTHER,
    PREFER_NOT_TO_SAY,
    COUNT
};

enum class AgeRangeCode : uint8_t {
    UNKNOWN = 0,
    AGE_0_TO_17,
    AGE_18_TO_29,
    AGE_30_TO_44,
    AGE_45_TO_64,
    AGE_65_TO_79,
    AGE_80_PLUS,
    PREFER_NOT_TO_SAY,
    COUNT
};

struct DemographicProfile {
    uint32_t patient_id;
    GenderCode gender;
    RaceCategoryCode race;
    AgeRangeCode age_range;
    uint32_t version;

    constexpr DemographicProfile()
        : patient_id(0),
          gender(GenderCode::UNKNOWN),
          race(RaceCategoryCode::UNKNOWN),
          age_range(AgeRangeCode::UNKNOWN),
          version(1) {}

    constexpr DemographicProfile(uint32_t pid, GenderCode g, RaceCategoryCode r, AgeRangeCode a, uint32_t ver = 1)
        : patient_id(pid), gender(g), race(r), age_range(a), version(ver) {}
};

class ProfileSelectionMap {
public:
    static constexpr uint32_t MAP_SCHEMA_VERSION = 1;

    static const char* genderToString(GenderCode code);
    static const char* raceToString(RaceCategoryCode code);
    static const char* ageRangeToString(AgeRangeCode code);

    static size_t genderOptionCount();
    static size_t raceOptionCount();
    static size_t ageRangeOptionCount();

    static GenderCode nextGender(GenderCode current);
    static RaceCategoryCode nextRace(RaceCategoryCode current);
    static AgeRangeCode nextAgeRange(AgeRangeCode current);
};

enum class UiProfileState : uint8_t {
    VIEW = 0,
    EDIT_FIELD,
    SELECT_OPTION,
    CONFIRM
};

enum class ActiveEditField : uint8_t {
    GENDER = 0,
    RACE,
    AGE_RANGE
};

class ProfileButtonController {
public:
    ProfileButtonController();

    UiProfileState getState() const { return state_; }
    ActiveEditField getActiveField() const { return active_field_; }
    const DemographicProfile& getProfile() const { return profile_; }
    const DemographicProfile& getPendingProfile() const { return pending_profile_; }

    void onShortPress();
    void onLongPress();

private:
    UiProfileState state_;
    ActiveEditField active_field_;
    DemographicProfile profile_;
    DemographicProfile pending_profile_;
};

ProfileButtonController& getProfileButtonController();

} // namespace patient_monitor
