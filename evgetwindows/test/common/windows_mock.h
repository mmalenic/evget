#ifndef EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
#define EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H

#include <gmock/gmock.h>

#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <vector>

#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_query_api.h"
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
        (UINT key, UINT scan_code, (const std::array<BYTE, evgetwindows::kKeyStateSize>&)key_state),
        (override)
    );
    MOCK_METHOD((std::optional<std::string>), DeviceName, (HANDLE device), (override));
    MOCK_METHOD((std::optional<evgetwindows::FocusWindowInfo>), FocusWindow, (), (override));
    MOCK_METHOD((std::optional<std::string>), Screen, (), (override));
    MOCK_METHOD(bool, ToggleState, (int key), (override));
};

class HidQueryApiMock : public evgetwindows::HidQueryApi {
public:
    MOCK_METHOD(evget::DeviceType, ClassifyDevice, (HANDLE device), (override));
    MOCK_METHOD((std::optional<evgetwindows::HidAxisRange>), AxisRange, (HANDLE device), (override));
    MOCK_METHOD(
        (std::optional<evgetwindows::HidReport>),
        DecodeReport,
        (HANDLE device, (std::span<const std::byte>)report),
        (override)
    );
    MOCK_METHOD(void, RemoveDevice, (HANDLE device), (override));
};

evgetwindows::RawEvent MakeMouseRawEvent();
evgetwindows::RawEvent MakeKeyboardRawEvent();

evgetwindows::RawEvent MakeMouseMoveRelative(LONG last_x, LONG last_y);
evgetwindows::RawEvent MakeMouseWheel(SHORT delta, bool horizontal);
evgetwindows::RawEvent MakeMouseButton(USHORT button_flags);
evgetwindows::RawEvent MakeMouseAbsolute(LONG last_x, LONG last_y);
evgetwindows::RawEvent MakeKeyboard(USHORT vkey, USHORT make_code, USHORT flags);
evgetwindows::RawEvent MakeInjected(USHORT vkey);

RAWINPUT MakeMouseRawInput(LONG last_x, LONG last_y);
RAWINPUT MakeKeyboardRawInput(USHORT vkey);
RAWINPUT MakeHidRawInput();

std::vector<std::byte> MakeHidReportBytes(std::size_t size);

std::vector<std::byte> MakeHidPacket(std::span<const std::byte> report, DWORD count);
std::vector<std::byte> MakeHidPacketNullDevice(std::span<const std::byte> report, DWORD count);
std::vector<std::byte> MakeHidPacketUndersized(std::span<const std::byte> report, DWORD count);

const RAWINPUT& AsRawInput(std::span<const std::byte> packet);

evgetwindows::RawEvent MakeHidRawEvent(std::span<const std::byte> report);
evgetwindows::HidReport MakeHidReport(std::uint32_t contact_id, bool tip_down, bool confident);

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
