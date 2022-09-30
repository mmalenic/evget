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
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <xorg/xserver-properties.h>
#include <boost/numeric/conversion/cast.hpp>
#include "evgetx11/EventTransformerX11.h"
#include "evgetcore/UnsupportedOperationException.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/Field.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetx11/XDeviceRefresh.h"

std::vector<EvgetCore::Event::Data> EvgetX11::EventTransformerX11::transformEvent(XInputEvent event) {
    std::vector<EvgetCore::Event::Data> data{};
    if (event.hasData()) {
        auto type = event.getEventType();

        if (type == XI_DeviceChanged || type == XI_HierarchyChanged) {
            refreshDevices();
            return data;
        }

        for (const auto& eventSwitches : switches) {
            if (eventSwitches.get().switchOnEvent(event, getTime(event), data)) {
                return data;
            }
        }
    }
    return data;
}

std::chrono::nanoseconds EvgetX11::EventTransformerX11::getTime(const EvgetX11::XInputEvent& event) {
    if (!start.has_value()) {
        start = event.getTimestamp();
    }
    return event.getTimestamp() - *start;
}

EvgetX11::EventTransformerX11::EventTransformerX11(XWrapper& xWrapper, std::initializer_list<std::reference_wrapper<XDeviceRefresh>> switches) : xWrapper{xWrapper}, switches{switches} {
    refreshDevices();
}