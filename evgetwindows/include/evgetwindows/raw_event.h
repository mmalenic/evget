/**
 * \file raw_event.h
 * \brief Owned raw input event that can cross a channel by value.
 */

#ifndef EVGETWINDOWS_RAW_EVENT_H
#define EVGETWINDOWS_RAW_EVENT_H

#include <windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <variant>

namespace evgetwindows {

/**
 * \brief Capacity for HID input report.
 */
constexpr std::size_t kHidReportCapacity = 256;

/**
 * \brief HID input report.
 */
struct HidPayload {
    std::array<std::byte, kHidReportCapacity> report;
    std::uint16_t size;
};

/**
 * \brief Owned raw input event that can cross a boost channel and is trivially copyable.
 */
struct RawEvent {
    RAWINPUTHEADER header{};
    std::variant<RAWMOUSE, RAWKEYBOARD, HidPayload> data;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_RAW_EVENT_H
