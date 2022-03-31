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
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <xorg/xserver-properties.h>
#include <X11/extensions/XInput.h>
#include "evgetx11/CoreXEventSwitch.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseClick.h"

bool EvgetX11::CoreXEventSwitch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EventData& data
) {
    XEventSwitch::switchOnEvent(event, timestamp, data);

    auto type = event.getEventType();
    if (motionEvent(event, timestamp, type, data)) {
        return true;
    }

    switch (type) {
    case XI_RawMotion:
        rawScrollEvent = scrollEvent(event, timestamp);
        return true;
    case XI_ButtonPress:
        return buttonEvent(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Press);
    case XI_ButtonRelease:
        return buttonEvent(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Release);
    case XI_KeyPress:
    case XI_KeyRelease:
        return keyEvent(event, timestamp, data);
    default:
        return false;
    }
}

bool EvgetX11::CoreXEventSwitch::buttonEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, EvgetCore::Event::Button::ButtonAction action) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!devices.contains(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated) ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_WHEEL_UP ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_HWHEEL_LEFT ||
        buttonMap[deviceEvent.deviceid][deviceEvent.detail] == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return true;
    }

    EvgetCore::Event::MouseClick::MouseClickBuilder builder{};
    builder.time(timestamp).device(devices[deviceEvent.deviceid]).positionX(deviceEvent.root_x)
           .positionY(deviceEvent.root_y).action(action).button(deviceEvent.detail).name(buttonMap[deviceEvent.deviceid][deviceEvent.detail]);

    addTableData(data, builder.build(), createSystemDataWithoutRoot(deviceEvent,"MouseClickSystemData"));
    return true;
}

bool EvgetX11::CoreXEventSwitch::keyEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!devices.contains(deviceEvent.deviceid)) {
        return true;
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

    EvgetCore::Event::Button::ButtonAction action = EvgetCore::Event::Button::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? EvgetCore::Event::Button::ButtonAction::Repeat : EvgetCore::Event::Button::ButtonAction::Press;
        array[bytes] = '\0';
        character = std::string{array.data()};
    }

    std::string name{};
    if (keySym != NoSymbol) {
        name = XKeysymToString(keySym);
    }

    EvgetCore::Event::Key::KeyBuilder builder{};
    builder.time(timestamp).action(action).button(deviceEvent.detail).character(character).name(name);

    addTableData(data, builder.build(), createSystemDataWithRoot(deviceEvent, "KeySystemData"));
    return true;
}

std::unique_ptr<EvgetCore::Event::MouseScroll> EvgetX11::CoreXEventSwitch::scrollEvent(
    const XInputEvent& event,
    std::chrono::nanoseconds timestamp
) {
    auto rawEvent = event.viewData<XIRawEvent>();
    if (!devices.contains(rawEvent.sourceid) || !scrollMap.contains(rawEvent.sourceid) || (rawEvent.flags & XIPointerEmulated)) {
        return {};
    }

    EvgetCore::Event::MouseScroll::MouseScrollBuilder builder{};
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

    return isScrollEvent ? builder.time(timestamp).device(devices[rawEvent.sourceid]).build() : nullptr;
}

bool EvgetX11::CoreXEventSwitch::motionEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, int type, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data) {
    if (type == XI_Motion) {
        auto deviceEvent = event.viewData<XIDeviceEvent>();
        if (devices.contains(deviceEvent.deviceid) && !(deviceEvent.flags & XIPointerEmulated)) {
            motionEvent(timestamp, data, deviceEvent);
        }
    }

    if (rawScrollEvent) {
        spdlog::warn("Missing complimentary scroll event after XIRawEvent.");
        data.emplace_back(EvgetCore::Event::TableData::TableDataBuilder{}.genericData(std::move(rawScrollEvent)).build());
    }

    return type == XI_Motion;
}

void EvgetX11::CoreXEventSwitch::motionEvent(
    std::chrono::nanoseconds time,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
    const XIDeviceEvent& deviceEvent
) {
    auto valuators = getValuators(deviceEvent.valuators);
    const auto& scrollValuators =
        scrollMap.contains(deviceEvent.deviceid) ? scrollMap[deviceEvent.deviceid] : std::map<int, XIScrollClassInfo>{};

    bool hasScroll = false;
    bool hasMotion = false;
    for (const auto& [valuator, value]: valuators) {
        if (!hasMotion && (valuator == valuatorX || valuator == valuatorY)) {
            EvgetCore::Event::MouseMove::MouseMoveBuilder builder{};
            builder.time(time).device(devices[deviceEvent.deviceid]).positionX(deviceEvent.root_x).positionY(deviceEvent.root_y);

            addTableData(data, builder.build(), createSystemDataWithoutRoot(deviceEvent, "MouseMoveSystemData"));
            hasMotion = true;
        } else if (!hasScroll) {
            hasScroll = scrollEvent(deviceEvent, data, scrollValuators, valuator);
        }
    }
}

bool EvgetX11::CoreXEventSwitch::scrollEvent(
    const XIDeviceEvent& event,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
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

void EvgetX11::CoreXEventSwitch::setButtonMap(const XIButtonClassInfo& buttonInfo, int id) {
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
                auto name = getAtomName(display, buttonInfo.labels[i]);
                if (name) {
                    buttonMap[id][map[i]] = name.get();
                }
            }
        }
    }
}

std::unique_ptr<_XIC, decltype(&XDestroyIC)> EvgetX11::CoreXEventSwitch::createIC(Display& display, XIM xim) {
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

EvgetX11::CoreXEventSwitch::CoreXEventSwitch(Display& display) : display{display} {
    setEvtypeNames();
}

void EvgetX11::CoreXEventSwitch::refreshDevices(
    int id,
    EvgetCore::Event::Common::Device device,
    const std::string& name,
    const XIDeviceInfo& info
) {
    XEventSwitch::refreshDevices(id, device, name, info);

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
        auto name = getAtomName(display, valuatorInfo->label);
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

void EvgetX11::CoreXEventSwitch::setEvtypeNames() {
    evtypeToName.emplace(XI_DeviceChanged, "DeviceChanged");
    evtypeToName.emplace(XI_KeyPress, "KeyPress");
    evtypeToName.emplace(XI_KeyRelease, "KeyRelease");
    evtypeToName.emplace(XI_ButtonPress, "ButtonPress");
    evtypeToName.emplace(XI_ButtonRelease, "ButtonRelease");
    evtypeToName.emplace(XI_Motion, "Motion");
    evtypeToName.emplace(XI_Enter, "Enter");
    evtypeToName.emplace(XI_Leave, "Leave");
    evtypeToName.emplace(XI_FocusIn, "FocusIn");
    evtypeToName.emplace(XI_FocusOut, "FocusOut");
    evtypeToName.emplace(XI_HierarchyChanged, "HierarchyChanged");
    evtypeToName.emplace(XI_PropertyEvent, "PropertyEvent");
    evtypeToName.emplace(XI_RawKeyPress, "RawKeyPress");
    evtypeToName.emplace(XI_RawKeyRelease, "RawKeyRelease");
    evtypeToName.emplace(XI_RawButtonPress, "RawButtonPress");
    evtypeToName.emplace(XI_RawButtonRelease, "RawButtonRelease");
    evtypeToName.emplace(XI_RawMotion, "RawMotion");
}
