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
    if (data_.size() >= detail::kMouseMoveNFields) {
        data_.at(detail::kMouseMoveNFields - 1) =
            FromDevice(FromUnderlying<DeviceType>(data_.at(detail::kMouseMoveNFields - 1)));
    }
    if (data_.size() >= detail::kMouseClickNFields) {
        data_.at(detail::kMouseClickNFields - 1) =
            FromButtonAction(FromUnderlying<ButtonAction>(data_.at(detail::kMouseClickNFields - 1)));
    }

    std::ranges::for_each(modifiers_.begin(), modifiers_.end(), [](std::string& modifier) {
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
