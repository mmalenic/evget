#include "evget/storage/exclude_window_title_store.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include "common/database.h"
#include "common/store.h"
#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"
#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/key.h"
#include "evget/event/schema.h"
#include "evget/storage/filter_store.h"

namespace {
using ExcludeWindowTitleSqliteTest = test::DatabaseTest;
} // namespace

using test::StoreErrorMock;
using test::StoreMock;

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ExcludeWindowTitleStoreTest, RemovesTitleFromEntryType) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());
    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)).has_value());
    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeMouseMoveData(evget::DeviceType::kMouse)).has_value());
    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeMouseScrollData(evget::DeviceType::kMouse)).has_value());

    const std::vector expected_types{
        evget::EntryType::kKey,
        evget::EntryType::kMouseClick,
        evget::EntryType::kMouseMove,
        evget::EntryType::kMouseScroll
    };
    const auto events = inner.Events();
    ASSERT_EQ(events.size(), expected_types.size());
    for (std::size_t index = 0; index < events.size(); ++index) {
        ASSERT_EQ(events.at(index).Entries().size(), 1);
        const auto& entry = events.at(index).Entries().at(0);
        EXPECT_EQ(entry.Type(), expected_types.at(index));
        EXPECT_FALSE(entry.Data().at(evget::detail::kFocusWindowNameIndex).has_value());
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ExcludeWindowTitleStoreTest, OtherValuesRetained) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    const std::vector inputs{
        StoreMock::MakeKeyData(evget::DeviceType::kKeyboard),
        StoreMock::MakeMouseClickData(evget::DeviceType::kMouse),
        StoreMock::MakeMouseMoveData(evget::DeviceType::kMouse),
        StoreMock::MakeMouseScrollData(evget::DeviceType::kMouse)
    };
    for (const auto& input : inputs) {
        ASSERT_TRUE(store.StoreEvent(input).has_value());
    }

    const auto events = inner.Events();
    ASSERT_EQ(events.size(), inputs.size());
    for (std::size_t index = 0; index < inputs.size(); ++index) {
        ASSERT_EQ(events.at(index).Entries().size(), 1);
        const auto& expected = inputs.at(index).Entries().at(0);
        const auto& actual = events.at(index).Entries().at(0);

        ASSERT_TRUE(expected.Data().at(evget::detail::kFocusWindowNameIndex).has_value());
        EXPECT_EQ(actual.Type(), expected.Type());
        EXPECT_EQ(actual.Modifiers(), expected.Modifiers());
        ASSERT_EQ(actual.Data().size(), expected.Data().size());
        for (std::size_t slot = 0; slot < expected.Data().size(); ++slot) {
            if (slot != evget::detail::kFocusWindowNameIndex) {
                EXPECT_EQ(actual.Data().at(slot), expected.Data().at(slot)) << "slot " << slot;
            }
        }
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ExcludeWindowTitleStoreTest, EntryOrderRetained) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    evget::Data combined{};
    for (auto data :
         {StoreMock::MakeKeyData(evget::DeviceType::kKeyboard),
          StoreMock::MakeMouseMoveData(evget::DeviceType::kMouse),
          StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)}) {
        for (auto&& entry : std::move(data).IntoEntries()) {
            combined.AddEntry(std::move(entry));
        }
    }

    ASSERT_TRUE(store.StoreEvent(std::move(combined)).has_value());

    const auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    const auto& entries = events.at(0).Entries();
    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(2).Type(), evget::EntryType::kMouseClick);
}

TEST(ExcludeWindowTitleStoreTest, EntryPassesThrough) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeData()).has_value());

    const auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    const auto& entry = events.at(0).Entries().at(0);
    EXPECT_EQ(entry.Type(), evget::EntryType::kKey);
    EXPECT_EQ(entry.Data(), std::vector<evget::EntryValue>{"value"});
}

TEST(ExcludeWindowTitleStoreTest, AbsentTitle) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    evget::Data data{};
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(data);
    const auto expected = data.Entries().at(0).Data();

    ASSERT_TRUE(store.StoreEvent(std::move(data)).has_value());

    const auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    const auto& actual = events.at(0).Entries().at(0).Data();
    EXPECT_FALSE(actual.at(evget::detail::kFocusWindowNameIndex).has_value());
    EXPECT_EQ(actual, expected);
}

TEST(ExcludeWindowTitleStoreTest, EmptyEvent) {
    StoreMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    ASSERT_TRUE(store.StoreEvent(evget::Data{}).has_value());

    EXPECT_TRUE(inner.Events().empty());
}

TEST(ExcludeWindowTitleStoreTest, FilterCompose) {
    StoreMock inner{};
    evget::FilterStore filter{inner, std::set{evget::DeviceType::kKeyboard}};
    evget::ExcludeWindowTitleStore store{filter};

    evget::Data combined{};
    for (auto&& entry : StoreMock::MakeKeyData(evget::DeviceType::kKeyboard).IntoEntries()) {
        combined.AddEntry(std::move(entry));
    }
    for (auto&& entry : StoreMock::MakeMouseClickData(evget::DeviceType::kMouse).IntoEntries()) {
        combined.AddEntry(std::move(entry));
    }

    ASSERT_TRUE(store.StoreEvent(std::move(combined)).has_value());

    const auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    const auto& entry = events.at(0).Entries().at(0);
    EXPECT_EQ(entry.Type(), evget::EntryType::kKey);
    EXPECT_FALSE(entry.Data().at(evget::detail::kFocusWindowNameIndex).has_value());
}

TEST(ExcludeWindowTitleStoreTest, InnerErrorPropagates) {
    StoreErrorMock inner{};
    evget::ExcludeWindowTitleStore store{inner};

    const auto result = store.StoreEvent(StoreMock::MakeKeyData(evget::DeviceType::kKeyboard));

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_type, evget::ErrorType::kDatabaseManagerError);
    EXPECT_EQ(result.error().message, "mock error");
}

TEST(ExcludeWindowTitleStoreTest, FocusWindowNameIndex) {
    EXPECT_EQ(
        evget::detail::kBaseFields.at(evget::detail::kFocusWindowNameIndex),
        std::string_view{"focus_window_name"}
    );
}

TEST_F(ExcludeWindowTitleSqliteTest, StoreNullTitle) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::ExcludeWindowTitleStore store{storage};
    ASSERT_TRUE(store.StoreEvent(StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());

    auto null_query = connection.BuildQuery("select count(*) from key where focus_window_name is null;");
    ASSERT_TRUE(null_query->Next().has_value());
    ASSERT_EQ(null_query->AsInt(0).value(), 1);

    auto width_query = connection.BuildQuery("select count(*) from key where focus_window_width is not null;");
    ASSERT_TRUE(width_query->Next().has_value());
    ASSERT_EQ(width_query->AsInt(0).value(), 1);
}
