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

#include "DatabaseTest.h"
#include "database/sqlite/Connection.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, Exec) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto insert = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue);
    auto insertQuery = connection.buildQuery(insert.c_str());
    auto exec = insertQuery->exec();

    ASSERT_TRUE(exec.has_value());
}

TEST_F(DatabaseTest, Next) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asString(0);
    auto entry = selectQuery->asString(1);

    ASSERT_EQ(id.value(), "1");
    ASSERT_EQ(entry.value(), testTableValue);
}

TEST_F(DatabaseTest, NextFromString) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQueryFromString(select);

    selectQuery->next();

    auto id = selectQuery->asString(0);
    auto entry = selectQuery->asString(1);

    ASSERT_EQ(id.value(), "1");
    ASSERT_EQ(entry.value(), testTableValue);
}

TEST_F(DatabaseTest, NextWhile) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    auto next = selectQuery->nextWhile();

    ASSERT_TRUE(next.has_value());
}

TEST_F(DatabaseTest, Reset) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    auto reset = selectQuery->reset();

    ASSERT_TRUE(reset.has_value());
}

TEST_F(DatabaseTest, BindBool) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindBool(0, true);
    selectQuery->next();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindInt) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindInt(0, 1);
    selectQuery->next();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, BindDouble) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindDouble(0, 1.0);
    selectQuery->next();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1.0");
}

TEST_F(DatabaseTest, BindChars) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto selectQuery = connection.buildQuery("select $1;");

    selectQuery->bindChars(0, "1");
    selectQuery->next();

    auto result = selectQuery->asString(0);

    ASSERT_EQ(result.value(), "1");
}

TEST_F(DatabaseTest, AsBool) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asBool(0);

    ASSERT_EQ(id.value(), true);
}

TEST_F(DatabaseTest, AsInt) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asInt(0);

    ASSERT_EQ(id.value(), 1);
}

TEST_F(DatabaseTest, AsDouble) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asDouble(0);

    ASSERT_EQ(id.value(), 1.0);
}

TEST_F(DatabaseTest, AsString) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asString(0);

    ASSERT_EQ(id.value(), "1");
}