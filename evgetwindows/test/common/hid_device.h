#ifndef EVGETWINDOWS_TEST_COMMON_HID_DEVICE_H
#define EVGETWINDOWS_TEST_COMMON_HID_DEVICE_H

#include <windows.h>

// This depends on windows.h so it must stay below.
#include <hidsdi.h>

#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

#include "evgetwindows/hid_usages.h"

namespace test {

/**
 * \brief A button field of an input report.
 */
struct HidField {
    USAGE page{};
    USAGE usage{};
    std::uint16_t bits{};
    std::int32_t logical_max{};
    bool button{false};
};

/**
 * \brief A HID report device spec.
 */
struct HidDeviceSpec {
    USAGE usage{};
    std::uint8_t report_id{};
    std::uint16_t contacts{};
    std::span<const HidField> frame;
    std::span<const HidField> contact;
};

/**
 * \brief The HID contact values.
 */
struct HidContactValues {
    std::uint32_t contact_id{};
    std::int32_t position_x{};
    std::int32_t position_y{};
    bool tip_down{};
    bool confident{};
};

/**
 * \brief The report values for a device.
 */
struct HidReportValues {
    std::uint32_t contact_count{};
    bool button_one_down{};
    std::span<const HidContactValues> contacts;
};

/**
 * \brief The caps before parsing.
 */
struct PreparsedCaps {
    std::size_t caps;
    std::size_t nodes;
};

/**
 * \brief The value field.
 * \param page the usage page
 * \param usage the usage
 * \param bits the field width
 * \param logical_max the maximum
 * \return the field
 */
[[nodiscard]] constexpr HidField Value(USAGE page, USAGE usage, std::uint16_t bits, std::int32_t logical_max) {
    return HidField{.page = page, .usage = usage, .bits = bits, .logical_max = logical_max, .button = false};
}

/**
 * \brief The button field.
 * \param page the usage page
 * \param usage the usage
 * \return the field
 */
[[nodiscard]] constexpr HidField Button(USAGE page, USAGE usage) {
    return HidField{.page = page, .usage = usage, .bits = 1, .logical_max = 0, .button = true};
}

namespace detail {

// See
// https://github.com/libusb/hidapi/blob/5bec5896c26578a45bbe7930622513ad652123e0/windows/hidapi_descriptor_reconstruct.h#L103-L239
constexpr std::array<char, 8> kMagic{'H', 'i', 'd', 'P', ' ', 'K', 'D', 'R'};
constexpr std::size_t kHeaderSize = 44;
constexpr std::size_t kCapSize = 104;
constexpr std::size_t kNodeSize = 16;

/**
 * \brief The caps range and report length.
 */
struct CapsInfo {
    std::uint16_t first;
    std::uint16_t count;
    std::uint16_t last;
    std::uint16_t report_byte_length;
};

/**
 * \brief Preparsed data header.
 */
struct Header {
    std::array<char, kMagic.size()> magic;
    USAGE usage;
    USAGE usage_page;
    std::array<std::uint16_t, 2> reserved;
    std::array<CapsInfo, 3> caps_info;
    std::uint16_t link_offset;
    std::uint16_t link_count;
};

/**
 * \brief An unknown item.
 */
struct UnknownToken {
    std::uint8_t token;
    std::array<std::uint8_t, 3> reserved;
    std::uint32_t bit_field;
};

/**
 * \brief A field in a report.
 */
struct Cap {
    USAGE usage_page;
    std::uint8_t report_id;
    std::uint8_t bit_position;
    std::uint16_t bit_size;
    std::uint16_t report_count;
    std::uint16_t byte_position;
    std::uint16_t bit_count;
    std::uint32_t bit_field;
    std::uint16_t next_byte_position;
    std::uint16_t link_collection;
    USAGE link_usage_page;
    USAGE link_usage;
    std::uint8_t flags;
    std::array<std::uint8_t, 3> flags_reserved;
    std::array<UnknownToken, 4> unknown_tokens;
    USAGE usage_min;
    USAGE usage_max;
    std::uint16_t string_min;
    std::uint16_t string_max;
    std::uint16_t designator_min;
    std::uint16_t designator_max;
    std::uint16_t data_index_min;
    std::uint16_t data_index_max;
    std::uint8_t has_null;
    std::array<std::uint8_t, 3> null_reserved;
    std::int32_t logical_min;
    std::int32_t logical_max;
    std::int32_t physical_min;
    std::int32_t physical_max;
    std::uint32_t units;
    std::uint32_t units_exp;
};

/**
 * \brief The node in a collection.
 */
struct Node {
    USAGE usage;
    USAGE usage_page;
    std::uint16_t parent;
    std::uint16_t children;
    std::uint16_t next_sibling;
    std::uint16_t first_child;
    std::uint32_t type;
};

static_assert(sizeof(Header) == kHeaderSize && sizeof(Cap) == kCapSize && sizeof(Node) == kNodeSize);
static_assert(
    std::has_unique_object_representations_v<Header> && std::has_unique_object_representations_v<Cap> &&
    std::has_unique_object_representations_v<Node>
);
static_assert(std::endian::native == std::endian::little);

constexpr std::uint8_t kCapIsButton = 1U << 2U;
constexpr std::uint8_t kCapIsAbsolute = 1U << 3U;

constexpr std::uint32_t kMainItemVariable = 0x2;
constexpr std::uint32_t kApplicationCollection = 1;
constexpr std::uint32_t kLogicalCollection = 2;

/**
 * \brief The preparsed layout.
 */
template <PreparsedCaps Shape>
struct Layout {
    Header header;
    std::array<Cap, Shape.caps> caps;
    std::array<Node, Shape.nodes> nodes;
    Cap spare;
};

constexpr std::size_t FieldBits(std::span<const HidField> fields) {
    std::size_t bits = 0;
    for (const auto& field : fields) {
        bits += field.bits;
    }
    return bits;
}

template <typename Visit>
constexpr void ForEachField(const HidDeviceSpec& device, const Visit& visit) {
    std::size_t bit = CHAR_BIT;
    for (const auto& field : device.frame) {
        visit(field, bit, std::uint16_t{0});
        bit += field.bits;
    }
    for (std::uint16_t collection = 1; collection <= device.contacts; ++collection) {
        for (const auto& field : device.contact) {
            visit(field, bit, collection);
            bit += field.bits;
        }
    }
}

constexpr Cap MakeCap(
    const HidField& field,
    const HidDeviceSpec& device,
    std::size_t bit,
    std::uint16_t collection,
    std::uint16_t data_index
) {
    Cap cap{};
    cap.usage_page = field.page;
    cap.report_id = device.report_id;
    cap.bit_position = static_cast<std::uint8_t>(bit % CHAR_BIT);
    cap.bit_size = field.bits;
    cap.report_count = 1;
    cap.byte_position = static_cast<std::uint16_t>(bit / CHAR_BIT);
    cap.bit_count = field.bits;
    cap.bit_field = kMainItemVariable;
    cap.next_byte_position = static_cast<std::uint16_t>((bit + field.bits + CHAR_BIT - 1) / CHAR_BIT);
    cap.link_collection = collection;
    cap.link_usage_page = HID_USAGE_PAGE_DIGITIZER;
    // Fields outside the contact link to the application.
    cap.link_usage = collection == 0 ? device.usage : HID_USAGE_DIGITIZER_FINGER;
    cap.flags = static_cast<std::uint8_t>(kCapIsAbsolute | (field.button ? kCapIsButton : std::uint8_t{0}));
    // Usage is stored as a range of one.
    cap.usage_min = field.usage;
    cap.usage_max = field.usage;
    cap.data_index_min = data_index;
    cap.data_index_max = data_index;
    // A button range has only value caps being set for the range.
    if (!field.button) {
        cap.logical_max = field.logical_max;
    }
    return cap;
}

constexpr std::uint32_t FieldValue(const HidField& field, const HidReportValues& values, std::uint16_t collection) {
    const auto matches = [&field](USAGE page, USAGE usage) { return field.page == page && field.usage == usage; };
    if (collection == 0) {
        if (matches(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_CONTACT_COUNT)) {
            return values.contact_count;
        }
        if (matches(HID_USAGE_PAGE_BUTTON, USAGE{1})) {
            return values.button_one_down ? 1U : 0U;
        }
        return 0;
    }

    // Values past contacts are empty.
    const HidContactValues contact =
        collection <= values.contacts.size() ? values.contacts[collection - 1] : HidContactValues{};
    if (matches(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_TIP_SWITCH)) {
        return contact.tip_down ? 1U : 0U;
    }
    if (matches(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_TOUCH_VALID)) {
        return contact.confident ? 1U : 0U;
    }
    if (matches(HID_USAGE_PAGE_DIGITIZER, HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER)) {
        return contact.contact_id;
    }
    if (matches(HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X)) {
        return static_cast<std::uint32_t>(contact.position_x);
    }
    if (matches(HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y)) {
        return static_cast<std::uint32_t>(contact.position_y);
    }
    return 0;
}

constexpr void PutBits(std::span<std::uint8_t> report, std::size_t first, std::size_t width, std::uint32_t value) {
    for (std::size_t index = 0; index < width; ++index) {
        if (((value >> index) & 1U) != 0) {
            const std::size_t bit = first + index;
            report[bit / CHAR_BIT] |= static_cast<std::uint8_t>(1U << (bit % CHAR_BIT));
        }
    }
}

} // namespace detail

/**
 * \brief The length of a report.
 * \param device the device
 * \return the byte length
 */
[[nodiscard]] constexpr std::size_t InputReportLength(const HidDeviceSpec& device) {
    const std::size_t bits =
        CHAR_BIT + detail::FieldBits(device.frame) + (detail::FieldBits(device.contact) * device.contacts);
    return (bits + CHAR_BIT - 1) / CHAR_BIT;
}

/**
 * \brief Get the prepased layout of the spec.
 * \param device the device
 * \return the prepared caps
 */
[[nodiscard]] constexpr PreparsedCaps LayoutOf(const HidDeviceSpec& device) {
    return {
        .caps = device.frame.size() + (device.contact.size() * device.contacts),
        .nodes = device.contacts + std::size_t{1},
    };
}

/**
 * \brief Build the preparsed data.
 * \tparam Layout the result of LayoutOf for the device
 * \param device the device
 * \return the preparsed data
 */
template <PreparsedCaps Layout>
[[nodiscard]] constexpr std::array<std::uint8_t, sizeof(detail::Layout<Layout>)> BuildPreparsed(
    const HidDeviceSpec& device
) {
    detail::Layout<Layout> out{};
    out.header.magic = detail::kMagic;
    out.header.usage = device.usage;
    out.header.usage_page = HID_USAGE_PAGE_DIGITIZER;

    std::uint16_t cap = 0;
    detail::ForEachField(device, [&](const HidField& field, std::size_t bit, std::uint16_t collection) {
        out.caps.at(cap) = detail::MakeCap(field, device, bit, collection, cap);
        ++cap;
    });

    // Only the input report has caps, output ranges are empty.
    const auto length = static_cast<std::uint16_t>(InputReportLength(device));
    out.header.caps_info = {
        detail::CapsInfo{.first = 0, .count = cap, .last = cap, .report_byte_length = length},
        detail::CapsInfo{.first = cap, .count = 0, .last = cap, .report_byte_length = 0},
        detail::CapsInfo{.first = cap, .count = 0, .last = cap, .report_byte_length = 0},
    };
    out.header.link_offset = static_cast<std::uint16_t>(cap * sizeof(detail::Cap));
    out.header.link_count = static_cast<std::uint16_t>(Layout.nodes);

    out.nodes.front() = {
        .usage = device.usage,
        .usage_page = HID_USAGE_PAGE_DIGITIZER,
        .parent = 0,
        .children = device.contacts,
        .next_sibling = 0,
        .first_child = device.contacts,
        .type = detail::kApplicationCollection,
    };
    for (std::uint16_t contact = 1; contact <= device.contacts; ++contact) {
        out.nodes.at(contact) = {
            .usage = HID_USAGE_DIGITIZER_FINGER,
            .usage_page = HID_USAGE_PAGE_DIGITIZER,
            .parent = 0,
            .children = 0,
            .next_sibling = static_cast<std::uint16_t>(contact - 1),
            .first_child = 0,
            .type = detail::kLogicalCollection,
        };
    }

    return std::bit_cast<std::array<std::uint8_t, sizeof(out)>>(out);
}

/**
 * \brief Build the input report.
 * \tparam Size the result of InputReportLength for the device
 * \param device the device
 * \param values the contacts, count and button.
 * \return the report
 */
template <std::size_t Size>
[[nodiscard]] constexpr std::array<std::uint8_t, Size>
BuildReport(const HidDeviceSpec& device, const HidReportValues& values) {
    std::array<std::uint8_t, Size> report{};
    report.front() = device.report_id;
    detail::ForEachField(device, [&](const HidField& field, std::size_t bit, std::uint16_t collection) {
        detail::PutBits(report, bit, field.bits, detail::FieldValue(field, values, collection));
    });
    return report;
}

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_HID_DEVICE_H
