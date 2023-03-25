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

#include "evgetx11/XDeviceRefresh.h"

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>

#include "evgetcore/UnsupportedOperationException.h"

// EvgetCore::Event::FieldDefinition::Entries EvgetX11::XDeviceRefresh::createButtonEntries(const XIDeviceEvent& event)
// {
//     EvgetCore::Event::FieldDefinition::Entries entries{};
//
//     EvgetX11::XWrapperX11::onMasks(event.buttons.mask, event.buttons.mask_len, [&entries](int mask) {
//         EvgetCore::Event::Data data{"ButtonState"};
//         data.addField("ButtonActive", std::to_string(mask));
//         entries.emplace_back(data);
//     });
//
//     return entries;
// }
//
// EvgetCore::Event::FieldDefinition::Entries EvgetX11::XDeviceRefresh::createValuatorEntries(const XIValuatorState&
// valuatorState) {
//     EvgetCore::Event::FieldDefinition::Entries entries{};
//
//     auto values = valuatorState.values;
//     EvgetX11::XWrapperX11::onMasks(valuatorState.mask, valuatorState.mask_len, [&entries, &values](int mask) {
//         EvgetCore::Event::Data data{"Valuators"};
//         data.addField("Valuator", std::to_string(mask));
//         data.addField("Value", std::to_string(*values++));
//         entries.emplace_back(data);
//     });
//
//     return entries;
// }

EvgetCore::Event::Data EvgetX11::XDeviceRefresh::createSystemData(const XIDeviceEvent& event, const std::string& name) {
    EvgetCore::Event::Data data{};

    //    data.addField("DeviceName", idToName[event.deviceid]);
    //    data.addField("EventTypeId", std::to_string(event.evtype));
    //    data.addField("EventTypeName", evtypeToName[event.evtype]);
    //    data.addField("XInputTime", std::to_string(event.time));
    //    data.addField("DeviceId", std::to_string(event.deviceid));
    //    data.addField("SourceId", std::to_string(event.sourceid));
    //
    //    data.addField("Flags", formatValue(event.flags));
    //
    //    data.addField({"ButtonState", createButtonEntries(event)});
    //
    //    data.addField({"Valuators", createValuatorEntries(event.valuators)});
    //
    //    data.addField("ModifiersBase", formatValue(event.mods.base));
    //    data.addField("ModifiersEffective", formatValue(event.mods.effective));
    //    data.addField("ModifiersLatched", formatValue(event.mods.latched));
    //    data.addField("ModifiersLocked", formatValue(event.mods.locked));
    //
    //    data.addField("GroupBase", formatValue(event.group.base));
    //    data.addField("GroupEffective", formatValue(event.group.effective));
    //    data.addField("GroupLatched", formatValue(event.group.latched));
    //    data.addField("GroupLocked", formatValue(event.group.locked));

    return data;
}

std::string EvgetX11::XDeviceRefresh::formatValue(int value) {
    return value != 0 ? std::to_string(value) : "";
}

std::map<int, int> EvgetX11::XDeviceRefresh::getValuators(const XIValuatorState& valuatorState) {
    std::map<int, int> valuators{};
    auto* values = valuatorState.values;
    EvgetX11::XWrapperX11::onMasks(valuatorState.mask, valuatorState.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, *values++);
    });
    return valuators;
}

void EvgetX11::XDeviceRefresh::refreshDevices(
    int id,
    EvgetCore::Event::Device device,
    const std::string& name,
    const XIDeviceInfo& _
) {
    devices.emplace(id, device);
    idToName.emplace(id, name);
}

EvgetCore::Event::Device EvgetX11::XDeviceRefresh::getDevice(int id) const {
    return devices.at(id);
}

const std::string& EvgetX11::XDeviceRefresh::getNameFromId(int id) const {
    return idToName.at(id);
}

const std::string& EvgetX11::XDeviceRefresh::getEvtypeName(int evtype) const {
    return evtypeToName.at(evtype);
}

void EvgetX11::XDeviceRefresh::setDevice(int id, EvgetCore::Event::Device device) {
    devices.emplace(id, device);
}

void EvgetX11::XDeviceRefresh::setNameFromId(int id, const std::string& name) {
    idToName.emplace(id, name);
}

void EvgetX11::XDeviceRefresh::setEvtypeName(int evtype, const std::string& name) {
    evtypeToName.emplace(evtype, name);
}

bool EvgetX11::XDeviceRefresh::containsDevice(int id) {
    return devices.contains(id);
}

bool EvgetX11::XDeviceRefresh::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EvgetX11::EventData& data
) {
    return false;
}
