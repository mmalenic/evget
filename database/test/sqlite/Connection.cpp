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

#include "database/sqlite/Connection.h"
#include "DatabaseTest.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, CreateDatabase) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    ASSERT_TRUE(connect.has_value());
}

TEST_F(DatabaseTest, StartTransaction) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);
    auto transaction = connection.transaction();

    ASSERT_TRUE(transaction.has_value());
}

TEST_F(DatabaseTest, StartTransactionNoConnection) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};

    auto transaction = connection.transaction();

    ASSERT_FALSE(transaction.has_value());
}