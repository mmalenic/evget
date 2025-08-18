#include "evget/event/mouse_click.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

evget::MouseClick& evget::MouseClick::Interval(IntervalType interval) {
    interval_ = interval;
    return *this;
}

evget::MouseClick& evget::MouseClick::Interval(std::optional<IntervalType> interval) {
    interval_ = interval;
    return *this;
}

evget::MouseClick& evget::MouseClick::Timestamp(TimestampType timestamp) {
    timestamp_ = timestamp;
    return *this;
}

evget::MouseClick& evget::MouseClick::Device(evget::DeviceType device) {
    device_ = device;
    return *this;
}

evget::MouseClick& evget::MouseClick::PositionX(double x_pos) {
    position_x_ = x_pos;
    return *this;
}

evget::MouseClick& evget::MouseClick::PositionY(double y_pos) {
    position_y_ = y_pos;
    return *this;
}

evget::MouseClick& evget::MouseClick::Action(evget::ButtonAction action) {
    action_ = action;
    return *this;
}

evget::MouseClick& evget::MouseClick::Button(int button) {
    button_ = button;
    return *this;
}

evget::MouseClick& evget::MouseClick::Name(std::string name) {
    name_ = std::move(name);
    return *this;
}

evget::MouseClick& evget::MouseClick::DeviceName(std::string device_name) {
    device_name_ = std::move(device_name);
    return *this;
}

evget::MouseClick& evget::MouseClick::FocusWindowName(std::string name) {
    focus_window_name_ = std::move(name);
    return *this;
}

evget::MouseClick& evget::MouseClick::FocusWindowPositionX(double x_pos) {
    focus_window_position_x_ = x_pos;
    return *this;
}

evget::MouseClick& evget::MouseClick::FocusWindowPositionY(double y_pos) {
    focus_window_position_y_ = y_pos;
    return *this;
}

evget::MouseClick& evget::MouseClick::FocusWindowWidth(double width) {
    focus_window_width_ = width;
    return *this;
}

evget::MouseClick& evget::MouseClick::FocusWindowHeight(double height) {
    focus_window_height_ = height;
    return *this;
}

evget::MouseClick& evget::MouseClick::Modifier(evget::ModifierValue modifier_value) {
    modifiers_.push_back(ToUnderlying(modifier_value));
    return *this;
}

evget::MouseClick& evget::MouseClick::Screen(int screen) {
    screen_ = screen;
    return *this;
}

evget::Data& evget::MouseClick::Build(Data& data) {
    const Entry entry{
        EntryType::kMouseClick,
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
        },
        modifiers_
    };

    data.AddEntry(entry);

    return data;
}
