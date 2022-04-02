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

#include "evgetx11/TouchXEventSwitch.h"

bool EvgetX11::TouchXEventSwitch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EvgetX11::XEventSwitch::EventData& data
) {
    switch (event.getEventType()) {
    case XI_TouchBegin:
        touchMotion(event, timestamp, data);
        touchButton(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Press);
        return true;
    case XI_TouchUpdate:
        touchMotion(event, timestamp, data);
        return true;
    case XI_TouchEnd:
        touchMotion(event, timestamp, data);
        touchButton(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Release);
        return true;
    default:
        return false;
    }
}


EvgetX11::TouchXEventSwitch::TouchXEventSwitch(EvgetX11::CoreXEventSwitch& coreXEventSwitch) : coreXEventSwitch{coreXEventSwitch} {
}

void EvgetX11::TouchXEventSwitch::touchButton(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
    EvgetCore::Event::Button::ButtonAction action
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (devices.contains(deviceEvent.deviceid)) {
        coreXEventSwitch.get().addButtonEvent(deviceEvent, timestamp, data, action, 1);
    }
}

void EvgetX11::TouchXEventSwitch::touchMotion(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (devices.contains(deviceEvent.deviceid)) {
        coreXEventSwitch.get().addMotionEvent(deviceEvent, timestamp, data);
    }
}
