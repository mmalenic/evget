#include "evgetx11/event_switch.h"

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/device_type.h>

#include <cstddef>
#include <format>
#include <optional>
#include <ranges>
#include <span>
#include <string>

#include "evgetx11/x11.h"

evget::DeviceType evgetx11::EventSwitch::GetDevice(int device_id, int xi2_event_type) const {
    switch (xi2_event_type) {
        case XI_RawKeyPress:
        case XI_RawKeyRelease:
            return evget::DeviceType::kKeyboard;
        default:
            return devices_.at(device_id);
    }
}

bool evgetx11::EventSwitch::HasDevice(int device_id) const {
    return devices_.contains(device_id);
}

void evgetx11::EventSwitch::SetButtonMap(const XIButtonClassInfo& button_info, int device_id) {
    const auto labels = std::span{button_info.labels, static_cast<std::size_t>(button_info.num_buttons)};
    for (const auto [index, label] : std::views::enumerate(labels)) {
        if (label == None) {
            continue;
        }
        // Labels are in device order and raw events have the unmapped button, so both ignore the button mapping.
        auto name = x_wrapper_.get().AtomName(label);
        button_map_[device_id][static_cast<int>(index) + 1] = name ? name.get() : "";
    }
}

void evgetx11::EventSwitch::RefreshDevices(
    int device_id,
    std::optional<int> pointer_id,
    evget::DeviceType device,
    const std::string& name,
    const XIDeviceInfo& info
) {
    if (pointer_id.has_value()) {
        this->pointer_id_ = *pointer_id;
    }

    // The server reuses the ids even if a device is removed, so a refresh needs to replace everything known.
    devices_.insert_or_assign(device_id, device);
    id_to_name_.insert_or_assign(device_id, name);
    button_map_.erase(device_id);

    const XIButtonClassInfo* button_info = nullptr;
    auto classes = std::span(info.classes, info.num_classes);
    for (auto* info_class : classes) {
        if (info_class != nullptr && info_class->type == XIButtonClass) {
            // Reinterpret cast is required by X11.
            // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
            button_info = reinterpret_cast<const XIButtonClassInfo*>(info_class);
            // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
            break;
        }
    }

    if (button_info != nullptr && button_info->num_buttons > 0) {
        SetButtonMap(*button_info, info.deviceid);
    }
}

void evgetx11::EventSwitch::RemoveDevice(int device_id) {
    devices_.erase(device_id);
    id_to_name_.erase(device_id);
    button_map_.erase(device_id);
    device_ids_.Remove(device_id);
}

evgetx11::EventSwitch::EventSwitch(X11Api& x_wrapper) : x_wrapper_{x_wrapper} {}

const std::string& evgetx11::EventSwitch::GetDeviceUuid(int device_id) {
    const auto name = id_to_name_.find(device_id);
    if (name != id_to_name_.end() && name->second == kInjectedDeviceName) {
        return named_device_ids_.UuidDeterministic(std::format("evget:x11:device:{}", name->second));
    }

    return device_ids_.Uuid(device_id);
}

std::optional<std::string> evgetx11::EventSwitch::GetButtonName(int device_id, int button) const {
    const auto device = button_map_.find(device_id);
    if (device == button_map_.end()) {
        return std::nullopt;
    }

    const auto name = device->second.find(button);
    if (name == device->second.end() || name->second.empty()) {
        return std::nullopt;
    }

    return name->second;
}

evgetx11::QueryPointerResult evgetx11::EventSwitch::QueryPointerForDevice() {
    return x_wrapper_.get().QueryPointer(pointer_id_);
}
