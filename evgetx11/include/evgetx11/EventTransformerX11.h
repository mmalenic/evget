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

#ifndef EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
#define EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H

#include <unordered_map>
#include <X11/extensions/XInput2.h>
#include <set>
#include <map>
#include <concepts>
#include "XInputHandler.h"
#include "evgetcore/EventTransformer.h"
#include "evgetcore/Util.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetx11/XDeviceRefresh.h"
#include "XWrapper.h"

namespace EvgetX11 {
    class EventTransformerX11 : EvgetCore::EventTransformer<XInputEvent> {
    public:
        EventTransformerX11(XWrapper& xWrapper, std::initializer_list<std::reference_wrapper<XDeviceRefresh>> switches);
        std::vector<EvgetCore::Event::Data> transformEvent(XInputEvent event) override;

    private:
        std::chrono::nanoseconds getTime(const XInputEvent& event);
        void refreshDevices();

        std::reference_wrapper<XWrapper> xWrapper;
        std::optional<XInputEvent::Timestamp> start{std::nullopt};

        std::unordered_map<int, EvgetCore::Event::Device> devices{};
        std::unordered_map<int, std::string> idToName{};

        std::vector<std::reference_wrapper<XDeviceRefresh>> switches{};
    };
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
