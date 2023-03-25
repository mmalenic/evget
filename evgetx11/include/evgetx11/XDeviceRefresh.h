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

#include <X11/extensions/XInput2.h>

#include <map>
#include <unordered_map>

#include "XInputEvent.h"

namespace EvgetX11 {
using EventData = EvgetCore::Event::Data;

class XDeviceRefresh {
public:
    XDeviceRefresh() = default;

    /**
     * Switch on the event type and add relevant data to the event data.
     * Returns true if event was successfully consumed.
     */
    bool switchOnEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, EventData& data);
    void refreshDevices(int id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info);

    bool containsDevice(int id);
    EvgetCore::Event::Device getDevice(int id) const;
    const std::string& getNameFromId(int id) const;
    const std::string& getEvtypeName(int evtype) const;

    void setDevice(int id, EvgetCore::Event::Device device);
    void setNameFromId(int id, const std::string& name);
    void setEvtypeName(int evtype, const std::string& name);

    EvgetCore::Event::Data createSystemData(const XIDeviceEvent& event, const std::string& name);
    //        static EvgetCore::Event::FieldDefinition::Entries createValuatorEntries(const XIValuatorState&
    //        valuatorState); static EvgetCore::Event::FieldDefinition::Entries createButtonEntries(const XIDeviceEvent&
    //        event);

    static std::map<int, int> getValuators(const XIValuatorState& valuatorState);
    static std::string formatValue(int value);

private:
    std::unordered_map<int, EvgetCore::Event::Device> devices{};
    std::unordered_map<int, std::string> idToName{};
    std::unordered_map<int, std::string> evtypeToName{};
};
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_SRC_XEVENTSWITCH_H
