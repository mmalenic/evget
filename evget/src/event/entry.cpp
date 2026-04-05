#include "evget/event/entry.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "evget/event/button_action.h"
#include "evget/event/device_type.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

evget::Entry::Entry(EntryType type, const std::vector<std::string>& data, std::vector<std::string> modifiers)
    : type_{type}, data_{data}, modifiers_{std::move(modifiers)} {}

const std::vector<std::string>& evget::Entry::Data() const {
    return data_;
}

const std::vector<std::string>& evget::Entry::Modifiers() const {
    return modifiers_;
}

void evget::Entry::ToNamedRepresentation() {
    constexpr auto kDeviceTypeIndex = detail::kBaseNFields - 1;
    if (data_.size() > kDeviceTypeIndex) {
        data_.at(kDeviceTypeIndex) = FromDevice(FromUnderlying<DeviceType>(data_.at(kDeviceTypeIndex)));
    }

    // button_action position depends on the entry type.
    switch (type_) {
        case EntryType::kMouseClick:
            if (data_.size() >= detail::kMouseClickNFields) {
                constexpr auto kIndex = detail::kMouseClickNFields - 1;
                data_.at(kIndex) = FromButtonAction(FromUnderlying<ButtonAction>(data_.at(kIndex)));
            }
            break;
        case EntryType::kKey:
            if (data_.size() >= detail::kKeyNFields) {
                constexpr auto kIndex = detail::kKeyNFields - 1;
                data_.at(kIndex) = FromButtonAction(FromUnderlying<ButtonAction>(data_.at(kIndex)));
            }
            break;
        default:
            break;
    }

    std::ranges::for_each(modifiers_, [](std::string& modifier) {
        modifier = FromModifierValue(FromUnderlying<ModifierValue>(modifier));
    });
}

evget::EntryWithFields evget::Entry::GetEntryWithFields() const {
    std::vector<std::string> fields;
    switch (this->Type()) {
        case EntryType::kKey:
            fields = {detail::kKeyFields.begin(), detail::kKeyFields.end()};
            break;
        case EntryType::kMouseClick:
            fields = {detail::kMouseClickFields.begin(), detail::kMouseClickFields.end()};
            break;
        case EntryType::kMouseMove:
            fields = {detail::kMouseMoveFields.begin(), detail::kMouseMoveFields.end()};
            break;
        case EntryType::kMouseScroll:
            fields = {detail::kMouseScrollFields.begin(), detail::kMouseScrollFields.end()};
            break;
    }

    return {.type = this->Type(), .fields = fields, .data = Data(), .modifiers = Modifiers()};
}

evget::EntryType evget::Entry::Type() const {
    return type_;
}
