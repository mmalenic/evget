#include "evget/event/key.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

evget::Key& evget::Key::Interval(IntervalType interval) {
    interval_ = interval;
    return *this;
}

evget::Key& evget::Key::Interval(std::optional<IntervalType> interval) {
    interval_ = interval;
    return *this;
}

evget::Key& evget::Key::Timestamp(TimestampType timestamp) {
    timestamp_ = timestamp;
    return *this;
}

evget::Key& evget::Key::Device(DeviceType device) {
    device_ = device;
    return *this;
}

evget::Key& evget::Key::PositionX(double x_pos) {
    position_x_ = x_pos;
    return *this;
}

evget::Key& evget::Key::PositionY(double y_pos) {
    position_y_ = y_pos;
    return *this;
}

evget::Key& evget::Key::Action(ButtonAction action) {
    action_ = action;
    return *this;
}

evget::Key& evget::Key::Button(int button) {
    button_ = button;
    return *this;
}

evget::Key& evget::Key::Name(std::string name) {
    name_ = std::move(name);
    return *this;
}

evget::Key& evget::Key::Character(std::string character) {
    character_ = std::move(character);
    return *this;
}

evget::Key& evget::Key::DeviceName(std::string device_name) {
    device_name_ = std::move(device_name);
    return *this;
}

evget::Key& evget::Key::FocusWindowName(std::string name) {
    focus_window_name_ = std::move(name);
    return *this;
}

evget::Key& evget::Key::FocusWindowPositionX(double x_pos) {
    focus_window_position_x_ = x_pos;
    return *this;
}

evget::Key& evget::Key::FocusWindowPositionY(double y_pos) {
    focus_window_position_y_ = y_pos;
    return *this;
}

evget::Key& evget::Key::FocusWindowWidth(double width) {
    focus_window_width_ = width;
    return *this;
}

evget::Key& evget::Key::FocusWindowHeight(double height) {
    focus_window_height_ = height;
    return *this;
}

evget::Key& evget::Key::Screen(int screen) {
    screen_ = screen;
    return *this;
}

evget::Key& evget::Key::Modifier(ModifierValue modifier_value) {
    modifiers_.push_back(ToUnderlying(modifier_value));
    return *this;
}

evget::Data& evget::Key::Build(Data& data) const {
    const Entry entry{
        EntryType::kKey,
        {
            FromInterval(interval_),
            FromTimestamp(timestamp_),
            FromDouble(position_x_),
            FromDouble(position_y_),
            FromString(device_name_),
            FromString(focus_window_name_),
            FromDouble(focus_window_position_x_),
            FromDouble(focus_window_position_y_),
            FromDouble(focus_window_width_),
            FromDouble(focus_window_height_),
            FromInt(screen_),
            ToUnderlyingOptional(device_),
            FromInt(button_),
            FromString(name_),
            ToUnderlyingOptional(action_),
            FromString(character_),
        },
        modifiers_
    };

    data.AddEntry(entry);

    return data;
}
