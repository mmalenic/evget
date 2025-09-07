#ifndef EVGETX11_TEST_UTILS_EVGETX11TESTUTILS_H
#define EVGETX11_TEST_UTILS_EVGETX11TESTUTILS_H

#include <gmock/gmock.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <array>
#include <memory>
#include <optional>
#include <span>
#include <string>

#include "evgetx11/x11_api.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
namespace test {
class X11ApiMock : public evgetx11::X11Api {
public:
    using XWindowDimensions = evgetx11::XWindowDimensions;
    using XEventPointer = evgetx11::XEventPointer;
    using QueryPointerResult = evgetx11::QueryPointerResult;

    MOCK_METHOD(
        std::string,
        LookupCharacter,
        (const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& key_sym),
        (override)
    );
    MOCK_METHOD(QueryPointerResult, QueryPointer, (int device_id), (override));
    MOCK_METHOD(std::unique_ptr<unsigned char[]>, GetDeviceButtonMapping, (int device_id, int map_size), (override));
    MOCK_METHOD(
        (std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>),
        ListInputDevices,
        (int& n_devices),
        (override)
    );
    MOCK_METHOD(
        (std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>),
        QueryDevice,
        (int& n_devices),
        (override)
    );
    MOCK_METHOD((std::unique_ptr<char[], decltype(&XFree)>), AtomName, (Atom atom), (override));
    MOCK_METHOD(std::optional<std::string>, GetWindowName, (Window window), (override));
    MOCK_METHOD(std::optional<Window>, GetActiveWindow, (), (override));
    MOCK_METHOD(std::optional<Window>, GetFocusWindow, (), (override));
    MOCK_METHOD(std::optional<XWindowDimensions>, GetWindowSize, (Window window), (override));
    MOCK_METHOD(std::optional<XWindowDimensions>, GetWindowPosition, (Window window), (override));

    MOCK_METHOD(XEvent, NextEvent, (), (override));
    MOCK_METHOD(XEventPointer, EventData, (XEvent & event), (override));
    MOCK_METHOD(Status, QueryVersion, (int& major, int& minor), (override));
    MOCK_METHOD(void, SelectEvents, (XIEventMask & mask), (override));
};

XIValuatorClassInfo CreateXiValuatorClassInfo();
XIScrollClassInfo CreateXiScrollClassInfo();
XIButtonClassInfo CreateXiButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask);
XIDeviceInfo CreateXiDeviceInfo(std::array<XIAnyClassInfo*, 3>& info, std::span<char> name);
XDeviceInfo CreateXDeviceInfo();
XIRawEvent CreateXiRawEvent(int evtype, std::array<unsigned char, 1>& valuator_mask, std::array<double, 1>& values);
XEvent CreateXEvent(XIRawEvent& event);
XIValuatorState CreateXiValuatorState(std::array<unsigned char, 1>& valuator_mask, std::array<double, 1>& values);
evgetx11::QueryPointerResult CreatePointerResult();

void SetXWrapperEventMocks(X11ApiMock& x_wrapper_mock, XIRawEvent& device_event, XEvent& x_event);
void SetXWrapperKeyMocks(X11ApiMock& x_wrapper_mock, XIRawEvent& device_event, XEvent& x_event);
void SetXWrapperMocks(X11ApiMock& x_wrapper_mock);
} // namespace test

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

#endif
