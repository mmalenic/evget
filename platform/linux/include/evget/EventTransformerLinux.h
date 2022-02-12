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

#include <map>
#include <X11/extensions/XInput.h>
#include "XInputHandler.h"
#include "evget/EventTransformer.h"
#include "evget/Event/TableData.h"

namespace evget {

    class EventTransformerLinux : EventTransformer<XInputEvent> {
    public:
        explicit EventTransformerLinux(Display& display);

        std::unique_ptr<Event::TableData> transformEvent(XInputEvent event) override;

    private:
        static std::unique_ptr<Event::AbstractData> createSystemData(XIDeviceEvent& event, const std::string& data, const std::string& deviceName);
        static std::vector<int> getMask(int maskLen, const unsigned char* mask);
        static std::string formatValue(int value);
        static std::string formatValue(std::vector<int> values);

        std::unique_ptr<Event::TableData> buttonEvent(XIDeviceEvent& event, std::chrono::nanoseconds time);

        void refreshDeviceIds();
        void setButtonMap(const XIDeviceInfo& info);
        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

        std::reference_wrapper<Display> display;

        std::map<int, std::map<int, std::string>> buttonMap{};
        std::map<int, std::map<int, XIScrollClassInfo>> scrollMap{};
        bool downScrollNegative{false};
        std::map<int, std::string> mouseIds{};
        std::map<int, std::string> keyboardIds{};
        std::map<int, std::string> touchscreenIds{};
        std::map<int, std::string> touchpadIds{};

        std::optional<XInputEvent::Timestamp> start{std::nullopt};
    };
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
