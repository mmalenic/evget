#include "evgetwindows/hid_query_api.h"

#include <gtest/gtest.h>

#include "common/hid_device.h"
#include "common/hid_fixtures.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/raw_event.h"

// Required for gtest.
// NOLINTBEGIN(bugprone-unchecked-optional-access)

using test::AsBytes;
using test::ExpectDecoded;
using test::InputReportLength;
using test::kButtonPressed;
using test::kButtonPressedReport;
using test::kDownAndLifted;
using test::kDownAndLiftedReport;
using test::kTouchpad;
using test::kTouchpadAxis;
using test::kTouchpadContactCollections;
using test::kTouchpadPreparsedData;
using test::kTouchscreenReportByteLength;

TEST(EvgetWindowsHidQuery, CapsFrom) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));

    ASSERT_TRUE(caps.has_value());
    const evgetwindows::HidDeviceCaps& resolved = caps.value();
    EXPECT_EQ(resolved.contact_collections.size(), kTouchpadContactCollections);
    EXPECT_TRUE(resolved.axis_valid);
    EXPECT_EQ(resolved.axis.min_x, kTouchpadAxis.min_x);
    EXPECT_EQ(resolved.axis.max_x, kTouchpadAxis.max_x);
    EXPECT_EQ(resolved.axis.min_y, kTouchpadAxis.min_y);
    EXPECT_EQ(resolved.axis.max_y, kTouchpadAxis.max_y);
    EXPECT_EQ(resolved.input_report_byte_length, InputReportLength(kTouchpad));
    EXPECT_TRUE(resolved.reports_touch_valid);
    EXPECT_GT(resolved.button_usage_list_length, 0U);
}

TEST(EvgetWindowsHidQuery, CapsFromRejectsEmpty) {
    EXPECT_FALSE(evgetwindows::HidQuery::CapsFrom({}).has_value());
}

TEST(EvgetWindowsHidQuery, ReportsInlineCapacity) {
    EXPECT_GE(evgetwindows::kHidReportCapacity, kTouchscreenReportByteLength);
}

TEST(EvgetWindowsHidQuery, DecodeDownAndLifted) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchpadPreparsedData),
        caps.value(),
        AsBytes(kDownAndLiftedReport)
    );

    ASSERT_TRUE(report.has_value());
    ExpectDecoded(report.value(), kDownAndLifted);
}

TEST(EvgetWindowsHidQuery, DecodeButtonPressed) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());

    const auto report = evgetwindows::HidQuery::DecodeWith(
        AsBytes(kTouchpadPreparsedData),
        caps.value(),
        AsBytes(kButtonPressedReport)
    );

    ASSERT_TRUE(report.has_value());
    ExpectDecoded(report.value(), kButtonPressed);
}

TEST(EvgetWindowsHidQuery, DecodeRejectTruncatedReport) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());

    const auto whole = AsBytes(kDownAndLiftedReport);
    const auto truncated = whole.first(whole.size() / 2);

    // The span length is the only thing the parser knows about the buffer, so a too short report fails rather than
    // continuing.
    EXPECT_FALSE(
        evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchpadPreparsedData), caps.value(), truncated).has_value()
    );
}

TEST(EvgetWindowsHidQuery, DecodeRejectEmptyReport) {
    const auto caps = evgetwindows::HidQuery::CapsFrom(AsBytes(kTouchpadPreparsedData));
    ASSERT_TRUE(caps.has_value());

    EXPECT_FALSE(evgetwindows::HidQuery::DecodeWith(AsBytes(kTouchpadPreparsedData), caps.value(), {}).has_value());
}

// NOLINTEND(bugprone-unchecked-optional-access)
