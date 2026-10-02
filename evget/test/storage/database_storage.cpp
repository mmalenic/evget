#include "evget/storage/database_storage.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <utility>

#include "common/database.h"
#include "common/log.h"
#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"
#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/key.h"
#include "evget/event/modifier_value.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/schema.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {
using DatabaseStorageTest = test::DatabaseTest;
using test::CapturedLog;
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
        .Screen("1")
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
        .Screen("2")
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

TEST_F(DatabaseStorageTest, AbsentValueStoresNull) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());

    auto null_query = connection.BuildQuery("select count(*) from key where focus_window_name is null;");
    ASSERT_TRUE(null_query->Next().has_value());
    ASSERT_EQ(null_query->AsInt(0).value(), 1);

    auto empty_query = connection.BuildQuery("select count(*) from key where focus_window_name = '';");
    ASSERT_TRUE(empty_query->Next().has_value());
    ASSERT_EQ(empty_query->AsInt(0).value(), 0);
}

TEST_F(DatabaseStorageTest, EmptyStringStoresNull) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .FocusWindowName("")
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());

    auto query = connection.BuildQuery("select count(*) from key where focus_window_name is null;");
    ASSERT_TRUE(query->Next().has_value());
    ASSERT_EQ(query->AsInt(0).value(), 1);
}

TEST_F(DatabaseStorageTest, MissingRequiredValue) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());
    const CapturedLog log{};

    evget::Data data{};
    evget::Key{}.Device(evget::DeviceType::kKeyboard).Action(evget::ButtonAction::kPress).Build(data);
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(data);
    evget::MouseClick{}.Timestamp(evget::TimestampType{}).Device(evget::DeviceType::kMouse).Build(data);
    evget::MouseMove{}.Timestamp(evget::TimestampType{}).Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto key_query = connection.BuildQuery("select count(*) from key;");
    ASSERT_TRUE(key_query->Next().has_value());
    ASSERT_EQ(key_query->AsInt(0).value(), 1);
    auto click_query = connection.BuildQuery("select count(*) from mouse_click;");
    ASSERT_TRUE(click_query->Next().has_value());
    ASSERT_EQ(click_query->AsInt(0).value(), 0);
    auto move_query = connection.BuildQuery("select count(*) from mouse_move;");
    ASSERT_TRUE(move_query->Next().has_value());
    ASSERT_EQ(move_query->AsInt(0).value(), 0);

    ASSERT_EQ(
        std::ranges::count(log.Lines(), "error dropped 3 events missing a required value, the first missing timestamp"),
        1
    );
}

TEST_F(DatabaseStorageTest, StoresAfterDroppedEntry) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data dropped{};
    evget::Key{}.Device(evget::DeviceType::kKeyboard).Action(evget::ButtonAction::kPress).Build(dropped);
    ASSERT_TRUE(storage.StoreEvent(std::move(dropped)).has_value());

    evget::Data stored{};
    evget::Key{}
        .Timestamp(evget::TimestampType{})
        .Device(evget::DeviceType::kKeyboard)
        .Action(evget::ButtonAction::kPress)
        .Build(stored);
    ASSERT_TRUE(storage.StoreEvent(std::move(stored)).has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());
    auto query = connection.BuildQuery("select count(*) from key;");
    ASSERT_TRUE(query->Next().has_value());
    ASSERT_EQ(query->AsInt(0).value(), 1);
}

TEST_F(DatabaseStorageTest, MouseMoveStoresNulls) {
    auto storage = MakeStorage();
    auto init = storage.Init();
    ASSERT_TRUE(init.has_value());

    evget::Data data{};
    evget::MouseMove{}.Timestamp(evget::TimestampType{}).Device(evget::DeviceType::kMouse).Build(data);

    auto result = storage.StoreEvent(std::move(data));
    ASSERT_TRUE(result.has_value());

    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadOnly);
    ASSERT_TRUE(connect.has_value());

    auto columns_query = connection.BuildQuery(
        "select group_concat(name, ', ') from pragma_table_info('mouse_move') where \"notnull\" = 0 and pk = 0;"
    );
    ASSERT_TRUE(columns_query->Next().has_value());
    auto columns = columns_query->AsString(0);
    ASSERT_TRUE(columns.has_value());

    // SQLite lets a text primary key hold null, so uuid needs its own check.
    auto query = connection.BuildQuery(
        std::format("select count(*) from mouse_move where uuid is not null and coalesce({}) is null;", *columns)
    );
    ASSERT_TRUE(query->Next().has_value());
    ASSERT_EQ(query->AsInt(0).value(), 1);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
