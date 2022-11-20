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

#include "evgetx11/XEventSwitchTouch.h"
#include "evgetx11/XEventSwitch.h"

bool EvgetX11::XEventSwitchTouch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EvgetX11::EventData& data
) {
    switch (event.getEventType()) {
    case XI_TouchBegin:
        touchMotion(event, timestamp, data);
        touchButton(event, timestamp, data, EvgetCore::Event::ButtonAction::Press);
        return true;
    case XI_TouchUpdate:
        touchMotion(event, timestamp, data);
        return true;
    case XI_TouchEnd:
        touchMotion(event, timestamp, data);
        touchButton(event, timestamp, data, EvgetCore::Event::ButtonAction::Release);
        return true;
    default:
        return false;
    }
}

EvgetX11::XEventSwitchTouch::XEventSwitchTouch(XEventSwitchPointer& xEventSwitchPointer, XDeviceRefresh& xDeviceRefresh) :
                                                                     xEventSwitchPointer{xEventSwitchPointer},
                                                                     xDeviceRefresh{xDeviceRefresh} {
    xDeviceRefresh.setEvtypeName(XI_TouchBegin, "TouchBegin");
    xDeviceRefresh.setEvtypeName(XI_TouchUpdate, "TouchUpdate");
    xDeviceRefresh.setEvtypeName(XI_TouchEnd, "TouchEnd");
}

void EvgetX11::XEventSwitchTouch::touchButton(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<EvgetCore::Event::Data>& data,
    EvgetCore::Event::ButtonAction action
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xDeviceRefresh.get().containsDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get().addButtonEvent(deviceEvent, timestamp, event.getTimestamp(), data, action, 1);
    }
}

void EvgetX11::XEventSwitchTouch::touchMotion(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<EvgetCore::Event::Data>& data
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xDeviceRefresh.get().containsDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get().addMotionEvent(deviceEvent, timestamp, event.getTimestamp(), data);
    }
}
