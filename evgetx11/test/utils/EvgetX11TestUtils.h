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

#ifndef EVGET_UTILS_H
#define EVGET_UTILS_H

#include <gmock/gmock.h>

#include <X11/Xlib.h>

#include <optional>

#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XWrapper.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
namespace EvgetX11TestUtils {
class XWrapperMock : public EvgetX11::XWrapper {
public:
    using XWindowDimensions = EvgetX11::XWindowDimensions;
    using XEventPointer = EvgetX11::XEventPointer;
    using QueryPointerResult = EvgetX11::QueryPointerResult;

    MOCK_METHOD(
        std::string,
        lookupCharacter,
        (const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& keySym),
        (override)
    );
    MOCK_METHOD(QueryPointerResult, query_pointer, (int device_id), (override));
    MOCK_METHOD(std::unique_ptr<unsigned char[]>, getDeviceButtonMapping, (int device_id, int mapSize), (override));
    MOCK_METHOD(
        (std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>),
        listInputDevices,
        (int& ndevices),
        (override)
    );
    MOCK_METHOD(
        (std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>),
        queryDevice,
        (int& ndevices),
        (override)
    );
    MOCK_METHOD((std::unique_ptr<char[], decltype(&XFree)>), atomName, (Atom atom), (override));
    MOCK_METHOD(std::optional<std::string>, getWindowName, (Window window), (override));
    MOCK_METHOD(std::optional<Window>, getActiveWindow, (), (override));
    MOCK_METHOD(std::optional<Window>, getFocusWindow, (), (override));
    MOCK_METHOD(std::optional<XWindowDimensions>, getWindowSize, (Window window), (override));
    MOCK_METHOD(std::optional<XWindowDimensions>, getWindowPosition, (Window window), (override));

    MOCK_METHOD(XEvent, nextEvent, (), (override));
    MOCK_METHOD(XEventPointer, eventData, (XEvent & event), (override));
    MOCK_METHOD(Status, queryVersion, (int& major, int& minor), (override));
    MOCK_METHOD(void, selectEvents, (XIEventMask & mask), (override));
};

XIValuatorClassInfo createXIValuatorClassInfo();
XIScrollClassInfo createXIScrollClassInfo();
XIButtonClassInfo createXIButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask);
XIDeviceInfo createXIDeviceInfo(std::array<XIAnyClassInfo*, 3>& info, std::span<char> name);
XDeviceInfo createXDeviceInfo();
XIRawEvent createXIRawEvent(int evtype, std::array<unsigned char, 1>& valuatorMask, std::array<double, 1>& values);
XEvent createXEvent(XIRawEvent& event);
XIValuatorState createXIValuatorState(std::array<unsigned char, 1>& valuatorMask, std::array<double, 1>& values);
EvgetX11::QueryPointerResult create_pointer_result();

void set_x_wrapper_event_mocks(EvgetX11TestUtils::XWrapperMock& xWrapperMock, XIRawEvent& device_event, XEvent& xEvent);
void set_x_wrapper_key_mocks(EvgetX11TestUtils::XWrapperMock& xWrapperMock, XIRawEvent& device_event, XEvent& xEvent);
void set_x_wrapper_mocks(EvgetX11TestUtils::XWrapperMock& xWrapperMock);
}  // namespace EvgetX11TestUtils

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

#endif  // EVGET_UTILS_H
