#include "evgetwindows/hid_query_api.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "common/hid_fixtures.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/raw_event.h"

// Required for gtest.
// NOLINTBEGIN(bugprone-unchecked-optional-access)

using test::ActiveContacts;
using test::AsBytes;
using test::ContactById;
using test::ContactIdsUnique;
using test::ExpectContact;
using test::kTouchpadAxis;
using test::kTouchpadButtonExpected;
using test::kTouchpadButtonReport;
using test::kTouchpadContactCollections;
using test::kTouchpadContactExpected;
using test::kTouchpadContactReport;
using test::kTouchpadPreparsedData;
using test::kTouchpadReportByteLength;
using test::kTouchscreenAxis;
using test::kTouchscreenContactCollections;
using test::kTouchscreenContactDownExpected;
using test::kTouchscreenContactDownReport;
using test::kTouchscreenContactMoveExpected;
using test::kTouchscreenContactMoveReport;
using test::kTouchscreenContactUpExpected;
using test::kTouchscreenContactUpReport;
using test::kTouchscreenPreparsedData;
using test::kTouchscreenReportByteLength;
using test::kTouchscreenTwoContactExpected;
using test::kTouchscreenTwoContactReport;

TEST(EvgetWindowsHidQuery, CapsFromRecordedTouchscreen) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));

    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();
    EXPECT_EQ(resolved.contact_collections.size(), kTouchscreenContactCollections);
    EXPECT_TRUE(resolved.axis_valid);
    EXPECT_GT(resolved.axis.max_x, resolved.axis.min_x);
    EXPECT_GT(resolved.axis.max_y, resolved.axis.min_y);
    EXPECT_EQ(resolved.axis.min_x, kTouchscreenAxis.min_x);
    EXPECT_EQ(resolved.axis.max_x, kTouchscreenAxis.max_x);
    EXPECT_EQ(resolved.axis.min_y, kTouchscreenAxis.min_y);
    EXPECT_EQ(resolved.axis.max_y, kTouchscreenAxis.max_y);
    EXPECT_EQ(resolved.input_report_byte_length, kTouchscreenReportByteLength);
    EXPECT_TRUE(resolved.reports_touch_valid);
}

TEST(EvgetWindowsHidQuery, CapsFromRecordedTouchpad) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));

    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();
    EXPECT_EQ(resolved.contact_collections.size(), kTouchpadContactCollections);
    EXPECT_TRUE(resolved.axis_valid);
    EXPECT_GT(resolved.axis.max_x, resolved.axis.min_x);
    EXPECT_GT(resolved.axis.max_y, resolved.axis.min_y);
    EXPECT_EQ(resolved.axis.min_x, kTouchpadAxis.min_x);
    EXPECT_EQ(resolved.axis.max_x, kTouchpadAxis.max_x);
    EXPECT_EQ(resolved.axis.min_y, kTouchpadAxis.min_y);
    EXPECT_EQ(resolved.axis.max_y, kTouchpadAxis.max_y);
    EXPECT_EQ(resolved.input_report_byte_length, kTouchpadReportByteLength);
    EXPECT_TRUE(resolved.reports_touch_valid);
    EXPECT_GT(resolved.button_usage_list_length, 0U);
}

TEST(EvgetWindowsHidQuery, CapsFromRejectsEmpty) {
    EXPECT_FALSE(evgetwindows::HidQuery::CapsFrom({}).has_value());
}

TEST(EvgetWindowsHidQuery, RecordedReportsInlineCapacity) {
    EXPECT_GE(evgetwindows::kHidReportCapacity, kTouchscreenReportByteLength);
    EXPECT_GE(evgetwindows::kHidReportCapacity, kTouchpadReportByteLength);
    EXPECT_GE(evgetwindows::kHidReportCapacity, kTouchscreenContactDownReport.size());
    EXPECT_GE(evgetwindows::kHidReportCapacity, kTouchpadButtonReport.size());
}

TEST(EvgetWindowsHidQuery, DecodeRecordedContactDown) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchscreenPreparsedData),
        resolved,
        AsBytes(kTouchscreenContactDownReport)
    );

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    ASSERT_TRUE(decoded.contact_count.has_value());
    EXPECT_EQ(decoded.contact_count.value(), kTouchscreenContactDownExpected.contact_count);
    EXPECT_EQ(decoded.button_one_down, kTouchscreenContactDownExpected.button_one_down);

    const auto active = ActiveContacts(decoded);
    ASSERT_EQ(active.size(), kTouchscreenContactDownExpected.contacts.size());
    ExpectContact(active.front(), kTouchscreenContactDownExpected.contacts.front(), resolved.axis);
    EXPECT_TRUE(active.front().confident);
}

TEST(EvgetWindowsHidQuery, DecodeRecordedContactMove) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchscreenPreparsedData),
        resolved,
        AsBytes(kTouchscreenContactMoveReport)
    );

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    const auto active = ActiveContacts(decoded);
    ASSERT_EQ(active.size(), kTouchscreenContactMoveExpected.contacts.size());
    ExpectContact(active.front(), kTouchscreenContactMoveExpected.contacts.front(), resolved.axis);
}

TEST(EvgetWindowsHidQuery, DecodeRecordedContactUp) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchscreenPreparsedData),
        resolved,
        AsBytes(kTouchscreenContactUpReport)
    );

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    EXPECT_TRUE(ActiveContacts(decoded).empty());

    const auto& expected = kTouchscreenContactUpExpected.contacts.front();
    const auto lifted = ContactById(decoded, expected.contact_id);
    ASSERT_TRUE(lifted.has_value());
    ExpectContact(lifted.value(), expected, resolved.axis);
}

TEST(EvgetWindowsHidQuery, DecodeRecordedTwoContacts) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchscreenPreparsedData),
        resolved,
        AsBytes(kTouchscreenTwoContactReport)
    );

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    ASSERT_TRUE(decoded.contact_count.has_value());
    EXPECT_EQ(decoded.contact_count.value(), kTouchscreenTwoContactExpected.contact_count);

    const auto active = ActiveContacts(decoded);
    ASSERT_EQ(active.size(), kTouchscreenTwoContactExpected.contacts.size());
    for (std::size_t index = 0; index < active.size(); ++index) {
        ExpectContact(active.at(index), kTouchscreenTwoContactExpected.contacts[index], resolved.axis);
    }
    EXPECT_NE(active.front().contact_id, active.back().contact_id);
}

TEST(EvgetWindowsHidQuery, DecodeRecordedTouchpadContact) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report =
        evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchpadPreparsedData), resolved, AsBytes(kTouchpadContactReport));

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    ASSERT_TRUE(decoded.contact_count.has_value());
    EXPECT_EQ(decoded.contact_count.value(), kTouchpadContactExpected.contact_count);
    EXPECT_EQ(decoded.button_one_down, kTouchpadContactExpected.button_one_down);

    const auto active = ActiveContacts(decoded);
    ASSERT_EQ(active.size(), kTouchpadContactExpected.contacts.size());
    ExpectContact(active.front(), kTouchpadContactExpected.contacts.front(), resolved.axis);
}

TEST(EvgetWindowsHidQuery, DecodeRecordedTouchpadButton) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report =
        evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchpadPreparsedData), resolved, AsBytes(kTouchpadButtonReport));

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    EXPECT_EQ(decoded.button_one_down, kTouchpadButtonExpected.button_one_down);
    ASSERT_TRUE(decoded.contact_count.has_value());
    EXPECT_EQ(decoded.contact_count.value(), kTouchpadButtonExpected.contact_count);

    const auto active = ActiveContacts(decoded);
    ASSERT_EQ(active.size(), kTouchpadButtonExpected.contacts.size());
    for (std::size_t index = 0; index < active.size(); ++index) {
        ExpectContact(active.at(index), kTouchpadButtonExpected.contacts[index], resolved.axis);
    }
}

TEST(EvgetWindowsHidQuery, DecodeKeepsTheDeviceUses) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report =
        evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchpadPreparsedData), resolved, AsBytes(kTouchpadButtonReport));

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    ASSERT_TRUE(decoded.contact_count.has_value());
    EXPECT_EQ(decoded.contacts.size(), decoded.contact_count.value());
    EXPECT_TRUE(ContactIdsUnique(decoded));
}

TEST(EvgetWindowsHidQuery, DecodeRemovesEmptyEntries) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchscreenPreparsedData),
        resolved,
        AsBytes(kTouchscreenContactDownReport)
    );

    ASSERT_TRUE(report.has_value());
    const evgetwindows::HidReport& decoded = report.value();
    EXPECT_TRUE(ContactIdsUnique(decoded));
    EXPECT_FALSE(ContactById(decoded, 0).has_value());
    EXPECT_TRUE(ContactById(decoded, kTouchscreenContactDownExpected.contacts.front().contact_id).has_value());
}

TEST(EvgetWindowsHidQuery, DecodeRejectTruncatedReport) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    const auto whole = AsBytes(kTouchscreenContactDownReport);
    const auto truncated = whole.first(whole.size() / 2);

    const auto report = evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchscreenPreparsedData), resolved, truncated);

    // The span length is the only authority the parser has over the buffer, so a short report fails rather than
    // continuing.
    EXPECT_FALSE(report.has_value());
}

TEST(EvgetWindowsHidQuery, DecodeRejectEmptyReport) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchscreenPreparsedData));
    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();

    EXPECT_FALSE(evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchscreenPreparsedData), resolved, {}).has_value());
}

// NOLINTEND(bugprone-unchecked-optional-access)
