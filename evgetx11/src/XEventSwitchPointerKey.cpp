#include "evgetx11/XEventSwitchPointerKey.h"

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/device_type.h>
#include <spdlog/spdlog.h>
#include <xorg/xserver-properties.h>

#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XWrapper.h"
#include "evgetx11/XWrapperX11.h"

evgetx11::XEventSwitchPointerKey::XEventSwitchPointerKey(XWrapper& x_wrapper) : x_wrapper_{x_wrapper} {}

void evgetx11::XEventSwitchPointerKey::RefreshDevices(
    int device_id,
    std::optional<int> pointer_id,
    evget::DeviceType device,
    const std::string& name,
    const XIDeviceInfo& info,
    evgetx11::XEventSwitch& x_event_switch
) {
    x_event_switch.RefreshDevices(device_id, pointer_id, device, name, info);

    if (pointer_id.has_value()) {
        this->pointer_id_ = *pointer_id;
    }

    std::vector<const XIScrollClassInfo*> scroll_infos{};
    std::vector<const XIValuatorClassInfo*> valuator_infos{};
    auto classes = std::span(info.classes, info.num_classes);
    for (auto* class_info : classes) {
        if (class_info == nullptr) {
            spdlog::error("unexpected null pointer XIAnyClassInfo");
            continue;
        }

        // Reinterpret cast is required by X11.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        if (class_info->type == XIScrollClass) {
            scroll_infos.emplace_back(reinterpret_cast<const XIScrollClassInfo*>(class_info));
        } else if (class_info->type == XIValuatorClass) {
            valuator_infos.emplace_back(reinterpret_cast<const XIValuatorClassInfo*>(class_info));
        }
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    for (const auto* scroll_info : scroll_infos) {
        scroll_map_[device_id][scroll_info->number] = *scroll_info;
    }
    for (const auto* valuator_info : valuator_infos) {
        auto valuator_name = x_wrapper_.get().AtomName(valuator_info->label);
        if (valuator_name) {
            if (strcmp(valuator_name.get(), AXIS_LABEL_PROP_ABS_X) == 0 ||
                strcmp(valuator_name.get(), AXIS_LABEL_PROP_REL_X) == 0) {
                valuator_x_[device_id] = valuator_info->number;
            } else if (strcmp(valuator_name.get(), AXIS_LABEL_PROP_ABS_Y) == 0 ||
                       strcmp(valuator_name.get(), AXIS_LABEL_PROP_REL_Y) == 0) {
                valuator_y_[device_id] = valuator_info->number;
            }
        }
    }
}

std::map<int, int> evgetx11::XEventSwitchPointerKey::GetValuators(const XIValuatorState& valuator_state) {
    std::map<int, int> valuators{};
    auto* values = valuator_state.values;
    evgetx11::XWrapperX11::OnMasks(valuator_state.mask, valuator_state.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, (*values)++);
    });
    return valuators;
}
