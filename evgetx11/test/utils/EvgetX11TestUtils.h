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

#ifndef EVGET_UTILS_H
#define EVGET_UTILS_H

#include "gmock/gmock.h"
#include "evgetx11/XWrapper.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/EventTransformerX11.h"

namespace EvgetX11TestUtils {
    class XWrapperMock : public EvgetX11::XWrapper {
    public:
        MOCK_METHOD(std::string, lookupCharacter, (const XIDeviceEvent& event, KeySym& keySym), (override));
        MOCK_METHOD(std::unique_ptr<unsigned char[]>, getDeviceButtonMapping, (int id, int mapSize), (override));
        MOCK_METHOD((std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>), listInputDevices, (int& ndevices), (override));
        MOCK_METHOD((std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>), queryDevice, (int& ndevices), (override));
        MOCK_METHOD((std::unique_ptr<char[], decltype(&XFree)>), atomName, (Atom atom), (override));
        MOCK_METHOD(XEvent, nextEvent, (), (override));
        MOCK_METHOD(XEventPointer, eventData, (XEvent& event), (override));
        MOCK_METHOD(Status, queryVersion, (int& major, int& minor), (override));
        MOCK_METHOD(void, selectEvents, (XIEventMask& mask), (override));
    };

    class XEventSwitchMock : public EvgetX11::XEventSwitch {
    public:
        MOCK_METHOD(bool, switchOnEvent, (const EvgetX11::XInputEvent &event, std::chrono::nanoseconds timestamp, EventData &data), (override));
        MOCK_METHOD(void, refreshDevices, (int id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info), (override));
    };
}

#endif //EVGET_UTILS_H