/**
 * \file hid_frame.h
 * \brief Value types describing a decoded HID report.
 */

#ifndef EVGETWINDOWS_HID_FRAME_H
#define EVGETWINDOWS_HID_FRAME_H

#include <cstdint>
#include <optional>
#include <vector>

namespace evgetwindows {

/**
 * \brief A single contact as the device reported it.
 */
struct HidContact {
    std::uint32_t contact_id;
    std::optional<std::int32_t> position_x;
    std::optional<std::int32_t> position_y;
    bool tip_down;
    bool confident;
};

/**
 * \brief Decoded HID input report.
 */
struct HidReport {
    std::vector<HidContact> contacts;
    std::optional<std::uint32_t> contact_count;
    bool button_one_down;
};

/**
 * \brief The axis a reports has positions in.
 */
struct HidAxisRange {
    std::int32_t min_x;
    std::int32_t max_x;
    std::int32_t min_y;
    std::int32_t max_y;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_HID_FRAME_H
