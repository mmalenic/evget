// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>

#include <format>

#include "evget/database/Connection.h"
#include "evget/database/sqlite/Connection.h"
#include "utils/DatabaseTest.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, Exec) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto insert = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue);
    auto insertQuery = connection.buildQuery(insert);
    auto exec = insertQuery->exec();

    ASSERT_TRUE(exec.has_value());
}

TEST_F(DatabaseTest, Next) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asString(0);
    auto entry = selectQuery->asString(1);

    ASSERT_EQ(pos.value(), "1");
    ASSERT_EQ(entry.value(), testTableValue);
}

TEST_F(DatabaseTest, NextWhile) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    auto next = selectQuery->nextWhile();

    ASSERT_TRUE(next.has_value());
}

TEST_F(DatabaseTest, Reset) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    auto reset = (*selectQuery).reset();

    ASSERT_TRUE(reset.has_value());
}

TEST_F(DatabaseTest, BindBool) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindBool(0, true);
    selectQuery->next().value();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindInt) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindInt(0, 1);
    selectQuery->next().value();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindDouble) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindDouble(0, 1.0);
    selectQuery->next().value();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1.0");
}

TEST_F(DatabaseTest, BindChars) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindChars(0, "1");
    selectQuery->next().value();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, AsBool) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asBool(0);

    ASSERT_EQ(pos.value(), true);
}

TEST_F(DatabaseTest, AsInt) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asInt(0);

    ASSERT_EQ(pos.value(), 1);
}

TEST_F(DatabaseTest, AsDouble) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asDouble(0);

    ASSERT_EQ(pos.value(), 1.0);
}

TEST_F(DatabaseTest, AsString) {
    evget::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asString(0);

    ASSERT_EQ(pos.value(), "1");
}
