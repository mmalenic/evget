#include "evget/cli.h"

#include <gtest/gtest.h>

#include <chrono>

#include "common/args.h"
#include "evget/event/device_type.h"

TEST(CliTest, GetStorageTypeJsonDefault) {
    EXPECT_EQ(evget::Cli::GetStorageType("-"), evget::StorageType::kJson);
    EXPECT_EQ(evget::Cli::GetStorageType("out.json"), evget::StorageType::kJson);
    EXPECT_EQ(evget::Cli::GetStorageType("noextension"), evget::StorageType::kJson);
}

TEST(CliTest, GetStorageTypeSqliteExtensions) {
    EXPECT_EQ(evget::Cli::GetStorageType("events.sqlite"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.sqlite3"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.db"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.db3"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.s3db"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.sl3"), evget::StorageType::kSqLite);
}

TEST(CliTest, GetStorageTypeExtensionCaseInsensitive) {
    EXPECT_EQ(evget::Cli::GetStorageType("events.SQLITE"), evget::StorageType::kSqLite);
    EXPECT_EQ(evget::Cli::GetStorageType("events.Db"), evget::StorageType::kSqLite);
}

TEST(CliTest, ParseDefaults) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--output", "-"}};

    auto result = cli.Parse(argv.Argc(), argv.Argv());

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

TEST(CliTest, ParseFilterDeviceSet) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--filter", "mouse,keyboard"}};

    ASSERT_TRUE(cli.Parse(argv.Argc(), argv.Argv()).has_value());
    const auto& filter = cli.Filter();
    if (!filter.has_value()) {
        FAIL() << "filter should be populated when --filter is given";
    }
    EXPECT_TRUE(filter->contains(evget::DeviceType::kMouse));
    EXPECT_TRUE(filter->contains(evget::DeviceType::kKeyboard));
    EXPECT_FALSE(filter->contains(evget::DeviceType::kTablet));
}

TEST(CliTest, ParseFilterAllNoFilter) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--filter", "all"}};

    ASSERT_TRUE(cli.Parse(argv.Argc(), argv.Argv()).has_value());
    EXPECT_FALSE(cli.Filter().has_value());
}

TEST(CliTest, ParseScreenDimensions) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--screen-dimensions", "1920x1080"}};

    ASSERT_TRUE(cli.Parse(argv.Argc(), argv.Argv()).has_value());
    const auto dimensions = cli.ScreenDimensions();
    if (!dimensions.has_value()) {
        FAIL() << "screen dimensions should be populated";
    }
    EXPECT_EQ(dimensions->first, 1920U);
    EXPECT_EQ(dimensions->second, 1080U);
}

TEST(CliTest, ParseMalformedScreenDimensions) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--screen-dimensions", "badformat"}};

    auto result = cli.Parse(argv.Argc(), argv.Argv());

    ASSERT_FALSE(result.has_value());
}

TEST(CliTest, ParseEventSourceOverride) {
    evget::Cli cli{evget::EventSource::kX11};
    test::Args argv{{"evget", "--event-source", "libinput"}};

    ASSERT_TRUE(cli.Parse(argv.Argc(), argv.Argv()).has_value());
    EXPECT_EQ(cli.EventSource(), evget::EventSource::kLibInput);
}
