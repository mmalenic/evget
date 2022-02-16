// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <spdlog/spdlog.h>
#include <X11/extensions/XInput.h>
#include <boost/numeric/conversion/cast.hpp>
#include "evget/EventTransformerLinux.h"
#include "evget/UnsupportedOperationException.h"
#include "evget/Event/MouseClick.h"
#include "evget/Event/Field.h"
#include "evget/Event/MouseWheel.h"

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    if (event.hasData()) {
        auto type = event.getEventType();

        switch (type) {
        case XI_ButtonPress:
            return buttonEvent(event.viewData<XIDeviceEvent>(), getTime(event), Event::Button::Action::Press);
        case XI_ButtonRelease:
            return buttonEvent(event.viewData<XIDeviceEvent>(), getTime(event), Event::Button::Action::Release);
        case XI_KeyPress:break;
        case XI_KeyRelease:break;
        case XI_Motion:break;
        case XI_RawMotion:break;
#if defined XI_TouchBegin && defined XI_TouchUpdate && defined XI_TouchEnd
        case XI_TouchBegin:break;
        case XI_TouchUpdate:break;
        case XI_TouchEnd:break;
#endif
        case XI_HierarchyChanged:
        case XI_DeviceChanged:
            refreshDeviceIds();
            break;
        default:
            spdlog::info(
                "Unsupported event with type '{}' passed to event transformer.",
                type
            );
            return {};
        }
    }
    return {};
}

std::chrono::nanoseconds evget::EventTransformerLinux::getTime(evget::XInputEvent& event) {
    if (!start.has_value()) {
        start = event.getTimestamp();
    }
    return event.getTimestamp() - *start;
}

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::buttonEvent(const XIDeviceEvent& event, std::chrono::nanoseconds time, Event::Button::Action action) {
    if (!devices.contains(event.deviceid)) {
        return {};
    }

    Event::MouseClick::MouseClickBuilder builder{};
    builder.time(time).device(devices[event.deviceid]).positionX(event.root_x).positionY(event.root_y).action(action).button(event.detail).name(buttonMap[event.deviceid][event.detail]);
    return Event::TableData::TableDataBuilder{}.genericData(builder.build()).systemData(createSystemData(event, "MouseClickSystemData")).build();
}

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::scrollEvent(
    evget::XInputEvent& event
) {
    Event::MouseWheel::MouseWheelBuilder builder;
    return std::unique_ptr<Event::TableData>();
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemData(const XIDeviceEvent& event, const std::string& name, std::initializer_list<int> excludeValuators) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", idToName[event.deviceid]));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));
    fields.emplace_back(std::make_unique<Event::Field>("Flags", formatValue(event.flags)));

    //auto buttonState = getMask(event.buttons.mask_len, event.buttons.mask);
    //fields.emplace_back(std::make_unique<Event::Field>("ButtonState", formatValue(buttonState)));

    fields.emplace_back(std::make_unique<Event::Field>("Valuators", createValuatorEntries(event, excludeValuators)));

    fields.emplace_back(std::make_unique<Event::Field>("ModifiersBase", formatValue(event.mods.base)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersEffective", formatValue(event.mods.effective)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersLatched", formatValue(event.mods.latched)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersLocked", formatValue(event.mods.locked)));

    fields.emplace_back(std::make_unique<Event::Field>("GroupBase", formatValue(event.group.base)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupEffective", formatValue(event.group.effective)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupLatched", formatValue(event.group.latched)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupLocked", formatValue(event.group.locked)));

    return std::make_unique<Event::Data>(name, std::move(fields));
}

Event::AbstractField::Entries evget::EventTransformerLinux::createValuatorEntries(const XIDeviceEvent& event, std::initializer_list<int> exclude) {
    std::vector<std::unique_ptr<Event::AbstractData>> data{};

    auto* values = event.valuators.values;
    for (int i = 0; i < event.valuators.mask_len * 8; i++) {
        if (XIMaskIsSet(event.valuators.mask, i) && std::find(exclude.begin(), exclude.end(), i) == exclude.end()) {
            std::vector<std::unique_ptr<Event::AbstractField>> fields{};
            fields.emplace_back(std::make_unique<Event::Field>("Valuator", std::to_string(i)));
            fields.emplace_back(std::make_unique<Event::Field>("Value", std::to_string(*values++)));

            data.emplace_back(std::make_unique<Event::Data>("Valuators", std::move(fields)));
        }
    }

    return data;
}

std::string evget::EventTransformerLinux::formatValue(int value) {
    return value != 0 ? std::to_string(value) : "";
}

std::string evget::EventTransformerLinux::formatValue(std::vector<int> values) {
    return !values.empty() ? fmt::format("[{}]", fmt::join(values, ", ")) : "";
}

void evget::EventTransformerLinux::refreshDeviceIds() {
    int nDevices;
    int xi2NDevices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>(XListInputDevices(&display.get(), &nDevices),
        XFreeDeviceList);
    auto xi2Info = std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>(XIQueryDevice(&display.get(), XIAllDevices, &xi2NDevices),
        XIFreeDeviceInfo);

    if (nDevices != xi2NDevices) {
        spdlog::warn("Devices with ids greater than 127 found. Set the device of these devices manually if their use is required.");
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2Devices{};
    for (int i = 0; i < xi2NDevices; i++) {
        xi2Devices.emplace(xi2Info[i].deviceid, xi2Info[i]);
    }

    for (int i = 0; i < nDevices; i++) {
        const auto& device = info[i];
        int id = boost::numeric_cast<int>(device.id);

        if (!xi2Devices.contains(id)) {
            throw evget::UnsupportedOperationException{"Device id from XDeviceInfo not found in XIDeviceInfo."};
        }
        const auto& xi2Device = xi2Devices.at(id).get();

        if (xi2Device.enabled && device.type != None && (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice)) {
            auto type = std::unique_ptr<char[], decltype(&XFree)>(XGetAtomName(&display.get(), device.type), XFree);

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                devices.emplace(id, Event::Common::Device::Mouse);
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                devices.emplace(id, Event::Common::Device::Keyboard);
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                devices.emplace(id, Event::Common::Device::Touchscreen);
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                devices.emplace(id, Event::Common::Device::Touchpad);
            } else {
                spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
                continue;
            }

            idToName.emplace(id, device.name);
            setInfo(xi2Devices.at(id).get());
        }
    }
}

void evget::EventTransformerLinux::setInfo(const XIDeviceInfo& info) {
    const XIButtonClassInfo* buttonInfo = nullptr;
    std::vector<const XIScrollClassInfo*> scrollInfos{};
    for (int i = 0; i < info.num_classes; i++) {
        const auto* classInfo = info.classes[i];

        if (classInfo->type == ButtonClass) {
            buttonInfo = reinterpret_cast<const XIButtonClassInfo*>(classInfo);
        }
        if (classInfo->type == XIScrollClass) {
            scrollInfos.emplace_back(reinterpret_cast<const XIScrollClassInfo*>(classInfo));
        }
    }

    if (buttonInfo && buttonInfo->num_buttons > 0) {
        setButtonMap(*buttonInfo, info.deviceid);
    }
    if (!scrollInfos.empty()) {
        for (auto scrollInfo : scrollInfos) {
            scrollMap[info.deviceid][scrollInfo->number] = *scrollInfo;
        }
    }
}

void evget::EventTransformerLinux::setButtonMap(const XIButtonClassInfo& buttonInfo, int id) {
    Display& displayCapture = display;
    auto deleter = [&displayCapture](XDevice* device) {
        XCloseDevice(&displayCapture, device);
    };
    auto device = std::unique_ptr<XDevice, decltype(deleter)>(XOpenDevice(&display.get(), id), deleter);

    if (device) {
        auto map = std::make_unique<unsigned char[]>(buttonInfo.num_buttons);
        XGetDeviceButtonMapping(&display.get(), device.get(), map.get(), buttonInfo.num_buttons);

        for (int i = 0; i < buttonInfo.num_buttons; i++) {
            if (!buttonInfo.labels[i]) {
                auto name = std::unique_ptr<char[], decltype(&XFree)>(XGetAtomName(&display.get(), buttonInfo.labels[i]), XFree);
                if (name) {
                    buttonMap[id][map[i]] = name.get();
                }
            }
        }
    }
}

evget::EventTransformerLinux::EventTransformerLinux(Display& display) : display{display} {
    refreshDeviceIds();
}
