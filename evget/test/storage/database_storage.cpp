#include "evget/storage/database_storage.h"

#include <gtest/gtest.h>

#include <utility>

#include "common/database.h"
#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"
#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/key.h"
#include "evget/event/modifier_value.h"
#include "evget/event/mouse_move.h"
#include "evget/event/schema.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {
using DatabaseStorageTest = test::DatabaseTest;
} // namespace

TEST_F(DatabaseStorageTest, InitRunsMigrations) {
    auto storage = MakeStorage();

    auto result = storage.Init();
    ASSERT_TRUE(result.has_value());

    // Verify schema was created by querying a table.
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto query = connection.BuildQuery("select count(*) from button_action;");
    auto next = query->Next();
    ASSERT_TRUE(next.has_value());

    ASSERT_EQ(query->AsInt(0).value(), 3);
}

TEST_F(DatabaseStorageTest, StoreKeyEvent) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    evget::Key{}
        .Interval(evget::IntervalType{100})
        .Timestamp(evget::TimestampType{})
        .PositionX(1.0)
        .PositionY(2.0)
        .DeviceName("test_device")
        .FocusWindowName("test_window")
        .FocusWindowPositionX(3.0)
        .FocusWindowPositionY(4.0)
        .FocusWindowWidth(1920)
        .FocusWindowHeight(1080)
        .Screen(1)
        .DeviceId("dev-1")
        .Device(evget::DeviceType::kKeyboard)
        .SystemEvent("test_event")
        .EventSource("test_source")
        .Button(65)
        .ButtonName("a")
        .Character("a")
        .Action(evget::ButtonAction::kPress)
        .Modifier(evget::ModifierValue::kAlt)
        .Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto query = connection.BuildQuery("select * from key;");
    auto next = query->Next();
    ASSERT_TRUE(next.has_value());

    ASSERT_EQ(query->AsDouble(1).value(), 100.0);
    ASSERT_EQ(query->AsDouble(3).value(), 1.0);
    ASSERT_EQ(query->AsDouble(4).value(), 2.0);
    ASSERT_EQ(query->AsString(5).value(), "test_device");
    ASSERT_EQ(query->AsString(6).value(), "test_window");
    ASSERT_EQ(query->AsDouble(7).value(), 3.0);
    ASSERT_EQ(query->AsDouble(8).value(), 4.0);
    ASSERT_EQ(query->AsDouble(9).value(), 1920.0);
    ASSERT_EQ(query->AsDouble(10).value(), 1080.0);
    ASSERT_EQ(query->AsDouble(11).value(), 1.0);
    ASSERT_EQ(query->AsString(12).value(), "dev-1");
    ASSERT_EQ(query->AsString(13).value(), "test_event");
    ASSERT_EQ(query->AsString(14).value(), "test_source");
    ASSERT_EQ(query->AsInt(15).value(), 1);
    ASSERT_EQ(query->AsInt(16).value(), 65);
    ASSERT_EQ(query->AsString(17).value(), "a");
    ASSERT_EQ(query->AsString(18).value(), "a");
    ASSERT_EQ(query->AsInt(19).value(), 0);

    auto mod_query = connection.BuildQuery("select modifier_id from key_modifier;");
    auto mod_next = mod_query->Next();
    ASSERT_TRUE(mod_next.has_value());
    ASSERT_EQ(mod_query->AsString(0).value(), "3");
}

TEST_F(DatabaseStorageTest, StoreMouseMoveEvent) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    evget::MouseMove{}
        .Interval(evget::IntervalType{200})
        .Timestamp(evget::TimestampType{})
        .PositionX(100.0)
        .PositionY(200.0)
        .DeviceName("test_mouse")
        .FocusWindowName("window")
        .FocusWindowPositionX(5.0)
        .FocusWindowPositionY(6.0)
        .FocusWindowWidth(1920)
        .FocusWindowHeight(1080)
        .Screen(2)
        .DeviceId("dev-2")
        .Device(evget::DeviceType::kMouse)
        .SystemEvent("move_event")
        .EventSource("move_source")
        .TouchId(7)
        .Modifier(evget::ModifierValue::kShift)
        .Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto query = connection.BuildQuery("select * from mouse_move;");
    auto next = query->Next();
    ASSERT_TRUE(next.has_value());

    ASSERT_EQ(query->AsDouble(1).value(), 200.0);
    ASSERT_EQ(query->AsDouble(3).value(), 100.0);
    ASSERT_EQ(query->AsDouble(4).value(), 200.0);
    ASSERT_EQ(query->AsString(5).value(), "test_mouse");
    ASSERT_EQ(query->AsString(6).value(), "window");
    ASSERT_EQ(query->AsDouble(7).value(), 5.0);
    ASSERT_EQ(query->AsDouble(8).value(), 6.0);
    ASSERT_EQ(query->AsDouble(9).value(), 1920.0);
    ASSERT_EQ(query->AsDouble(10).value(), 1080.0);
    ASSERT_EQ(query->AsDouble(11).value(), 2.0);
    ASSERT_EQ(query->AsString(12).value(), "dev-2");
    ASSERT_EQ(query->AsString(13).value(), "move_event");
    ASSERT_EQ(query->AsString(14).value(), "move_source");
    ASSERT_EQ(query->AsInt(15).value(), 0);
    ASSERT_EQ(query->AsInt(16).value(), 7);

    auto mod_query = connection.BuildQuery("select modifier_id from mouse_move_modifier;");
    auto mod_next = mod_query->Next();
    ASSERT_TRUE(mod_next.has_value());
    ASSERT_EQ(mod_query->AsString(0).value(), "0"); // kShift
}

TEST_F(DatabaseStorageTest, EmptyEntrySkipped) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {}, {}});

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    // Verify nothing was inserted.
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto query = connection.BuildQuery("select count(*) from key;");
    auto next = query->Next();
    ASSERT_TRUE(next.has_value());

    ASSERT_EQ(query->AsInt(0).value(), 0);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
