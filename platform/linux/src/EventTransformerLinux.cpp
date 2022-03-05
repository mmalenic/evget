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
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <xorg/xserver-properties.h>
#include <boost/numeric/conversion/cast.hpp>
#include "evget/EventTransformerLinux.h"
#include "evget/UnsupportedOperationException.h"
#include "evget/Event/MouseClick.h"
#include "evget/Event/Key.h"
#include "evget/Event/Field.h"
#include "evget/Event/MouseScroll.h"
#include "evget/Event/MouseMove.h"

std::vector<std::unique_ptr<Event::TableData>> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    if (event.hasData()) {
        std::vector<std::unique_ptr<Event::TableData>> data{};
        auto type = event.getEventType();

        if (motionEvent(event, type, data)) {
            return data;
        }

        switch (type) {
        case XI_RawMotion:
            rawScrollEvent = scrollEvent(event);
            break;
        case XI_ButtonPress:
            buttonEvent(event, data, Event::Button::ButtonAction::Press);
            break;
        case XI_ButtonRelease:
            buttonEvent(event, data, Event::Button::ButtonAction::Release);
            break;
        case XI_KeyPress:
        case XI_KeyRelease:
            keyEvent(event, data);
            break;
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

void evget::EventTransformerLinux::buttonEvent(const XInputEvent& event, std::vector<std::unique_ptr<Event::TableData>>& data, Event::Button::ButtonAction action) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!devices.contains(deviceEvent.deviceid) ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_WHEEL_UP ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_HWHEEL_LEFT ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    Event::MouseClick::MouseClickBuilder builder{};
    builder.time(getTime(event)).device(devices[deviceEvent.deviceid]).positionX(deviceEvent.root_x)
    .positionY(deviceEvent.root_y).action(action).button(deviceEvent.detail).name(buttonMap[deviceEvent.deviceid][deviceEvent.detail]);

    addTableData(data, builder.build(), createSystemDataWithoutRoot(deviceEvent,"MouseClickSystemData"));
}

void evget::EventTransformerLinux::keyEvent(const XInputEvent& event, std::vector<std::unique_ptr<Event::TableData>>& data) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!devices.contains(deviceEvent.deviceid)) {
        return;
    }

    // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems a
    // little bit hacky to do this conversion, however it should be okay as all the elements have a direct relationship.
    XKeyEvent keyEvent{
        .type = ButtonPress,
        .serial = deviceEvent.serial,
        .send_event = deviceEvent.send_event,
        .display = deviceEvent.display,
        .window = deviceEvent.event,
        .root = deviceEvent.root,
        .subwindow = deviceEvent.child,
        .time = deviceEvent.time,
        .x = static_cast<int>(deviceEvent.event_x),
        .y = static_cast<int>(deviceEvent.event_y),
        .x_root = static_cast<int>(deviceEvent.root_x),
        .y_root = static_cast<int>(deviceEvent.root_y),
        .state = static_cast<unsigned int>(deviceEvent.mods.effective),
        .keycode = static_cast<unsigned int>(deviceEvent.detail),
        .same_screen = true
    };

    std::string character{};
    KeySym keySym;

    int bytes;
    std::array<char, utf8MaxBytes + 1> array{};
    if (xic) {
        Status status;
        bytes = Xutf8LookupString(xic.get(), &keyEvent, array.data(), utf8MaxBytes, &keySym, &status);
        if (status == XBufferOverflow) {
            spdlog::warn("Buffer overflowed when looking up string, falling back to encoding key events in ISO Latin-1.");
            bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
        }
    } else {
        bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
    }

    Event::Button::ButtonAction action = Event::Button::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? Event::Button::ButtonAction::Repeat : Event::Button::ButtonAction::Press;
        array[bytes] = '\0';
        character = std::string{array.data()};
    }

    std::string name{};
    if (keySym != NoSymbol) {
        name = XKeysymToString(keySym);
    }

    Event::Key::KeyBuilder builder{};
    builder.time(getTime(event)).action(action).button(deviceEvent.detail).character(character).name(name);

    addTableData(data, builder.build(), createSystemDataWithRoot(deviceEvent, "KeySystemData"));
}

std::unique_ptr<Event::MouseScroll> evget::EventTransformerLinux::scrollEvent(
    const XInputEvent& event
) {
    auto rawEvent = event.viewData<XIRawEvent>();
    if (!devices.contains(rawEvent.sourceid) || !scrollMap.contains(rawEvent.sourceid)) {
        return {};
    }

    Event::MouseScroll::MouseScrollBuilder builder{};
    auto valuators = getValuators(rawEvent.valuators);
    bool isScrollEvent = false;
    for (const auto& [valuator, info] : scrollMap[rawEvent.sourceid]) {
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

    return isScrollEvent ? builder.time(getTime(event)).device(devices[rawEvent.sourceid]).build() : nullptr;
}

std::map<int, int> evget::EventTransformerLinux::getValuators(const XIValuatorState& valuatorState) {
    std::map<int, int> valuators{};
    auto* values = valuatorState.values;
    getMasks(valuatorState.mask, valuatorState.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, *values++);
    });
    return valuators;
}

bool evget::EventTransformerLinux::motionEvent(const XInputEvent& event, int type, std::vector<std::unique_ptr<Event::TableData>>& data) {
    if (type == XI_Motion) {
        auto deviceEvent = event.viewData<XIDeviceEvent>();
        if (devices.contains(deviceEvent.deviceid)) {
            motionEvent(getTime(event), data, deviceEvent);
        }
    }

    if (rawScrollEvent) {
        spdlog::warn("Missing complimentary scroll event after XIRawEvent.");
        data.emplace_back(Event::TableData::TableDataBuilder{}.genericData(std::move(rawScrollEvent)).build());
    }

    return type == XI_Motion;
}

void evget::EventTransformerLinux::motionEvent(
    std::chrono::nanoseconds time,
    std::vector<std::unique_ptr<Event::TableData>>& data,
    const XIDeviceEvent& deviceEvent
) {
    auto valuators = getValuators(deviceEvent.valuators);
    const auto& scrollValuators =
        scrollMap.contains(deviceEvent.deviceid) ? scrollMap[deviceEvent.deviceid] : std::map<int, XIScrollClassInfo>{};

    bool hasScroll = false;
    bool hasMotion = false;
    for (const auto& [valuator, value]: valuators) {
        if (!hasMotion && (valuator == valuatorX || valuator == valuatorY)) {
            Event::MouseMove::MouseMoveBuilder builder{};
            builder.time(time).device(devices[deviceEvent.deviceid]).positionX(deviceEvent.root_x).positionY(deviceEvent.root_y);

            addTableData(data, builder.build(), createSystemDataWithoutRoot(deviceEvent, "MouseMoveSystemData"));
            hasMotion = true;
        } else if (!hasScroll) {
            hasScroll = scrollEvent(deviceEvent, data, scrollValuators, valuator);
        }
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
            addTableData(data, std::move(rawScrollEvent), createSystemDataWithRoot(event, "MouseScrollSystemData"));
            return true;
        }
    }
    return false;
}

void evget::EventTransformerLinux::addTableData(
    std::vector<std::unique_ptr<Event::TableData>>& data,
    std::unique_ptr<Event::AbstractData> genericData,
    std::unique_ptr<Event::AbstractData> systemData
) {
    data.emplace_back(Event::TableData::TableDataBuilder{}.genericData(std::move(genericData)).systemData(std::move(systemData)).build());
}


std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemDataWithRoot(
    const XIDeviceEvent& event,
    const std::string& name
) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields = createSystemData(event, name);
    fields.emplace_back(std::make_unique<Event::Field>("RootX", std::to_string(event.root_x)));
    fields.emplace_back(std::make_unique<Event::Field>("RootY", std::to_string(event.root_y)));
    return std::make_unique<Event::Data>(name, std::move(fields));
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createSystemDataWithoutRoot(
    const XIDeviceEvent& event,
    const std::string& name
) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields = createSystemData(event, name);
    fields.emplace_back(std::make_unique<Event::Field>("RootX"));
    fields.emplace_back(std::make_unique<Event::Field>("RootY"));
    return std::make_unique<Event::Data>(name, std::move(fields));
}

std::vector<std::unique_ptr<Event::AbstractField>> evget::EventTransformerLinux::createSystemData(const XIDeviceEvent& event, const std::string& name) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", idToName[event.deviceid]));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));

    fields.emplace_back(std::make_unique<Event::Field>("Flags", formatValue(event.flags)));

    fields.emplace_back(std::make_unique<Event::Field>("ButtonState", createButtonEntries(event)));

    fields.emplace_back(std::make_unique<Event::Field>("Valuators", createValuatorEntries(event.valuators)));

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

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createRawData(
    const XIRawEvent& event
) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", idToName[event.sourceid]));
    fields.emplace_back(std::make_unique<Event::Field>("EventType", std::to_string(event.evtype)));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));
    fields.emplace_back(std::make_unique<Event::Field>("Detail", std::to_string(event.detail)));

    fields.emplace_back(std::make_unique<Event::Field>("Flags", formatValue(event.flags)));

    fields.emplace_back(std::make_unique<Event::Field>("Valuators", createValuatorEntries(event.valuators)));

    return std::make_unique<Event::Data>("RawEvent", std::move(fields));
}

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::createDeviceChangedEvent(const XIDeviceChangedEvent& event) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<Event::Field>("DeviceName", idToName[event.deviceid]));
    fields.emplace_back(std::make_unique<Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<Event::Field>("SourceId", std::to_string(event.sourceid)));
    fields.emplace_back(std::make_unique<Event::Field>("Reason", std::to_string(event.reason)));
    fields.emplace_back(std::make_unique<Event::Field>("ReasonName", (event.reason == XISlaveSwitch) ? "SlaveSwitch" : "DeviceChanged"));

    return std::make_unique<Event::Data>("DeviceChangedEvent", std::move(fields));
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

Event::AbstractField::Entries evget::EventTransformerLinux::createValuatorEntries(const XIValuatorState& valuatorState) {
    std::vector<std::unique_ptr<Event::AbstractData>> data{};

    auto values = valuatorState.values;
    getMasks(valuatorState.mask, valuatorState.mask_len, [&data, &values](int mask) {
        std::vector<std::unique_ptr<Event::AbstractField>> fields{};
        fields.emplace_back(std::make_unique<Event::Field>("Valuator", std::to_string(mask)));
        fields.emplace_back(std::make_unique<Event::Field>("Value", std::to_string(*values++)));
        data.emplace_back(std::make_unique<Event::Data>("Valuators", std::move(fields)));
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
            valuatorNames.emplace(valuatorInfo->number, name.get());
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
            if (buttonInfo.labels[i]) {
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

std::unique_ptr<_XIC, decltype(&XDestroyIC)> evget::EventTransformerLinux::createIC(Display& display, XIM xim) {
    if (xim) {
        XIMStyles* xim_styles;
        auto values = XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
        if (values || !xim_styles) {
            spdlog::warn("The input method does not support any styles, falling back to encoding key events in ISO Latin-1.");
            return {nullptr, XDestroyIC};
        }

        for (int i = 0;  i < xim_styles->count_styles;  i++) {
            if (xim_styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                XFree(xim_styles);
                Window window = XDefaultRootWindow(&display);
                return {XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, XNFocusWindow, window, nullptr), XDestroyIC};
            }
        }

        spdlog::warn("The input method does not support the PreeditNothing and StatusNothing style, falling back to encoding key events in ISO Latin-1.");
        XFree (xim_styles);
        return {nullptr, XDestroyIC};
    }
}

std::map<int, std::string> evget::EventTransformerLinux::typeToName() {
    std::map<int, std::string> map{};

    map.emplace(XI_DeviceChanged, "DeviceChanged");
    map.emplace(XI_KeyPress, "KeyPress");
    map.emplace(XI_KeyRelease, "KeyRelease");
    map.emplace(XI_ButtonPress, "ButtonPress");
    map.emplace(XI_ButtonRelease, "ButtonRelease");
    map.emplace(XI_Motion, "Motion");
    map.emplace(XI_Enter, "Enter");
    map.emplace(XI_Leave, "Leave");
    map.emplace(XI_FocusIn, "FocusIn");
    map.emplace(XI_FocusOut, "FocusOut");
    map.emplace(XI_HierarchyChanged, "HierarchyChanged");
    map.emplace(XI_PropertyEvent, "PropertyEvent");
    map.emplace(XI_RawKeyPress, "RawKeyPress");
    map.emplace(XI_RawKeyRelease, "RawKeyRelease");
    map.emplace(XI_RawButtonPress, "RawButtonPress");
    map.emplace(XI_RawButtonRelease, "RawButtonRelease");
    map.emplace(XI_RawMotion, "RawMotion");
#if defined XI_TouchBegin && defined XI_TouchUpdate && defined XI_TouchEnd
    map.emplace(XI_TouchBegin, "TouchBegin");
    map.emplace(XI_TouchUpdate, "TouchUpdate");
    map.emplace(XI_TouchEnd, "TouchEnd");
    map.emplace(XI_RawTouchBegin, "RawTouchBegin");
    map.emplace(XI_RawTouchUpdate, "RawTouchUpdate");
    map.emplace(XI_RawTouchEnd, "RawTouchEnd");
#endif

    return map;
}
