#include "common/windows_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evgetwindows/event_transformer.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_query_api.h"
#include "evgetwindows/message_window.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows_query_api.h"

namespace test {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-union-access)

evgetwindows::RawEvent MakeMouseRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.lLastX = 0x1234;
    mouse.lLastY = 0x5678;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeKeyboardRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = static_cast<USHORT>(0x41);
    event.data = keyboard;
    return event;
}

evgetwindows::RawEvent MakeMouseMoveRelative(LONG last_x, LONG last_y) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usFlags = MOUSE_MOVE_RELATIVE;
    mouse.lLastX = last_x;
    mouse.lLastY = last_y;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseWheel(SHORT delta, bool horizontal) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usButtonFlags = static_cast<USHORT>(horizontal ? RI_MOUSE_HWHEEL : RI_MOUSE_WHEEL);
    mouse.usButtonData = static_cast<USHORT>(delta);
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseButton(USHORT button_flags) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usButtonFlags = button_flags;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseAbsolute(LONG last_x, LONG last_y) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usFlags = MOUSE_MOVE_ABSOLUTE;
    mouse.lLastX = last_x;
    mouse.lLastY = last_y;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeKeyboard(USHORT vkey, USHORT make_code, USHORT flags) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = vkey;
    keyboard.MakeCode = make_code;
    keyboard.Flags = flags;
    event.data = keyboard;
    return event;
}

evgetwindows::RawEvent MakeInjected(USHORT vkey) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    event.header.hDevice = nullptr;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = vkey;
    event.data = keyboard;
    return event;
}

RAWINPUT MakeMouseRawInput(LONG last_x, LONG last_y) {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEMOUSE;
    raw.data.mouse.lLastX = last_x;
    raw.data.mouse.lLastY = last_y;
    return raw;
}

RAWINPUT MakeKeyboardRawInput(USHORT vkey) {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEKEYBOARD;
    raw.data.keyboard.VKey = vkey;
    return raw;
}

RAWINPUT MakeHidRawInput() {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEHID;
    return raw;
}

namespace {

constexpr std::size_t kHidPacketHeaderSize = offsetof(RAWINPUT, data) + offsetof(RAWHID, bRawData);

int hid_device_backing = 0;

std::vector<std::byte>
BuildHidPacket(std::span<const std::byte> report, DWORD count, HANDLE device, bool understate_size, bool distinct) {
    const std::size_t packet_size = kHidPacketHeaderSize + (report.size() * count);
    std::vector<std::byte> packet(packet_size);

    auto* raw = reinterpret_cast<RAWINPUT*>(packet.data());
    raw->header.dwType = RIM_TYPEHID;
    raw->header.hDevice = device;
    raw->header.dwSize = static_cast<DWORD>(understate_size ? kHidPacketHeaderSize : packet_size);
    raw->data.hid.dwSizeHid = static_cast<DWORD>(report.size());
    raw->data.hid.dwCount = count;

    auto* reports = reinterpret_cast<std::byte*>(raw->data.hid.bRawData);
    std::vector<std::byte> slice{report.begin(), report.end()};
    for (DWORD index = 0; index < count; ++index) {
        if (distinct && !slice.empty()) {
            slice.front() = static_cast<std::byte>(index);
        }
        std::ranges::copy(slice, reports + (index * report.size()));
    }

    return packet;
}

} // namespace

std::vector<std::byte> MakeHidReportBytes(std::size_t size) {
    std::vector<std::byte> report(size);
    for (std::size_t index = 0; index < size; ++index) {
        report[index] = static_cast<std::byte>(index);
    }
    return report;
}

std::vector<std::byte> MakeHidPacket(std::span<const std::byte> report, DWORD count) {
    return BuildHidPacket(report, count, &hid_device_backing, false, false);
}

std::vector<std::byte> MakeHidPacketDistinct(std::size_t report_size, DWORD count) {
    const auto report = MakeHidReportBytes(report_size);
    return BuildHidPacket(report, count, &hid_device_backing, false, true);
}

std::vector<std::byte> MakeHidPacketNullDevice(std::span<const std::byte> report, DWORD count) {
    return BuildHidPacket(report, count, nullptr, false, false);
}

std::vector<std::byte> MakeHidPacketUndersized(std::span<const std::byte> report, DWORD count) {
    return BuildHidPacket(report, count, &hid_device_backing, true, false);
}

const RAWINPUT& AsRawInput(std::span<const std::byte> packet) {
    return *reinterpret_cast<const RAWINPUT*>(packet.data());
}

evgetwindows::RawEvent MakeHidRawEvent(std::span<const std::byte> report) {
    return MakeHidRawEventFrom(&hid_device_backing, report);
}

evgetwindows::RawEvent MakeHidRawEventFrom(HANDLE device, std::span<const std::byte> report) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEHID;
    event.header.hDevice = device;

    evgetwindows::HidPayload payload{};
    std::ranges::copy(report, payload.report.begin());
    payload.size = static_cast<std::uint16_t>(report.size());
    event.data = payload;

    return event;
}

evgetwindows::HidReport MakeHidReport(std::uint32_t contact_id, bool tip_down, bool confident) {
    return evgetwindows::HidReport{
        .contacts =
            {{.contact_id = contact_id,
              .position_x = std::nullopt,
              .position_y = std::nullopt,
              .tip_down = tip_down,
              .confident = confident}},
        .contact_count = 1,
        .button_one_down = false
    };
}

evgetwindows::HidAxisRange MakeAxisRange() {
    return evgetwindows::HidAxisRange{.min_x = 0, .max_x = kTestAxisMax, .min_y = 0, .max_y = kTestAxisMax};
}

evgetwindows::MonitorInfo MakeMappedMonitor() {
    return MakeMappedMonitorExtent(kTestMonitorWidth, kTestMonitorHeight);
}

evgetwindows::MonitorInfo MakeMappedMonitorExtent(double width, double height) {
    return evgetwindows::MonitorInfo{.name = std::string{kTestMappedDisplay}, .width = width, .height = height};
}

evgetwindows::HidContact MakeContact(std::uint32_t contact_id, std::int32_t position_x, std::int32_t position_y) {
    return evgetwindows::HidContact{
        .contact_id = contact_id,
        .position_x = position_x,
        .position_y = position_y,
        .tip_down = true,
        .confident = true
    };
}

evgetwindows::HidReport MakeHidReportFrom(std::vector<evgetwindows::HidContact> contacts) {
    const auto count = static_cast<std::uint32_t>(contacts.size());
    return evgetwindows::HidReport{.contacts = std::move(contacts), .contact_count = count, .button_one_down = false};
}

HANDLE HidDeviceHandle() {
    return &hid_device_backing;
}

HANDLE HidDeviceHandleAlternate() {
    static int backing = 0;
    return &backing;
}

HANDLE HidDeviceHandleAt(std::size_t index) {
    static std::array<int, evgetwindows::kMaxCachedDevices + 1> backing{};
    return &backing.at(index);
}

evgetwindows::HidContact MakeContactState(
    std::uint32_t contact_id,
    std::int32_t position_x,
    std::int32_t position_y,
    bool tip_down,
    bool confident
) {
    return evgetwindows::HidContact{
        .contact_id = contact_id,
        .position_x = position_x,
        .position_y = position_y,
        .tip_down = tip_down,
        .confident = confident
    };
}

evgetwindows::HidReport MakeHidFrame(std::vector<evgetwindows::HidContact> contacts, std::uint32_t contact_count) {
    return evgetwindows::HidReport{
        .contacts = std::move(contacts),
        .contact_count = contact_count,
        .button_one_down = false
    };
}

evgetwindows::HidReport MakeContactlessReport(bool button_one_down) {
    return evgetwindows::HidReport{.contacts = {}, .contact_count = 0, .button_one_down = button_one_down};
}

evgetwindows::RawEvent MakeDeviceChangeRawEvent(HANDLE device, bool arrival) {
    return evgetwindows::MessageWindow::ToDeviceChangeEvent(arrival ? GIDC_ARRIVAL : GIDC_REMOVAL, device);
}

void ExpectTouchParityColumns(const evget::Entry& entry, std::string_view device_column) {
    // The only two columns a libinput touch row differs from a windows one in.
    EXPECT_EQ(entry.Data().at(12), "RIM_TYPEHID");
    EXPECT_EQ(entry.Data().at(13), "windows");
    EXPECT_EQ(entry.Data().at(14), device_column);
    EXPECT_EQ(entry.Data().at(15), "7");
}

void ExpectTouchParityRows(
    const evget::Data& batch,
    std::string_view device_column,
    std::span<const TouchParityRow> expected
) {
    ASSERT_EQ(batch.Entries().size(), expected.size());

    for (std::size_t index = 0; index < expected.size(); ++index) {
        ExpectTouchParityRow(batch.Entries().at(index), expected[index], device_column);
    }
}

void ExpectTouchParityRow(const evget::Entry& entry, const TouchParityRow& row, std::string_view device_column) {
    EXPECT_EQ(entry.Type(), row.type);

    // A move row stops short of the action column, so each row is read at the index its type defines.
    if (row.type == evget::EntryType::kMouseClick) {
        EXPECT_EQ(entry.Data().at(18), row.action);
    } else {
        EXPECT_EQ(entry.Data().at(2), row.position);
    }

    ExpectTouchParityColumns(entry, device_column);
}

void ExpectTouchParitySequence(evget::DeviceType device_type, std::string_view device_column) {
    testing::NiceMock<WindowsQueryApiMock> query{};
    testing::NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(testing::Return(device_type));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(testing::Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(testing::Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(query, PointerMonitor()).WillRepeatedly(testing::Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(testing::Return(std::optional{MakeHidFrame({MakeContact(7, 0, 0)}, 1)}))
        .WillOnce(testing::Return(std::optional{MakeHidFrame({MakeContact(7, kTestAxisMax / 16, 0)}, 1)}))
        .WillOnce(
            testing::Return(std::optional{MakeHidFrame({MakeContactState(7, kTestAxisMax / 16, 0, false, true)}, 1)})
        );

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    const auto down = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto motion = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto lift = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    const std::array<TouchParityRow, 2> down_rows{
        {{.type = evget::EntryType::kMouseMove, .position = "", .action = ""},
         {.type = evget::EntryType::kMouseClick, .position = "", .action = "0"}}
    };
    // Held by name because the row only views it.
    const auto motion_position = evget::FromDouble(kTestMonitorWidth / 16);
    const std::array<TouchParityRow, 1> motion_rows{
        {{.type = evget::EntryType::kMouseMove, .position = motion_position, .action = ""}}
    };
    const std::array<TouchParityRow, 2> lift_rows{
        {{.type = evget::EntryType::kMouseMove, .position = "", .action = ""},
         {.type = evget::EntryType::kMouseClick, .position = "", .action = "1"}}
    };

    ExpectTouchParityRows(down, device_column, down_rows);
    ExpectTouchParityRows(motion, device_column, motion_rows);
    ExpectTouchParityRows(lift, device_column, lift_rows);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-union-access)

} // namespace test
