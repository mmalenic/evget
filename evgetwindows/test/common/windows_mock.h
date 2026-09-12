#ifndef EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
#define EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H

#include <gmock/gmock.h>

#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <array>
#include <optional>
#include <string>
#include <tuple>

#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"
#include "evgetwindows/windows_query_api.h"

namespace test {

class WindowsApiMock : public evgetwindows::WindowsApi {
public:
    MOCK_METHOD(evget::Result<void>, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(
        (boost::asio::awaitable<std::tuple<boost::system::error_code, evgetwindows::RawEvent>>),
        ReceiveNext,
        (),
        (override)
    );
};

class WindowsQueryApiMock : public evgetwindows::WindowsQueryApi {
public:
    MOCK_METHOD(
        (std::optional<std::string>),
        CharacterFor,
        (UINT vk, UINT scan_code, (const std::array<BYTE, evgetwindows::kKeyStateSize>&)key_state),
        (override)
    );
    MOCK_METHOD((std::optional<std::string>), DeviceName, (HANDLE device), (override));
    MOCK_METHOD((std::optional<evgetwindows::FocusWindowInfo>), FocusWindow, (), (override));
    MOCK_METHOD((std::optional<std::string>), Screen, (), (override));
    MOCK_METHOD(bool, ToggleState, (int vk), (override));
};

evgetwindows::RawEvent MakeMouseRawEvent();
evgetwindows::RawEvent MakeKeyboardRawEvent();

evgetwindows::RawEvent MakeMouseMoveRelative(LONG dx, LONG dy);
evgetwindows::RawEvent MakeMouseWheel(SHORT delta, bool horizontal);
evgetwindows::RawEvent MakeMouseButton(USHORT button_flags);
evgetwindows::RawEvent MakeMouseAbsolute(LONG x, LONG y);
evgetwindows::RawEvent MakeKeyboard(USHORT vkey, USHORT make_code, USHORT flags);
evgetwindows::RawEvent MakeInjected(USHORT vkey);

RAWINPUT MakeMouseRawInput(LONG last_x, LONG last_y);
RAWINPUT MakeKeyboardRawInput(USHORT vkey);
RAWINPUT MakeHidRawInput();

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
