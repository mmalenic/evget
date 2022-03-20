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

        /**defaul
         * Switch on the event type and add relevant data to the event data.
         * Returns true if event was successfully consumed.
         */
        virtual bool switchOnEvent(const XInputEvent &event, std::chrono::nanoseconds timestamp, EventTransformerLinux::EventData &data) = 0;
        virtual void refreshDevices(int id, EvgetCore::Event::Common::Device device, const std::string& name, const XIDeviceInfo& info);

        virtual ~XEventSwitch() = default;

        static std::unique_ptr<char[], decltype(&XFree)> getAtomName(Display& display, Atom atom);

    protected:
        XEventSwitch(XEventSwitch&&) noexcept = default;
        XEventSwitch& operator=(XEventSwitch&&) noexcept = default;

        XEventSwitch(const XEventSwitch&) = default;
        XEventSwitch& operator=(const XEventSwitch&) = default;

        std::unique_ptr<EvgetCore::Event::AbstractData> createSystemDataWithoutRoot(const XIDeviceEvent& event, const std::string& deviceName);
        std::unique_ptr<EvgetCore::Event::AbstractData> createSystemDataWithRoot(const XIDeviceEvent& event, const std::string& deviceName);
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> createSystemData(const XIDeviceEvent& event);
        static EvgetCore::Event::AbstractField::Entries createValuatorEntries(const XIValuatorState& valuatorState);
        static EvgetCore::Event::AbstractField::Entries createButtonEntries(const XIDeviceEvent& event);

        static void addTableData(std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, std::unique_ptr<EvgetCore::Event::AbstractData> genericData, std::unique_ptr<EvgetCore::Event::AbstractData> systemData);
        static void getMasks(const unsigned char* mask, int maskLen, EvgetCore::Util::Invocable<void, int> auto&& function);

        std::unique_ptr<EvgetCore::Event::AbstractData> createRawData(const XIRawEvent& event);

        static std::map<int, int> getValuators(const XIValuatorState& valuatorState);
        static std::string formatValue(int value);

        std::map<int, EvgetCore::Event::Common::Device> devices{};
        std::map<int, std::string> idToName{};
    };

    void EvgetX11::XEventSwitch::getMasks(const unsigned char* mask, int maskLen, EvgetCore::Util::Invocable<void, int> auto&& function) {
        for (int i = 0; i < maskLen * 8; i++) {
            if (XIMaskIsSet(mask, i)) {
                function(i);
            }
        }
    }
}

#endif //EVGET_EVGETX11_SRC_XEVENTSWITCH_H
