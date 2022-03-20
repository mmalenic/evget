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
#include <X11/extensions/XInput2.h>
#include <set>
#include <concepts>
#include "XInputHandler.h"
#include "evgetcore/EventTransformer.h"
#include "evgetcore/Event/TableData.h"
#include "evgetcore/Util.h"
#include "evgetcore/Event/Common/DeviceType.h"
#include "evgetcore/Event/Button/ButtonAction.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EvgetX11 {
    class EventTransformerLinux : EvgetCore::EventTransformer<XInputEvent> {
    public:
        using EventData = std::vector<std::unique_ptr<EvgetCore::Event::TableData>>;

        explicit EventTransformerLinux(Display& display);

        std::vector<std::unique_ptr<EvgetCore::Event::TableData>> transformEvent(XInputEvent event) override;

    private:
        std::unique_ptr<EvgetCore::Event::AbstractData> createDeviceChangedEvent(const XIDeviceChangedEvent& event);

        static std::map<int, std::string> typeToName();

        std::chrono::nanoseconds getTime(const XInputEvent& event);

        void buttonEvent(const XInputEvent& event, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, EvgetCore::Event::Button::ButtonAction action);
        void keyEvent(const XInputEvent& event, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data);
        void motionEvent(std::chrono::nanoseconds time, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, const XIDeviceEvent& deviceEvent);
        bool motionEvent(const XInputEvent& event, int type, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data);
        bool scrollEvent(const XIDeviceEvent& event, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, const std::map<int, XIScrollClassInfo>& scrollValuators, int valuator);
        std::unique_ptr<EvgetCore::Event::MouseScroll> scrollEvent(const XInputEvent& event);

        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

        static std::unique_ptr<_XIC, decltype(&XDestroyIC)> createIC(Display& display, XIM xim);

        static constexpr int utf8MaxBytes = 4;

        std::reference_wrapper<Display> display;

        std::map<int, std::map<int, std::string>> buttonMap{};

        std::map<int, std::map<int, XIScrollClassInfo>> scrollMap{};
        std::unique_ptr<EvgetCore::Event::MouseScroll> rawScrollEvent{};

        std::optional<int> valuatorX{};
        std::optional<int> valuatorY{};
        std::map<int, std::string> valuatorNames{};

        std::unique_ptr<_XIM, decltype(&XCloseIM)> xim = std::unique_ptr<_XIM, decltype(&XCloseIM)>{XOpenIM(&display.get(), nullptr, nullptr, nullptr), XCloseIM};
        std::unique_ptr<_XIC, decltype(&XDestroyIC)> xic = createIC(display, xim.get());

        std::optional<XInputEvent::Timestamp> start{std::nullopt};

        std::map<int, std::string> types = typeToName();
    };
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
