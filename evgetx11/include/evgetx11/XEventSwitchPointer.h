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
//

#ifndef EVGET_XEVENTSWITCHPOINTER_H
#define EVGET_XEVENTSWITCHPOINTER_H

#include "XDeviceRefresh.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetcore/Event/MouseClick.h"

namespace EvgetX11 {
    class XEventSwitchPointer {
    public:
        explicit XEventSwitchPointer(XWrapper& xWrapper, XDeviceRefresh& xDeviceRefresh);

        void refreshDevices(int id, EvgetCore::Event::Device device, const std::string &name, const XIDeviceInfo &info);

        void addButtonEvent(const XIDeviceEvent& event, EvgetCore::Event::SchemaField::Timestamp dateTime, std::vector<EvgetCore::Event::Data>& data, EvgetCore::Event::ButtonAction action, int button, EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime);
        void addMotionEvent(const XIDeviceEvent& event, EvgetCore::Event::SchemaField::Timestamp dateTime, std::vector<EvgetCore::Event::Data>& data, EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime);

        const std::string &getButtonName(int id, int button) const;

        /**
         * Get the xkb modifier value.
         */
        static std::optional<EvgetCore::Event::ModifierValue> getModifierValue(int modifierState);

    private:
        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

        std::reference_wrapper<XWrapper> xWrapper;
        std::reference_wrapper<XDeviceRefresh> xDeviceRefresh;
        std::unordered_map<int, std::unordered_map<int, std::string>> buttonMap{};
    };

    void EvgetX11::XEventSwitchPointer::addMotionEvent(
            const XIDeviceEvent& event,
            EvgetCore::Event::SchemaField::Timestamp dateTime,
            std::vector<EvgetCore::Event::Data>& data,
            EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    ) {
        EvgetCore::Event::MouseMove builder{};
        builder.interval(getTime(event.time)).timestamp(dateTime).device(xDeviceRefresh.get().getDevice(event.deviceid)).positionX(event.root_x).positionY(event.root_y);

        data.emplace_back(builder.build());
    }

    void EvgetX11::XEventSwitchPointer::addButtonEvent(
            const XIDeviceEvent& event,
            EvgetCore::Event::SchemaField::Timestamp dateTime,
            std::vector<EvgetCore::Event::Data>& data,
            EvgetCore::Event::ButtonAction action,
            int button,
            EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    ) {
        EvgetCore::Event::MouseClick builder{};
        builder.interval(getTime(event.time)).timestamp(dateTime).device(xDeviceRefresh.get().getDevice(event.deviceid)).positionX(event.root_x)
                .positionY(event.root_y).action(action).button(button).name(buttonMap[event.deviceid][button]);

        data.emplace_back(builder.build());
    }
}

#endif //EVGET_XEVENTSWITCHPOINTER_H
