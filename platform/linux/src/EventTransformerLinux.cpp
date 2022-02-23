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
#include <xorg/xserver-properties.h>
#include <boost/numeric/conversion/cast.hpp>
#include "evget/EventTransformerLinux.h"
#include "evget/UnsupportedOperationException.h"
#include "evget/Event/MouseClick.h"
#include "evget/Event/Field.h"
#include "evget/Event/MouseScroll.h"

std::vector<std::unique_ptr<Event::TableData>> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    if (event.hasData()) {
        std::vector<std::unique_ptr<Event::TableData>> data{};
        auto type = event.getEventType();

        scrollEvent(event.viewData<XIDeviceEvent>(), getTime(event), data);
        switch (type) {
        case XI_RawMotion:
            rawScrollEvent = scrollEvent(event.viewData<XIRawEvent>(), getTime(event));
            break;
        case XI_ButtonPress:
            data.emplace_back(buttonEvent(event.viewData<XIDeviceEvent>(), getTime(event), Event::Button::Action::Press));
            break;
        case XI_ButtonRelease:
            data.emplace_back(buttonEvent(event.viewData<XIDeviceEvent>(), getTime(event), Event::Button::Action::Release));
            break;
        case XI_KeyPress:break;
        case XI_KeyRelease:break;
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
        return data;
    }
    return {};
}

std::chrono::nanoseconds evget::EventTransformerLinux::getTime(const evget::XInputEvent& event) {
    if (!start.has_value()) {
        start = event.getTimestamp();
    }
    return event.getTimestamp() - *start;
}

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::buttonEvent(const XIDeviceEvent& event, std::chrono::nanoseconds time, Event::Button::Action action) {
    if (!devices.contains(event.deviceid) ||
        buttonMap[event.deviceid][event.detail] == BTN_LABEL_PROP_BTN_WHEEL_UP ||
        buttonMap[event.deviceid][event.detail] == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        buttonMap[event.deviceid][event.detail] == BTN_LABEL_PROP_BTN_HWHEEL_LEFT ||
        buttonMap[event.deviceid][event.detail] == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return {};
    }

    Event::MouseClick::MouseClickBuilder builder{};
    builder.time(time).device(devices[event.deviceid]).positionX(event.root_x).positionY(event.root_y).action(action).button(event.detail).name(buttonMap[event.deviceid][event.detail]);
    return Event::TableData::TableDataBuilder{}.genericData(builder.build()).systemData(createSystemDataNoRoot(event, "MouseClickSystemData")).build();
}

std::unique_ptr<Event::MouseScroll> evget::EventTransformerLinux::scrollEvent(
    const XIRawEvent& event,
    std::chrono::nanoseconds time
) {
    if (!devices.contains(event.sourceid) || !scrollMap.contains(event.sourceid)) {
        return {};
    }

    Event::MouseScroll::MouseScrollBuilder builder{};
    auto valuators = getValuators(event.valuators);
    bool isScrollEvent = false;
    for (const auto& [valuator, info] : scrollMap[event.sourceid]) {
        if (!valuators.contains(valuator)) {
            continue;
        }
        if (info.type == XIScrollTypeVertical) {
            if (info.increment * valuators[valuator] >= 0) {
                builder.down(valuators[valuator]);
            } else {
                builder.up(valuators[valuator]);
            }
        } else {
            if (info.increment * valuators[valuator] >= 0) {
                builder.left(valuators[valuator]);
            } else {
                builder.right(valuators[valuator]);
            }
        }
        isScrollEvent = true;
    }

    return isScrollEvent ? builder.time(time).device(devices[event.sourceid]).build() : nullptr;
}

std::map<int, int> evget::EventTransformerLinux::getValuators(const XIValuatorState& valuatorState) {
    std::map<int, int> valuators{};
    auto* values = valuatorState.values;
    getMasks(valuatorState.mask, valuatorState.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, *values++);
    });
    return valuators;
}

void evget::EventTransformerLinux::scrollEvent(const XIDeviceEvent& event, std::chrono::nanoseconds time, std::vector<std::unique_ptr<Event::TableData>>& data) {
    bool hasScroll = false;
    bool hasMotion = false;
    if (event.evtype == XI_Motion && devices.contains(event.deviceid)) {
        auto valuators = getValuators(event.valuators);
        const auto& scrollValuators =
            scrollMap.contains(event.deviceid) ? scrollMap[event.deviceid] : std::map<int, XIScrollClassInfo>{};
        for (const auto& [valuator, value]: valuators) {
            if (!hasMotion && (valuator == valuatorX || valuator == valuatorY)) {
                hasMotion = true;
            } else if (!hasScroll) {
                hasScroll = scrollEvent(event, data, scrollValuators, valuator);
            }
        }
    }
    if (rawScrollEvent) {
        spdlog::warn("Missing complimentary scroll event after XIRawEvent.");
        data.emplace_back(Event::TableData::TableDataBuilder{}.genericData(std::move(rawScrollEvent)).build());
    }
}
bool evget::EventTransformerLinux::scrollEvent(
    const XIDeviceEvent& event,
    std::vector<std::unique_ptr<Event::TableData>>& data,
    const std::map<int, XIScrollClassInfo>& scrollValuators,
    const int valuator
) {
    for (const auto& [scrollValuator, _]: scrollValuators) {
        if (valuator == scrollValuator) {
            data.emplace_back(
                Event::TableData::TableDataBuilder{}.genericData(std::move(rawScrollEvent)).systemData(
                    createSystemDataWithRoot(
                        event,
                        "MouseScrollSystemData"
                    )
                ).build()
            );
            return true;
        }
    }
    return false;
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemDataWithRoot(
    const XIDeviceEvent& event,
    const std::string& name,
    std::initializer_list<int> excludeValuators
) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields = createSystemData(event, name, excludeValuators);
    fields.emplace_back(std::make_unique<Event::Field>("RootX", std::to_string(event.root_x)));
    fields.emplace_back(std::make_unique<Event::Field>("RootY", std::to_string(event.root_y)));
    return std::make_unique<Event::Data>(name, std::move(fields));
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemDataNoRoot(
    const XIDeviceEvent& event,
    const std::string& name,
    std::initializer_list<int> excludeValuators
) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields = createSystemData(event, name, excludeValuators);
    fields.emplace_back(std::make_unique<Event::Field>("RootX"));
    fields.emplace_back(std::make_unique<Event::Field>("RootY"));
    return std::make_unique<Event::Data>(name, std::move(fields));
}

std::vector<std::unique_ptr<Event::AbstractField>> evget::EventTransformerLinux::createSystemData(const XIDeviceEvent& event, const std::string& name, std::initializer_list<int> excludeValuators) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", idToName[event.deviceid]));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));

    fields.emplace_back(std::make_unique<Event::Field>("Flags", formatValue(event.flags)));

    fields.emplace_back(std::make_unique<Event::Field>("ButtonState", createButtonEntries(event)));

    fields.emplace_back(std::make_unique<Event::Field>("Valuators", createValuatorEntries(event, excludeValuators)));

    fields.emplace_back(std::make_unique<Event::Field>("ModifiersBase", formatValue(event.mods.base)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersEffective", formatValue(event.mods.effective)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersLatched", formatValue(event.mods.latched)));
    fields.emplace_back(std::make_unique<Event::Field>("ModifiersLocked", formatValue(event.mods.locked)));

    fields.emplace_back(std::make_unique<Event::Field>("GroupBase", formatValue(event.group.base)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupEffective", formatValue(event.group.effective)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupLatched", formatValue(event.group.latched)));
    fields.emplace_back(std::make_unique<Event::Field>("GroupLocked", formatValue(event.group.locked)));

    return fields;
}

Event::AbstractField::Entries evget::EventTransformerLinux::createButtonEntries(const XIDeviceEvent& event) {
    std::vector<std::unique_ptr<Event::AbstractData>> data{};

    getMasks(event.buttons.mask, event.buttons.mask_len, [&data](int mask) {
        std::vector<std::unique_ptr<Event::AbstractField>> fields{};
        fields.emplace_back(std::make_unique<Event::Field>("ButtonActive", std::to_string(mask)));
        data.emplace_back(std::make_unique<Event::Data>("ButtonState", std::move(fields)));
    });

    return data;
}

Event::AbstractField::Entries evget::EventTransformerLinux::createValuatorEntries(const XIDeviceEvent& event, std::initializer_list<int> exclude) {
    std::vector<std::unique_ptr<Event::AbstractData>> data{};

    auto* values = event.valuators.values;
    getMasks(event.buttons.mask, event.buttons.mask_len, [&data, &exclude, &values](int mask) {
        if (std::find(exclude.begin(), exclude.end(), mask) == exclude.end()) {
            std::vector<std::unique_ptr<Event::AbstractField>> fields{};
            fields.emplace_back(std::make_unique<Event::Field>("Valuator", std::to_string(mask)));
            fields.emplace_back(std::make_unique<Event::Field>("Value", std::to_string(*values++)));
            data.emplace_back(std::make_unique<Event::Data>("Valuators", std::move(fields)));
        }
    });

    return data;
}

std::string evget::EventTransformerLinux::formatValue(int value) {
    return value != 0 ? std::to_string(value) : "";
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
            auto type = getAtomName(device.type);

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
    std::vector<const XIValuatorClassInfo*> valuatorInfos{};
    for (int i = 0; i < info.num_classes; i++) {
        const auto* classInfo = info.classes[i];

        if (classInfo->type == XIButtonClass) {
            buttonInfo = reinterpret_cast<const XIButtonClassInfo*>(classInfo);
        } else if (classInfo->type == XIScrollClass) {
            scrollInfos.emplace_back(reinterpret_cast<const XIScrollClassInfo*>(classInfo));
        } else if (classInfo->type == XIValuatorClass) {
            valuatorInfos.emplace_back(reinterpret_cast<const XIValuatorClassInfo*>(classInfo));
        }
    }

    if (buttonInfo && buttonInfo->num_buttons > 0) {
        setButtonMap(*buttonInfo, info.deviceid);
    }
    for (auto scrollInfo : scrollInfos) {
        scrollMap[info.deviceid][scrollInfo->number] = *scrollInfo;
    }
    for (auto valuatorInfo : valuatorInfos) {
        auto name = getAtomName(valuatorInfo->label);
        if (name) {
            if (strcmp(name.get(), AXIS_LABEL_PROP_ABS_X) == 0) {
                valuatorX = valuatorInfo->number;
            } else if (strcmp(name.get(), AXIS_LABEL_PROP_ABS_Y) == 0) {
                valuatorY = valuatorInfo->number;
            }
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
                auto name = getAtomName(buttonInfo.labels[i]);
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

std::unique_ptr<char[], decltype(&XFree)> evget::EventTransformerLinux::getAtomName(Atom atom) {
    return {XGetAtomName(&display.get(), atom), XFree};
}
