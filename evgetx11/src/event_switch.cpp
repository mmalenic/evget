#include "evgetx11/event_switch.h"

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/device_type.h>

#include <optional>
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
    auto map = x_wrapper_.get().GetDeviceButtonMapping(device_id, button_info.num_buttons);
    if (map != nullptr) {
        auto labels = std::span(button_info.labels, button_info.num_buttons);
        for (auto i = 0; i < labels.size(); i++) {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            if (labels[i] != 0U) {
                auto name = x_wrapper_.get().AtomName(labels[i]);
                if (name) {
                    button_map_[device_id][map[i]] = name.get();
                } else {
                    button_map_[device_id][map[i]] = "";
                }
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }
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
    devices_.emplace(device_id, device);
    id_to_name_.emplace(device_id, name);

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

evgetx11::EventSwitch::EventSwitch(X11Api& x_wrapper) : x_wrapper_{x_wrapper} {}

const std::string& evgetx11::EventSwitch::GetDeviceUuid(int device_id) {
    return device_ids_.Uuid(device_id);
}

const std::string& evgetx11::EventSwitch::GetButtonName(int device_id, int button) const {
    return button_map_.at(device_id).at(button);
}

evgetx11::QueryPointerResult evgetx11::EventSwitch::QueryPointerForDevice() {
    return x_wrapper_.get().QueryPointer(pointer_id_);
}
