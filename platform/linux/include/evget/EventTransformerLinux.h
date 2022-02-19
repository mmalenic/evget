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
#include <set>
#include <concepts>
#include "XInputHandler.h"
#include "evget/EventTransformer.h"
#include "evget/Event/TableData.h"
#include "evget/Util.h"
#include "evget/Event/Common/DeviceType.h"
#include "evget/Event/Button/Action.h"

namespace evget {

    class EventTransformerLinux : EventTransformer<XInputEvent> {
    public:
        explicit EventTransformerLinux(Display& display);

        std::unique_ptr<Event::TableData> transformEvent(XInputEvent event) override;

    private:
        std::unique_ptr<Event::AbstractData> createSystemDataNoRoot(const XIDeviceEvent& event, const std::string& name, std::initializer_list<int> excludeValuators = {});
        std::unique_ptr<Event::AbstractData> createSystemDataWithRoot(const XIDeviceEvent& event, const std::string& name, std::initializer_list<int> excludeValuators = {});
        std::vector<std::unique_ptr<Event::AbstractField>> createSystemData(const XIDeviceEvent& event, const std::string& name, std::initializer_list<int> excludeValuators);
        static Event::AbstractField::Entries createValuatorEntries(const XIDeviceEvent& event, std::initializer_list<int> exclude);
        static Event::AbstractField::Entries createButtonEntries(const XIDeviceEvent& event);

        static void getMasks(const unsigned char* mask, int maskLen, evget::Util::Invocable<void, int> auto&& function);
        static std::string formatValue(int value);

        std::chrono::nanoseconds getTime(evget::XInputEvent& event);

        std::unique_ptr<Event::TableData> buttonEvent(const XIDeviceEvent& event, std::chrono::nanoseconds time, Event::Button::Action action);
        std::unique_ptr<Event::TableData> scrollEvent(const XIDeviceEvent& event, std::chrono::nanoseconds time);
        std::unique_ptr<Event::TableData> scrollEvent(const XIRawEvent& event, std::chrono::nanoseconds time);

        void refreshDeviceIds();
        void setInfo(const XIDeviceInfo& info);
        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

        std::reference_wrapper<Display> display;

        std::map<int, std::map<int, std::string>> buttonMap{};
        std::map<int, std::map<int, XIScrollClassInfo>> scrollMap{};

        bool downScrollNegative{false};

        std::map<int, Event::Common::Device> devices{};
        std::map<int, std::string> idToName{};
        std::optional<XInputEvent::Timestamp> start{std::nullopt};
    };

    void evget::EventTransformerLinux::getMasks(const unsigned char* mask, int maskLen, evget::Util::Invocable<void, int> auto&& function) {
        for (int i = 0; i < maskLen * 8; i++) {
            if (XIMaskIsSet(mask, i)) {
                function(i);
            }
        }
    }
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
