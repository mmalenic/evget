#include "evget/event/mouse_scroll.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

evget::MouseScroll& evget::MouseScroll::Interval(IntervalType interval) {
    interval_ = interval;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Interval(std::optional<IntervalType> interval) {
    interval_ = interval;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Timestamp(TimestampType timestamp) {
    timestamp_ = timestamp;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Device(evget::DeviceType device) {
    device_ = device;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::PositionX(double x_pos) {
    position_x_ = x_pos;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::PositionY(double y_pos) {
    position_y_ = y_pos;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Vertical(double amount) {
    vertical_ = amount;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Horizontal(double amount) {
    horizontal_ = amount;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::DeviceName(std::string device_name) {
    device_name_ = std::move(device_name);
    return *this;
}

evget::MouseScroll& evget::MouseScroll::FocusWindowName(std::string name) {
    focus_window_name_ = std::move(name);
    return *this;
}

evget::MouseScroll& evget::MouseScroll::FocusWindowPositionX(double x_pos) {
    focus_window_position_x_ = x_pos;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::FocusWindowPositionY(double y_pos) {
    focus_window_position_y_ = y_pos;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::FocusWindowWidth(double width) {
    focus_window_width_ = width;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::FocusWindowHeight(double height) {
    focus_window_height_ = height;
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Modifier(evget::ModifierValue modifier_value) {
    modifiers_.push_back(ToUnderlying(modifier_value));
    return *this;
}

evget::MouseScroll& evget::MouseScroll::Screen(int screen) {
    screen_ = screen;
    return *this;
}

evget::Data& evget::MouseScroll::Build(Data& data) {
    const Entry entry{
        EntryType::kMouseScroll,
        {FromInterval(interval_),
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
         FromDouble(vertical_),
         FromDouble(horizontal_)},
        modifiers_
    };

    data.AddEntry(entry);

    return data;
}
