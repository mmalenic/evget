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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H

#include "XEventSwitch.h"
#include "XEventSwitchCore.h"

namespace EvgetX11 {
    class XEventTwitchTouch : XEventSwitch {
    public:
        explicit XEventTwitchTouch(XEventSwitchCore &coreXEventSwitch);

        bool switchOnEvent(const XInputEvent &event, std::chrono::nanoseconds timestamp, EventData &data) override;

    private:
        void touchButton(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, EvgetCore::Event::Button::ButtonAction action);
        void touchMotion(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data);

        std::reference_wrapper<XEventSwitchCore> coreXEventSwitch;
    };
}

#endif //EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H