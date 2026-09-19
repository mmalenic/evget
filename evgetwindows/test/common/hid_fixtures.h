#ifndef EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H
#define EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "evgetwindows/hid_frame.h"

namespace test {

/**
 * \brief The contact of a recorded frame.
 */
struct HidContactExpectation {
    std::uint32_t contact_id{};
    std::int32_t position_x{};
    std::int32_t position_y{};
    bool tip_down{};
};

/**
 * \brief The recorded input report expectation.
 */
struct HidReportExpectation {
    std::uint32_t contact_count{};
    bool button_one_down{};
    std::span<const HidContactExpectation> contacts;
};

extern const std::array<std::uint8_t, 4716> kTouchscreenPreparsedData;
extern const std::array<std::uint8_t, 3676> kTouchpadPreparsedData;
extern const std::array<std::uint8_t, 169> kTouchscreenContactDownReport;
extern const std::array<std::uint8_t, 169> kTouchscreenContactMoveReport;
extern const std::array<std::uint8_t, 169> kTouchscreenContactUpReport;
extern const std::array<std::uint8_t, 169> kTouchscreenTwoContactReport;
extern const std::array<std::uint8_t, 30> kTouchpadContactReport;
extern const std::array<std::uint8_t, 30> kTouchpadButtonReport;

constexpr std::uint32_t kTouchscreenReportByteLength = 169;
constexpr std::uint32_t kTouchpadReportByteLength = 30;
constexpr evgetwindows::HidAxisRange kTouchscreenAxis{.min_x = 0, .max_x = 25977, .min_y = 0, .max_y = 17318};
constexpr evgetwindows::HidAxisRange kTouchpadAxis{.min_x = 0, .max_x = 1956, .min_y = 0, .max_y = 997};

inline constexpr std::array<HidContactExpectation, 1> kTouchscreenContactDownContacts{
    HidContactExpectation{.contact_id = 19, .position_x = 14184, .position_y = 8991, .tip_down = true}
};

inline constexpr std::array<HidContactExpectation, 1> kTouchscreenContactMoveContacts{
    HidContactExpectation{.contact_id = 19, .position_x = 14175, .position_y = 8937, .tip_down = true}
};

inline constexpr std::array<HidContactExpectation, 1> kTouchscreenContactUpContacts{
    HidContactExpectation{.contact_id = 19, .position_x = 14175, .position_y = 8937, .tip_down = false}
};

inline constexpr std::array<HidContactExpectation, 2> kTouchscreenTwoContactContacts{
    HidContactExpectation{.contact_id = 21, .position_x = 15163, .position_y = 10635, .tip_down = true},
    HidContactExpectation{.contact_id = 22, .position_x = 18456, .position_y = 9416, .tip_down = true}
};

inline constexpr std::array<HidContactExpectation, 1> kTouchpadContactContacts{
    HidContactExpectation{.contact_id = 0, .position_x = 414, .position_y = 147, .tip_down = true}
};

inline constexpr std::array<HidContactExpectation, 3> kTouchpadButtonContacts{
    HidContactExpectation{.contact_id = 0, .position_x = 467, .position_y = 787, .tip_down = true},
    HidContactExpectation{.contact_id = 1, .position_x = 1053, .position_y = 840, .tip_down = true},
    HidContactExpectation{.contact_id = 2, .position_x = 1349, .position_y = 699, .tip_down = true}
};

inline constexpr HidReportExpectation kTouchscreenContactDownExpected{
    .contact_count = 1,
    .button_one_down = false,
    .contacts = kTouchscreenContactDownContacts
};

inline constexpr HidReportExpectation kTouchscreenContactMoveExpected{
    .contact_count = 1,
    .button_one_down = false,
    .contacts = kTouchscreenContactMoveContacts
};

inline constexpr HidReportExpectation kTouchscreenContactUpExpected{
    .contact_count = 1,
    .button_one_down = false,
    .contacts = kTouchscreenContactUpContacts
};

inline constexpr HidReportExpectation kTouchscreenTwoContactExpected{
    .contact_count = 2,
    .button_one_down = false,
    .contacts = kTouchscreenTwoContactContacts
};

inline constexpr HidReportExpectation kTouchpadContactExpected{
    .contact_count = 1,
    .button_one_down = false,
    .contacts = kTouchpadContactContacts
};

inline constexpr HidReportExpectation kTouchpadButtonExpected{
    .contact_count = 3,
    .button_one_down = true,
    .contacts = kTouchpadButtonContacts
};

/**
 * \brief View a recorded fixture as the byte span.
 * \param bytes the fixture array
 * \return a view over the bytes
 */
template <std::size_t Size>
[[nodiscard]] std::span<const std::byte> AsBytes(const std::array<std::uint8_t, Size>& bytes) {
    return std::as_bytes(std::span{bytes});
}

/**
 * \brief The contacts a decoded report shows as intentional.
 * \param report the decoded report
 * \return the contacts
 */
[[nodiscard]] std::vector<evgetwindows::HidContact> ActiveContacts(const evgetwindows::HidReport& report);

/**
 * \brief Find the contact of a decoded report by identifier.
 * \param report the decoded report
 * \param contact_id the identifier
 * \return the contact with the identifier, or nullopt if not in the report.
 */
[[nodiscard]] std::optional<evgetwindows::HidContact>
ContactById(const evgetwindows::HidReport& report, std::uint32_t contact_id);

/**
 * \brief Whether a contact of a decoded report has a unique id.
 * \param report the decoded report
 * \return true if unique id
 */
[[nodiscard]] bool ContactIdsUnique(const evgetwindows::HidReport& report);

/**
 * \brief Whether a decoded contact has a position inside descriptor bounds.
 * \param contact the decoded contact
 * \param axis the axis bounds
 * \return true when both coordinates are within the bounds
 */
[[nodiscard]] bool WithinAxis(const evgetwindows::HidContact& contact, const evgetwindows::HidAxisRange& axis);

/**
 * \brief Assert that the decoded contact is valid.
 * \param contact the decoded contact
 * \param expected the recorded expectation
 * \param axis the axis bounds
 */
void ExpectContact(
    const evgetwindows::HidContact& contact,
    const HidContactExpectation& expected,
    const evgetwindows::HidAxisRange& axis
);

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_HID_FIXTURES_H
