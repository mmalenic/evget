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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H

#include "XEventSwitch.h"
#include "evgetcore/Event/Button/ButtonAction.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EvgetX11 {
    class CoreXEventSwitch : XEventSwitch {
    public:
        explicit CoreXEventSwitch(Display& display);

        bool switchOnEvent(const XInputEvent &event, std::chrono::nanoseconds timestamp, EventData &data) override;
        void refreshDevices(int id, EvgetCore::Event::Common::Device device, const std::string &name, const XIDeviceInfo &info) override;

    private:
        bool buttonEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, EvgetCore::Event::Button::ButtonAction action);
        bool keyEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data);
        void motionEvent(std::chrono::nanoseconds time, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, const XIDeviceEvent& deviceEvent);
        bool motionEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, int type, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data);
        bool scrollEvent(const XIDeviceEvent& event, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, const std::map<int, XIScrollClassInfo>& scrollValuators, int valuator);
        std::unique_ptr<EvgetCore::Event::MouseScroll> scrollEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp);
        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);
        void setEvtypeNames();
        static std::unique_ptr<_XIC, decltype(&XDestroyIC)> createIC(Display& display, XIM xim);

        std::reference_wrapper<Display> display;
        static constexpr int utf8MaxBytes = 4;

        std::map<int, std::map<int, std::string>> buttonMap{};
        std::map<int, std::map<int, XIScrollClassInfo>> scrollMap{};
        std::unique_ptr<EvgetCore::Event::MouseScroll> rawScrollEvent{};
        std::optional<int> valuatorX{};
        std::optional<int> valuatorY{};
        std::map<int, std::string> valuatorNames{};

        std::unique_ptr<_XIM, decltype(&XCloseIM)> xim = std::unique_ptr<_XIM, decltype(&XCloseIM)>{XOpenIM(&display.get(), nullptr, nullptr, nullptr), XCloseIM};
        std::unique_ptr<_XIC, decltype(&XDestroyIC)> xic = createIC(display, xim.get());
    };
}

#endif //EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
