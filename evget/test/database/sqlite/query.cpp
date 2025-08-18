#include <gtest/gtest.h>

#include <format>

#include "common/database.h"
#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"

using DatabaseTest = test::DatabaseTest;

TEST_F(DatabaseTest, Exec) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto insert =
        std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue);
    auto insert_query = connection.BuildQuery(insert);
    auto exec = insert_query->Exec();

    ASSERT_TRUE(exec.has_value());
}

TEST_F(DatabaseTest, Next) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsString(0);
    auto entry = select_query->AsString(1);

    ASSERT_EQ(pos.value(), "1");
    ASSERT_EQ(entry.value(), kTestTableValue);
}

TEST_F(DatabaseTest, NextWhile) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    auto next = select_query->NextWhile();

    ASSERT_TRUE(next.has_value());
}

TEST_F(DatabaseTest, Reset) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    auto reset = (*select_query).Reset();

    ASSERT_TRUE(reset.has_value());
}

TEST_F(DatabaseTest, BindBool) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery("select $1;");

    select_query->BindBool(0, true);
    select_query->Next().value();

    auto result = select_query->AsString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindInt) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery("select $1;");

    select_query->BindInt(0, 1);
    select_query->Next().value();

    auto result = select_query->AsString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindDouble) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery("select $1;");

    select_query->BindDouble(0, 1.0);
    select_query->Next().value();

    auto result = select_query->AsString(0);

    ASSERT_EQ(result.value(), "1.0");
}

TEST_F(DatabaseTest, BindChars) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery("select $1;");

    select_query->BindChars(0, "1");
    select_query->Next().value();

    auto result = select_query->AsString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, AsBool) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsBool(0);

    ASSERT_EQ(pos.value(), true);
}

TEST_F(DatabaseTest, AsInt) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsInt(0);

    ASSERT_EQ(pos.value(), 1);
}

TEST_F(DatabaseTest, AsDouble) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsDouble(0);

    ASSERT_EQ(pos.value(), 1.0);
}

TEST_F(DatabaseTest, AsString) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsString(0);

    ASSERT_EQ(pos.value(), "1");
}
