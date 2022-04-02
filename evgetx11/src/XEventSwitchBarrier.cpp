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

#include "evgetx11/XEventSwitchBarrier.h"
#include "evgetcore/Event/Field.h"
#include "evgetcore/Event/Common/Time.h"

bool EvgetX11::XEventSwitchBarrier::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EvgetX11::XEventSwitch::EventData& data
) {
    switch (event.getEventType()) {
    case XI_BarrierHit:
    case XI_BarrierLeave:
        barrierEvent(event, timestamp, data);
        return true;
    default:
        return false;
    }
}

EvgetX11::XEventSwitchBarrier::XEventSwitchBarrier() {
    evtypeToName.emplace(XI_BarrierHit, "BarrierHit");
    evtypeToName.emplace(XI_BarrierLeave, "BarrierLeave");
}

void EvgetX11::XEventSwitchBarrier::barrierEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data
) {
    auto deviceEvent = event.viewData<XIBarrierEvent>();
    if (devices.contains(deviceEvent.deviceid)) {
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};

        fields.emplace_back(std::make_unique<EvgetCore::Event::Common::Time>(timestamp));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceName", idToName[deviceEvent.deviceid]));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("EventTypeId", std::to_string(deviceEvent.evtype)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("EventTypeName", evtypeToName[deviceEvent.evtype]));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("XInputTime", std::to_string(deviceEvent.time)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceId", std::to_string(deviceEvent.deviceid)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("SourceId", std::to_string(deviceEvent.sourceid)));

        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Dx", std::to_string(deviceEvent.dx)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Dy", std::to_string(deviceEvent.dy)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DTime", std::to_string(deviceEvent.dtime)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Barrier", std::to_string(deviceEvent.barrier)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("EventId", std::to_string(deviceEvent.eventid)));

        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootX", std::to_string(deviceEvent.root_x)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootY", std::to_string(deviceEvent.root_x)));

        addTableData(data, {}, std::make_unique<EvgetCore::Event::Data>("BarrierEventSystemData", std::move(fields)));
    }
}
