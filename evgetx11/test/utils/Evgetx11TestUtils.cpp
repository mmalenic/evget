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

#include "EvgetX11TestUtils.h"

#include <array>

XIValuatorClassInfo EvgetX11TestUtils::createXIValuatorClassInfo() {
    return {
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 1,
        .label = 1,
        .min = 0,
        .max = 0,
        .value = 1,
        .resolution = 1,
        .mode = XIModeAbsolute};
}

XIScrollClassInfo EvgetX11TestUtils::createXIScrollClassInfo() {
    return {
        .type = XIScrollClass,
        .sourceid = 1,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 1,
        .flags = 0};
}

XIButtonClassInfo
EvgetX11TestUtils::createXIButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask) {
    return {
        .type = XIButtonClass,
        .sourceid = 1,
        .num_buttons = static_cast<int>(labels.size()),
        .labels = labels.data(),
        .state =
            {
                .mask_len = static_cast<int>(mask.size()),
                .mask = mask.data(),
            },
    };
}

XIDeviceInfo EvgetX11TestUtils::createXIDeviceInfo(std::array<XIAnyClassInfo*, 3>& info, char name[]) {
    return {
        .deviceid = 1,
        .name = name,
        .use = XIMasterPointer,
        .attachment = 0,
        .enabled = true,
        .num_classes = static_cast<int>(info.size()),
        .classes = info.data()};
}

XDeviceInfo EvgetX11TestUtils::createXDeviceInfo() {
    return {
        .id = 1,
        .type = 1,
        .name = nullptr,
        .num_classes = 0,
        .use = IsXExtensionPointer,
        .inputclassinfo = nullptr};
}

XIDeviceEvent EvgetX11TestUtils::createXIDeviceEvent(
    int evtype,
    std::array<unsigned char, 1>& buttonMask,
    std::array<unsigned char, 1>& valuatorMask,
    std::array<double, 1>& values
) {
    return {
        .type = GenericEvent,
        .serial = 1,
        .send_event = false,
        .display = nullptr,
        .extension = 0,
        .evtype = evtype,
        .time = 1,
        .deviceid = 1,
        .sourceid = 1,
        .detail = 0,
        .root = 0,
        .event = 0,
        .child = 0,
        .root_x = 1,
        .root_y = 1,
        .event_x = 1,
        .event_y = 1,
        .flags = 0,
        .buttons =
            {
                .mask_len = static_cast<int>(buttonMask.size()),
                .mask = buttonMask.data(),
            },
        .valuators = createXIValuatorState(valuatorMask, values),
        .mods = {0, 0, 0, 0},
        .group = {0, 0, 0, 0},
    };
}

XEvent EvgetX11TestUtils::createXEvent(XIDeviceEvent& event) {
    return {
        .xcookie = {
            .type = GenericEvent,
            .serial = 0,
            .send_event = false,
            .display = nullptr,
            .extension = 0,
            .evtype = event.evtype,
            .cookie = 0,
            .data = &event,
        }};
}

XIValuatorState
EvgetX11TestUtils::createXIValuatorState(std::array<unsigned char, 1>& valuatorMask, std::array<double, 1>& values) {
    return {.mask_len = static_cast<int>(valuatorMask.size()), .mask = valuatorMask.data(), .values = values.data()};
}