#include "evget/storage/json_storage.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include "common/stream.h"
#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/key.h"
#include "evget/event/mouse_move.h"
#include "evget/event/schema.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

namespace {

using test::SyncCountingBuffer;

} // namespace

TEST(JsonStorageTest, EmptyDataNoOutput) {
    SyncCountingBuffer buffer{};
    std::ostream stream{&buffer};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(buffer.str().empty());
    ASSERT_EQ(buffer.SyncCount(), 0);
}

TEST(JsonStorageTest, FlushesAfterWrite) {
    SyncCountingBuffer buffer{};
    std::ostream stream{&buffer};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data first{};
    first.AddEntry({evget::EntryType::kKey, {"value1"}, {}});
    evget::Data second{};
    second.AddEntry({evget::EntryType::kKey, {"value2"}, {}});

    ASSERT_TRUE(storage.StoreEvent(std::move(first)).has_value());
    ASSERT_TRUE(storage.StoreEvent(std::move(second)).has_value());

    ASSERT_EQ(buffer.SyncCount(), 2);
    ASSERT_FALSE(buffer.str().empty());
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

TEST(JsonStorageTest, AbsentValueIsNull) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(data);

    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    auto json = nlohmann::json::parse(stream.str());
    auto field_data = [&json](const std::string& name) {
        for (const auto& field : json["entries"][0]["fields"]) {
            if (field["name"] == name) {
                return field["data"];
            }
        }
        return nlohmann::json{};
    };

    ASSERT_TRUE(field_data("focus_window_name").is_null());
    ASSERT_TRUE(field_data("timestamp").is_string());
}

TEST(JsonStorageTest, MouseMoveWritesNulls) {
    std::ostringstream stream{};
    evget::JsonStorage storage{
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>{&stream, [](auto*) {}}
    };

    evget::Data data{};
    evget::MouseMove{}.Timestamp(evget::TimestampType{}).Device(evget::DeviceType::kMouse).Build(data);

    auto result = storage.StoreEvent(std::move(data));

    ASSERT_TRUE(result.has_value());
    auto json = nlohmann::json::parse(stream.str());
    const auto& fields = json["entries"][0]["fields"];
    ASSERT_EQ(fields.size(), evget::detail::kMouseMoveNFields);
    std::map<std::string, std::string> non_null{};
    for (const auto& field : fields) {
        if (!field["data"].is_null()) {
            non_null.emplace(field["name"].get<std::string>(), field["data"].type_name());
        }
    }
    const std::map<std::string, std::string> expected{{"device_type", "string"}, {"timestamp", "string"}};
    EXPECT_EQ(non_null, expected);
}

// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
