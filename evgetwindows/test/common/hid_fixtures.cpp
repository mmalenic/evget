#include "common/hid_fixtures.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

#include "common/hid_device.h"
#include "evgetwindows/hid_frame.h"

void test::ExpectDecoded(const evgetwindows::HidReport& decoded, const HidReportValues& values) {
    EXPECT_EQ(decoded.contact_count, std::optional{values.contact_count});
    EXPECT_EQ(decoded.button_one_down, values.button_one_down);

    using Contact = std::tuple<std::uint32_t, std::optional<std::int32_t>, std::optional<std::int32_t>, bool, bool>;
    std::vector<Contact> actual;
    actual.reserve(decoded.contacts.size());
    for (const auto& contact : decoded.contacts) {
        actual.emplace_back(
            contact.contact_id,
            contact.position_x,
            contact.position_y,
            contact.tip_down,
            contact.confident
        );
    }

    std::vector<Contact> expected;
    expected.reserve(values.contacts.size());
    for (const auto& sent : values.contacts) {
        expected.emplace_back(sent.contact_id, sent.position_x, sent.position_y, sent.tip_down, sent.confident);
    }
    std::ranges::sort(expected);

    EXPECT_EQ(actual, expected);
}
