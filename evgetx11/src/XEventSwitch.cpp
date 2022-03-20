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

#include <spdlog/spdlog.h>
#include "evgetx11/XEventSwitch.h"
#include "evgetcore/Event/Field.h"
#include "evgetcore/UnsupportedOperationException.h"
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <boost/numeric/conversion/cast.hpp>

EvgetX11::XEventSwitch::XEventSwitch(Display& display) : display{display} {
}


EvgetCore::Event::AbstractField::Entries EvgetX11::XEventSwitch::createButtonEntries(const XIDeviceEvent& event) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractData>> data{};

    getMasks(event.buttons.mask, event.buttons.mask_len, [&data](int mask) {
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ButtonActive", std::to_string(mask)));
        data.emplace_back(std::make_unique<EvgetCore::Event::Data>("ButtonState", std::move(fields)));
    });

    return data;
}

EvgetCore::Event::AbstractField::Entries EvgetX11::XEventSwitch::createValuatorEntries(const XIValuatorState& valuatorState) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractData>> data{};

    auto values = valuatorState.values;
    getMasks(valuatorState.mask, valuatorState.mask_len, [&data, &values](int mask) {
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Valuator", std::to_string(mask)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Value", std::to_string(*values++)));
        data.emplace_back(std::make_unique<EvgetCore::Event::Data>("Valuators", std::move(fields)));
    });

    return data;
}

std::unique_ptr<EvgetCore::Event::AbstractData> EvgetX11::XEventSwitch::createSystemDataWithRoot(
    const XIDeviceEvent& event,
    const std::string& deviceName,
    const std::string& dataName
) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields = createSystemData(event, deviceName);
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootX", std::to_string(event.root_x)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootY", std::to_string(event.root_y)));
    return std::make_unique<EvgetCore::Event::Data>(dataName, std::move(fields));
}

std::unique_ptr<EvgetCore::Event::AbstractData> EvgetX11::XEventSwitch::createSystemDataWithoutRoot(
    const XIDeviceEvent& event,
    const std::string& deviceName,
    const std::string& dataName
) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields = createSystemData(event, deviceName);
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootX"));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("RootY"));
    return std::make_unique<EvgetCore::Event::Data>(dataName, std::move(fields));
}

std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> EvgetX11::XEventSwitch::createSystemData(const XIDeviceEvent& event, const std::string& deviceName) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceName", deviceName));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("SourceId", std::to_string(event.sourceid)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Flags", formatValue(event.flags)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ButtonState", createButtonEntries(event)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Valuators", createValuatorEntries(event.valuators)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ModifiersBase", formatValue(event.mods.base)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ModifiersEffective", formatValue(event.mods.effective)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ModifiersLatched", formatValue(event.mods.latched)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("ModifiersLocked", formatValue(event.mods.locked)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("GroupBase", formatValue(event.group.base)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("GroupEffective", formatValue(event.group.effective)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("GroupLatched", formatValue(event.group.latched)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("GroupLocked", formatValue(event.group.locked)));

    return fields;
}

void EvgetX11::XEventSwitch::addTableData(
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
    std::unique_ptr<EvgetCore::Event::AbstractData> genericData,
    std::unique_ptr<EvgetCore::Event::AbstractData> systemData
) {
    data.emplace_back(EvgetCore::Event::TableData::TableDataBuilder{}.genericData(std::move(genericData)).systemData(std::move(systemData)).build());
}

std::unique_ptr<EvgetCore::Event::AbstractData> EvgetX11::XEventSwitch::createRawData(
    const XIRawEvent& event,
    const std::string& deviceName
) {
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceName", deviceName));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("EventType", std::to_string(event.evtype)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("XInputTime", std::to_string(event.time)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceId", std::to_string(event.deviceid)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("SourceId", std::to_string(event.sourceid)));
    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Detail", std::to_string(event.detail)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Flags", formatValue(event.flags)));

    fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Valuators", createValuatorEntries(event.valuators)));

    return std::make_unique<EvgetCore::Event::Data>("RawEvent", std::move(fields));
}

std::string EvgetX11::XEventSwitch::formatValue(int value) {
    return value != 0 ? std::to_string(value) : "";
}

std::map<int, int> EvgetX11::XEventSwitch::getValuators(const XIValuatorState& valuatorState) {
    std::map<int, int> valuators{};
    auto* values = valuatorState.values;
    getMasks(valuatorState.mask, valuatorState.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, *values++);
    });
    return valuators;
}

void EvgetX11::XEventSwitch::refreshDevices() {
    int nDevices;
    int xi2NDevices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>(XListInputDevices(&display.get(), &nDevices),
        XFreeDeviceList);
    auto xi2Info = std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>(XIQueryDevice(&display.get(), XIAllDevices, &xi2NDevices),
        XIFreeDeviceInfo);

    if (nDevices != xi2NDevices) {
        spdlog::warn("Devices with ids greater than 127 found. Set the device of these devices manually if their use is required.");
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2Devices{};
    for (int i = 0; i < xi2NDevices; i++) {
        xi2Devices.emplace(xi2Info[i].deviceid, xi2Info[i]);
    }

    for (int i = 0; i < nDevices; i++) {
        const auto& device = info[i];
        int id = boost::numeric_cast<int>(device.id);

        if (!xi2Devices.contains(id)) {
            throw EvgetCore::UnsupportedOperationException{"Device id from XDeviceInfo not found in XIDeviceInfo."};
        }
        const auto& xi2Device = xi2Devices.at(id).get();

        if (xi2Device.enabled && device.type != None && (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice)) {
            auto type = getAtomName(device.type);

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                devices.emplace(id, EvgetCore::Event::Common::Device::Mouse);
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                devices.emplace(id, EvgetCore::Event::Common::Device::Keyboard);
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                devices.emplace(id, EvgetCore::Event::Common::Device::Touchscreen);
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                devices.emplace(id, EvgetCore::Event::Common::Device::Touchpad);
            } else {
                spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
                continue;
            }

            idToName.emplace(id, device.name);
            setInfo(xi2Devices.at(id).get());
        }
    }
}

std::unique_ptr<char[], decltype(&XFree)> EvgetX11::XEventSwitch::getAtomName(Atom atom) {
    return {XGetAtomName(&display.get(), atom), XFree};
}

void EvgetX11::XEventSwitch::setInfo(const XIDeviceInfo& info) {
}
