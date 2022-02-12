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

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    if (event.hasData()) {
        if (!start.has_value()) {
            start = event.getTimestamp();
        }
        std::chrono::nanoseconds time = event.getTimestamp() - *start;

        const auto& deviceEvent = event.viewData<XIDeviceEvent>();

        switch (deviceEvent.evtype) {
        case XI_ButtonPress:break;
        case XI_KeyPress:break;
        case XI_KeyRelease:break;
        case XI_Motion:break;
#if defined XI_TouchBegin && defined XI_TouchUpdate && defined XI_TouchEnd
        case XI_TouchBegin:break;
        case XI_TouchUpdate:break;
        case XI_TouchEnd:break;
#endif
        case XI_HierarchyChanged:
        case XI_DeviceChanged:refreshDeviceIds();
            break;
        default:
            spdlog::info(
                "Unsupported event with type '{}' from device with id '{}' passed to event transformer.",
                deviceEvent.evtype,
                deviceEvent.deviceid
            );
            return {};
        }
    }
    return {};
}

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::buttonEvent(XIDeviceEvent& event, std::chrono::nanoseconds time) {
    Event::MouseClick::MouseClickBuilder genericBuilder;
    if (event.type == XI_ButtonPress) {
        genericBuilder = Event::MouseClick::MouseClickBuilder{};
        genericBuilder.press(std::to_string(event.detail));
    } else if (event.type == XI_ButtonRelease) {
        genericBuilder = Event::MouseClick::MouseClickBuilder{};
        genericBuilder.release(std::to_string(event.detail));
    } else {
        return {};
    }

    genericBuilder.time(time).positionX(event.root_x).positionY(event.root_y);

    std::string deviceName{};
    if (mouseIds.contains(event.deviceid)) {
        genericBuilder.type(Event::Common::Type::Device::Mouse);
        deviceName = mouseIds[event.deviceid];
    } else if (keyboardIds.contains(event.deviceid)) {
        genericBuilder.type(Event::Common::Type::Device::Keyboard);
        deviceName = keyboardIds[event.deviceid];
    } else if (touchpadIds.contains(event.deviceid)) {
        genericBuilder.type(Event::Common::Type::Device::Touchpad);
        deviceName = touchpadIds[event.deviceid];
    } else if (touchscreenIds.contains(event.deviceid)) {
        genericBuilder.type(Event::Common::Type::Device::Touchscreen);
        deviceName = touchscreenIds[event.deviceid];
    } else {
        spdlog::warn("Device id '{}' not found in supported devices.", event.deviceid);
        return {};
    }

    return Event::TableData::TableDataBuilder{}.genericData(genericBuilder.build()).systemData(createSystemData(event, "MouseClickSystemData", deviceName)).build();
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemData(XIDeviceEvent& event, const std::string& name, const std::string& deviceName) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};
    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", deviceName));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));
    fields.emplace_back(std::make_unique<Event::Field>("Flags", formatValue(event.flags)));

    auto buttonState = getMask(event.buttons.mask_len, event.buttons.mask);
    fields.emplace_back(std::make_unique<Event::Field>("ButtonState", formatValue(buttonState)));

    auto valuatorState = getMask(event.valuators.mask_len, event.valuators.mask);
    auto values = event.valuators.values;
    std::vector<int> valuatorValues{};
    if (!valuatorState.empty()) {
        valuatorValues.insert(valuatorValues.end(), &values[0], &values[valuatorState.size()]);
    }
    fields.emplace_back(std::make_unique<Event::Field>("ValuatorsSet", formatValue(valuatorState)));
    fields.emplace_back(std::make_unique<Event::Field>("ValuatorsValues", formatValue(valuatorValues)));

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

std::string evget::EventTransformerLinux::formatValue(int value) {
    return value != 0 ? std::to_string(value) : "";
}

std::string evget::EventTransformerLinux::formatValue(std::vector<int> values) {
    return !values.empty() ? fmt::format("[{}]", fmt::join(values, ", ")) : "";
}

std::vector<int> evget::EventTransformerLinux::getMask(int maskLen, const unsigned char* mask) {
    std::vector<int> masksSet{};
    for (int i = 0; i < maskLen * 8; i++) {
        if (XIMaskIsSet(mask, i)) {
            masksSet.emplace_back(i);
        }
    }
    return masksSet;
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
        spdlog::warn("Devices with ids greater than 127 found. Set the type of these devices manually if their use is required.");
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
                mouseIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                keyboardIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                touchscreenIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                touchpadIds.emplace(id, device.name);
            } else {
                spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
            }

            setButtonMap(xi2Devices.at(id).get());
        }
    }
}

void evget::EventTransformerLinux::setButtonMap(const XIDeviceInfo& info) {
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
                buttonMap[id][map[i]] = "None";
            } else {
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
