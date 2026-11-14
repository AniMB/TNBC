#include "../../include/patient_monitor/profile_map.h"

namespace patient_monitor {

ProfileButtonController::ProfileButtonController()
    : state_(UiProfileState::VIEW),
      active_field_(ActiveEditField::GENDER),
      profile_(),
      pending_profile_() {}

void ProfileButtonController::onShortPress() {
    switch (state_) {
        case UiProfileState::VIEW:
            state_ = UiProfileState::EDIT_FIELD;
            pending_profile_ = profile_;
            break;

        case UiProfileState::EDIT_FIELD:
            if (active_field_ == ActiveEditField::GENDER) {
                active_field_ = ActiveEditField::RACE;
            } else if (active_field_ == ActiveEditField::RACE) {
                active_field_ = ActiveEditField::AGE_RANGE;
            } else {
                active_field_ = ActiveEditField::GENDER;
            }
            break;

        case UiProfileState::SELECT_OPTION:
            if (active_field_ == ActiveEditField::GENDER) {
                pending_profile_.gender = ProfileSelectionMap::nextGender(pending_profile_.gender);
            } else if (active_field_ == ActiveEditField::RACE) {
                pending_profile_.race = ProfileSelectionMap::nextRace(pending_profile_.race);
            } else if (active_field_ == ActiveEditField::AGE_RANGE) {
                pending_profile_.age_range = ProfileSelectionMap::nextAgeRange(pending_profile_.age_range);
            }
            break;

        case UiProfileState::CONFIRM:
            state_ = UiProfileState::VIEW;
            pending_profile_ = profile_;
            break;
    }
}

void ProfileButtonController::onLongPress() {
    switch (state_) {
        case UiProfileState::VIEW:
            break;

        case UiProfileState::EDIT_FIELD:
            state_ = UiProfileState::SELECT_OPTION;
            break;

        case UiProfileState::SELECT_OPTION:
            state_ = UiProfileState::CONFIRM;
            break;

        case UiProfileState::CONFIRM:
            profile_ = pending_profile_;
            profile_.version++;
            state_ = UiProfileState::VIEW;
            break;
    }
}

static ProfileButtonController s_profile_controller;

ProfileButtonController& getProfileButtonController() {
    return s_profile_controller;
}

} // namespace patient_monitor
