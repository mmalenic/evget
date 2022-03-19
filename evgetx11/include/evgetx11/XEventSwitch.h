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

#ifndef EVGET_EVGETX11_SRC_XEVENTSWITCH_H
#define EVGET_EVGETX11_SRC_XEVENTSWITCH_H

#include "evgetx11/EventTransformerLinux.h"

namespace EvgetX11 {
    class XEventSwitch {
    public:
        XEventSwitch() = default;

        /**
         * Switch on the event type and add relevant data to the event data.
         * Returns true if event was successfully consumed.
         */
        virtual bool switchOnEvent(const XInputEvent &event, EventTransformerLinux::EventData &data, std::chrono::nanoseconds timestamp) = 0;

        virtual ~XEventSwitch() = default;

        XEventSwitch(XEventSwitch&&) noexcept = delete;
        XEventSwitch& operator=(XEventSwitch&&) noexcept = delete;

        XEventSwitch(const XEventSwitch&) = delete;
        XEventSwitch& operator=(const XEventSwitch&) = delete;
    };
}

#endif //EVGET_EVGETX11_SRC_XEVENTSWITCH_H
