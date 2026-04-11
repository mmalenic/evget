#include "evget/storage/json_storage.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/entry.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

TEST(JsonStorageTest, EmptyDataNoOutput) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(stream.str().empty());
}

TEST(JsonStorageTest, SingleKeyEntry) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {"field_value"}, {}});

    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    auto json = nlohmann::json::parse(stream.str());
    ASSERT_EQ(json["entries"].size(), 1);
    ASSERT_EQ(json["entries"][0]["type"], "Key");
}

TEST(JsonStorageTest, MultipleEntries) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {"value1"}, {}});
    data.AddEntry({evget::EntryType::kMouseMove, {"value2"}, {}});

    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    auto json = nlohmann::json::parse(stream.str());
    ASSERT_EQ(json["entries"].size(), 2);
    ASSERT_EQ(json["entries"][0]["type"], "Key");
    ASSERT_EQ(json["entries"][1]["type"], "MouseMove");
}

TEST(JsonStorageTest, ModifiersIncluded) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {"value"}, {"0", "3"}});

    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    auto json = nlohmann::json::parse(stream.str());
    auto modifiers = json["entries"][0]["modifiers"];
    ASSERT_EQ(modifiers.size(), 2);
    ASSERT_EQ(modifiers[0], "Shift");
    ASSERT_EQ(modifiers[1], "Alt");
}

// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
