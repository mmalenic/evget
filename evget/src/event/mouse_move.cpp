#include "evget/event/mouse_move.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

evget::MouseMove& evget::MouseMove::Interval(IntervalType interval) {
    interval_ = interval;
    return *this;
}

evget::MouseMove& evget::MouseMove::Interval(std::optional<IntervalType> interval) {
    interval_ = interval;
    return *this;
}

evget::MouseMove& evget::MouseMove::Timestamp(TimestampType timestamp) {
    timestamp_ = timestamp;
    return *this;
}

evget::MouseMove& evget::MouseMove::Device(DeviceType device) {
    device_ = device;
    return *this;
}

evget::MouseMove& evget::MouseMove::PositionX(double x_pos) {
    position_x_ = x_pos;
    return *this;
}

evget::MouseMove& evget::MouseMove::PositionY(double y_pos) {
    position_y_ = y_pos;
    return *this;
}

evget::MouseMove& evget::MouseMove::DeviceName(std::string device_name) {
    device_name_ = std::move(device_name);
    return *this;
}

evget::MouseMove& evget::MouseMove::FocusWindowName(std::string name) {
    focus_window_name_ = std::move(name);
    return *this;
}

evget::MouseMove& evget::MouseMove::FocusWindowPositionX(double x_pos) {
    focus_window_position_x_ = x_pos;
    return *this;
}

evget::MouseMove& evget::MouseMove::FocusWindowPositionY(double y_pos) {
    focus_window_position_y_ = y_pos;
    return *this;
}

evget::MouseMove& evget::MouseMove::FocusWindowWidth(double width) {
    focus_window_width_ = width;
    return *this;
}

evget::MouseMove& evget::MouseMove::FocusWindowHeight(double height) {
    focus_window_height_ = height;
    return *this;
}

evget::MouseMove& evget::MouseMove::Screen(int screen) {
    screen_ = screen;
    return *this;
}

evget::MouseMove& evget::MouseMove::Modifier(ModifierValue modifier_value) {
    modifiers_.push_back(ToUnderlying(modifier_value));
    return *this;
}

evget::Data& evget::MouseMove::Build(Data& data) const {
    const Entry entry{
        EntryType::kMouseMove,
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
         ToUnderlyingOptional(device_)},
        modifiers_
    };

    data.AddEntry(entry);

    return data;
}
