#ifndef EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H
#define EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "common/hid_device.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_usages.h"

namespace test {

constexpr std::size_t kTouchscreenReportByteLength = 169;

constexpr std::uint16_t kTouchpadContactCollections = 5;
constexpr evgetwindows::HidAxisRange kTouchpadAxis{.min_x = 0, .max_x = 1956, .min_y = 0, .max_y = 997};

inline constexpr std::array<HidField, 2> kTouchpadFrame{
    Value(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_CONTACT_COUNT, 8, 255),
    Button(HID_USAGE_PAGE_BUTTON, USAGE{1}),
};

inline constexpr std::array<HidField, 5> kTouchpadContact{
    Button(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_TIP_SWITCH),
    Button(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_TOUCH_VALID),
    Value(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER, 8, 255),
    Value(HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X, 16, kTouchpadAxis.max_x),
    Value(HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y, 16, kTouchpadAxis.max_y),
};

inline constexpr HidDeviceSpec kTouchpad{
    .usage = HID_USAGE_DIGITIZER_TOUCH_PAD,
    .report_id = 4,
    .contacts = kTouchpadContactCollections,
    .frame = kTouchpadFrame,
    .contact = kTouchpadContact,
};

inline constexpr auto kTouchpadPreparsedData = BuildPreparsed<LayoutOf(kTouchpad)>(kTouchpad);

inline constexpr std::array<HidContactValues, 2> kDownAndUpSlots{
    HidContactValues{.contact_id = 2, .position_x = 414, .position_y = 147, .tip_down = true, .confident = true},
    HidContactValues{.contact_id = 1, .position_x = 1053, .position_y = 840, .tip_down = false, .confident = true},
};

inline constexpr HidReportValues kDownAndLifted{
    .contact_count = 1,
    .button_one_down = false,
    .contacts = kDownAndUpSlots,
};

inline constexpr std::array<HidContactValues, 3> kButtonPressedSlots{
    HidContactValues{.contact_id = 0, .position_x = 467, .position_y = 787, .tip_down = true, .confident = true},
    HidContactValues{.contact_id = 1, .position_x = 1053, .position_y = 840, .tip_down = true, .confident = true},
    HidContactValues{.contact_id = 2, .position_x = 1349, .position_y = 699, .tip_down = true, .confident = true},
};

inline constexpr HidReportValues kButtonPressed{
    .contact_count = 3,
    .button_one_down = true,
    .contacts = kButtonPressedSlots,
};

inline constexpr auto kDownAndLiftedReport = BuildReport<InputReportLength(kTouchpad)>(kTouchpad, kDownAndLifted);
inline constexpr auto kButtonPressedReport = BuildReport<InputReportLength(kTouchpad)>(kTouchpad, kButtonPressed);

/**
 * \brief View a fixture as a byte span.
 * \param bytes the fixture array
 * \return a view
 */
template <std::size_t Size>
[[nodiscard]] std::span<const std::byte> AsBytes(const std::array<std::uint8_t, Size>& bytes) {
    return std::as_bytes(std::span{bytes});
}

/**
 * \brief Assert that the decoded report holds the values it was built with.
 * \param decoded the decoded report
 * \param values the values in the report
 */
void ExpectDecoded(const evgetwindows::HidReport& decoded, const HidReportValues& values);

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H
