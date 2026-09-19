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
#include <string_view>
#include <tuple>
#include <vector>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_query_api.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"
#include "evgetwindows/windows_query_api.h"

namespace test {

constexpr std::int32_t kTestAxisMax = 4096;
constexpr double kTestMonitorWidth = 1920.0;
constexpr double kTestMonitorHeight = 1280.0;
constexpr std::string_view kTestPointerDisplay{R"(\\.\DISPLAY1)"};
constexpr std::string_view kTestMappedDisplay{R"(\\.\DISPLAY2)"};

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
    MOCK_METHOD((std::optional<evgetwindows::MonitorInfo>), MappedMonitor, (HANDLE device), (override));
    MOCK_METHOD((std::optional<evgetwindows::MonitorInfo>), PointerMonitor, (), (override));
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
std::vector<std::byte> MakeHidPacketDistinct(std::size_t report_size, DWORD count);
std::vector<std::byte> MakeHidPacketNullDevice(std::span<const std::byte> report, DWORD count);
std::vector<std::byte> MakeHidPacketUndersized(std::span<const std::byte> report, DWORD count);

const RAWINPUT& AsRawInput(std::span<const std::byte> packet);

evgetwindows::RawEvent MakeHidRawEvent(std::span<const std::byte> report);
evgetwindows::HidReport MakeHidReport(std::uint32_t contact_id, bool tip_down, bool confident);

evgetwindows::HidAxisRange MakeAxisRange();
evgetwindows::MonitorInfo MakeMappedMonitor();
evgetwindows::HidContact MakeContact(std::uint32_t contact_id, std::int32_t position_x, std::int32_t position_y);
evgetwindows::HidReport MakeHidReportFrom(std::vector<evgetwindows::HidContact> contacts);

HANDLE HidDeviceHandle();
HANDLE HidDeviceHandleAlternate();

/**
 * \brief A unique device handle for each index.
 * \param index the device index
 * \return the handle
 */
HANDLE HidDeviceHandleAt(std::size_t index);

/// \brief Create a HID report event for a device.
evgetwindows::RawEvent MakeHidRawEventFrom(HANDLE device, std::span<const std::byte> report);

/// \brief Create a contact at a position.
evgetwindows::HidContact MakeContactState(
    std::uint32_t contact_id,
    std::int32_t position_x,
    std::int32_t position_y,
    bool tip_down,
    bool confident
);

/// \brief Create a report with the contacts.
evgetwindows::HidReport MakeHidFrame(std::vector<evgetwindows::HidContact> contacts, std::uint32_t contact_count);

/// \brief Create a report carrying no contacts.
evgetwindows::HidReport MakeContactlessReport(bool button_one_down);

evgetwindows::RawEvent MakeDeviceChangeRawEvent(HANDLE device, bool arrival);

/// \brief The libinput touch row equivalent.
struct TouchParityRow {
    evget::EntryType type;
    std::string_view position;
    std::string_view action;
};

/**
 * \brief Assert touch rows are the libinput shape.
 * \param batch the rows of the batch
 * \param device_column the device type column
 * \param expected the row shape
 */
void ExpectTouchParityRows(
    const evget::Data& batch,
    std::string_view device_column,
    std::span<const TouchParityRow> expected
);

/**
 * \brief Assert a touch row has the libinput shape.
 * \param entry the row
 * \param row the row shape
 * \param device_column the device type column
 */
void ExpectTouchParityRow(const evget::Entry& entry, const TouchParityRow& row, std::string_view device_column);

/**
 * \brief Assert a touch row has the columns of a libinput row.
 * \param entry the row
 * \param device_column the device type column value
 */
void ExpectTouchParityColumns(const evget::Entry& entry, std::string_view device_column);

/**
 * \brief Assert a down, motion and up sequence as a test.
 * \param device_type the class of the device
 * \param device_column the device type column
 */
void ExpectTouchParitySequence(evget::DeviceType device_type, std::string_view device_column);

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
