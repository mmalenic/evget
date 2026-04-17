#include "evget/cli.h"

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <string>

#include "evget/event/device_type.h"

namespace {

std::array<char*, 3> ArgvOf(const char* arg0, const char* arg1, const char* arg2) {
    // CLI11's parse() takes char** because it's allowed to mutate argv. Our test strings
    // string literals so the cast is unavoidable.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
    return {const_cast<char*>(arg0), const_cast<char*>(arg1), const_cast<char*>(arg2)};
    // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
}

} // namespace

TEST(CliTest, GetStorageTypeIsJsonByDefault) {
    EXPECT_EQ(evget::Cli::GetStorageType("-"), evget::StorageType::kJson);
    EXPECT_EQ(evget::Cli::GetStorageType("out.json"), evget::StorageType::kJson);
    EXPECT_EQ(evget::Cli::GetStorageType("noextension"), evget::StorageType::kJson);
}

TEST(CliTest, GetStorageTypeRecognisesSqliteExtensions) {
    EXPECT_EQ(evget::Cli::GetStorageType("events.sqlite"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.sqlite3"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.db"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.db3"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.s3db"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.sl3"), evget::StorageType::kSqLite);
}

TEST(CliTest, GetStorageTypeExtensionIsCaseInsensitive) {
    EXPECT_EQ(evget::Cli::GetStorageType("events.SQLITE"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.Db"), evget::StorageType::kSqLite);
}

TEST(CliTest, ParseDefaultsExposedThroughAccessors) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--output", "-");

    auto result = cli.Parse(3, argv.data());

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(*result);
    EXPECT_EQ(cli.EventSource(), evget::EventSource::kX11);
    EXPECT_EQ(cli.StoreNEvents(), 100U);
    EXPECT_EQ(cli.StoreAfter(), std::chrono::seconds{100});
    EXPECT_FALSE(cli.Filter().has_value());
    EXPECT_FALSE(cli.Display().has_value());
    EXPECT_FALSE(cli.Seat().has_value());
    EXPECT_FALSE(cli.ScreenDimensions().has_value());
}

TEST(CliTest, ParseFilterExpandsDeviceSet) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--filter", "mouse,keyboard");

    ASSERT_TRUE(cli.Parse(3, argv.data()).has_value());
    const auto& filter = cli.Filter();
    if (!filter.has_value()) {
        FAIL() << "filter should be populated when --filter is given";
    }
    EXPECT_TRUE(filter->contains(evget::DeviceType::kMouse));
    EXPECT_TRUE(filter->contains(evget::DeviceType::kKeyboard));
    EXPECT_FALSE(filter->contains(evget::DeviceType::kTablet));
}

TEST(CliTest, ParseFilterAllLeavesNoFilter) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--filter", "all");

    ASSERT_TRUE(cli.Parse(3, argv.data()).has_value());
    EXPECT_FALSE(cli.Filter().has_value());
}

TEST(CliTest, ParseScreenDimensionsReadsWidthHeight) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--screen-dimensions", "1920x1080");

    ASSERT_TRUE(cli.Parse(3, argv.data()).has_value());
    const auto dimensions = cli.ScreenDimensions();
    if (!dimensions.has_value()) {
        FAIL() << "screen dimensions should be populated";
    }
    EXPECT_EQ(dimensions->first, 1920U);
    EXPECT_EQ(dimensions->second, 1080U);
}

TEST(CliTest, ParseRejectsMalformedScreenDimensions) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--screen-dimensions", "badformat");

    auto result = cli.Parse(3, argv.data());

    ASSERT_FALSE(result.has_value());
}

TEST(CliTest, ParseEventSourceOverride) {
    evget::Cli cli{evget::EventSource::kX11};
    auto argv = ArgvOf("evget", "--event-source", "libinput");

    ASSERT_TRUE(cli.Parse(3, argv.data()).has_value());
    EXPECT_EQ(cli.EventSource(), evget::EventSource::kLibInput);
}
