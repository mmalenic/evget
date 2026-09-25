/**
 * \file hid_frame.h
 * \brief Types describing a HID report.
 */

#ifndef EVGETWINDOWS_HID_FRAME_H
#define EVGETWINDOWS_HID_FRAME_H

#include <cstdint>
#include <optional>
#include <vector>

namespace evgetwindows {

/**
 * \brief The HID contact.
 */
struct HidContact {
    std::uint32_t contact_id{};
    std::optional<std::int32_t> position_x;
    std::optional<std::int32_t> position_y;
    bool tip_down{};
    bool confident{};
};

/**
 * \brief Decoded HID input report.
 */
struct HidReport {
    std::vector<HidContact> contacts;
    std::optional<std::uint32_t> contact_count;
    bool button_one_down{};
};

/**
 * \brief The axis for a report.
 */
struct HidAxisRange {
    std::int32_t min_x{};
    std::int32_t max_x{};
    std::int32_t min_y{};
    std::int32_t max_y{};
};

/**
 * \brief The device caps which derive a report descriptor.
 */
struct HidDeviceCaps {
    std::vector<std::uint32_t> contact_collections;
    HidAxisRange axis;
    bool axis_valid{};
    bool reports_touch_valid{};
    std::uint32_t digitizer_usage_list_length{};
    std::uint32_t button_usage_list_length{};
    std::uint32_t input_report_byte_length{};
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_HID_FRAME_H
