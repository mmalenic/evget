#include "common/windows_mock.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "evgetwindows/hid_frame.h"
#include "evgetwindows/raw_event.h"

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
BuildHidPacket(std::span<const std::byte> report, DWORD count, HANDLE device, bool understate_size) {
    const std::size_t packet_size = kHidPacketHeaderSize + (report.size() * count);
    std::vector<std::byte> packet(packet_size);

    auto* raw = reinterpret_cast<RAWINPUT*>(packet.data());
    raw->header.dwType = RIM_TYPEHID;
    raw->header.hDevice = device;
    raw->header.dwSize = static_cast<DWORD>(understate_size ? kHidPacketHeaderSize : packet_size);
    raw->data.hid.dwSizeHid = static_cast<DWORD>(report.size());
    raw->data.hid.dwCount = count;

    auto* reports = reinterpret_cast<std::byte*>(raw->data.hid.bRawData);
    for (DWORD index = 0; index < count; ++index) {
        std::ranges::copy(report, reports + (index * report.size()));
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
    return BuildHidPacket(report, count, &hid_device_backing, false);
}

std::vector<std::byte> MakeHidPacketNullDevice(std::span<const std::byte> report, DWORD count) {
    return BuildHidPacket(report, count, nullptr, false);
}

std::vector<std::byte> MakeHidPacketUndersized(std::span<const std::byte> report, DWORD count) {
    return BuildHidPacket(report, count, &hid_device_backing, true);
}

const RAWINPUT& AsRawInput(std::span<const std::byte> packet) {
    return *reinterpret_cast<const RAWINPUT*>(packet.data());
}

evgetwindows::RawEvent MakeHidRawEvent(std::span<const std::byte> report) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEHID;
    event.header.hDevice = &hid_device_backing;

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

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-union-access)

} // namespace test
