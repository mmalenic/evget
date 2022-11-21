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

#include "XDeviceRefresh.h"
#include <unordered_map>
#include "evgetcore/Event/MouseScroll.h"
#include "XWrapper.h"
#include "XEventSwitchPointer.h"

namespace EvgetX11 {
    class XEventSwitchCore {
    public:
        explicit XEventSwitchCore(XWrapper& xWrapper, XEventSwitchPointer& xEventSwitchPointer, XDeviceRefresh& xDeviceRefresh);

        void refreshDevices(int id, EvgetCore::Event::Device device, const std::string &name, const XIDeviceInfo &info);
        bool switchOnEvent(const XInputEvent &event, EventData &data);

    private:
        void buttonEvent(const XInputEvent& event, std::vector<EvgetCore::Event::Data>& data, EvgetCore::Event::ButtonAction action);
        void keyEvent(const XInputEvent& event, std::vector<EvgetCore::Event::Data>& data);
        void motionEvent(const XInputEvent& event, std::vector<EvgetCore::Event::Data>& data);
        void scrollEvent(const XInputEvent& event, std::vector<EvgetCore::Event::Data>& data);

        std::reference_wrapper<XWrapper> xWrapper;
        std::reference_wrapper<XEventSwitchPointer> xEventSwitchPointer;
        std::reference_wrapper<XDeviceRefresh> xDeviceRefresh;

        std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scrollMap{};
        std::unordered_map<int, std::optional<int>> valuatorX{};
        std::unordered_map<int, std::optional<int>> valuatorY{};
        std::unordered_map<int, std::unordered_map<int, double>> valuatorValues{};
    };
}

#endif //EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
