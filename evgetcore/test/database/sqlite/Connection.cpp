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

#include "evgetcore/database/sqlite/Connection.h"

#include <gtest/gtest.h>

#include "utils/DatabaseTest.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, Connect) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);

    ASSERT_TRUE(connect.has_value());
}

TEST_F(DatabaseTest, Transaction) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);
    auto transaction = connection.transaction();

    ASSERT_TRUE(transaction.has_value());
}

TEST_F(DatabaseTest, TransactionNoConnection) {
    EvgetCore::SQLiteConnection connection{};

    auto transaction = connection.transaction();

    ASSERT_FALSE(transaction.has_value());
}

TEST_F(DatabaseTest, Commit) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);
    auto transaction = connection.transaction();
    auto commit = connection.commit();

    ASSERT_TRUE(commit.has_value());
}

TEST_F(DatabaseTest, CommitNoTransaction) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);
    auto commit = connection.commit();

    ASSERT_FALSE(commit.has_value());
}

TEST_F(DatabaseTest, Rollback) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);
    auto transaction = connection.transaction();
    auto rollback = connection.rollback();

    ASSERT_TRUE(rollback.has_value());
}

TEST_F(DatabaseTest, RollbackNoTransaction) {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(database_file(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);
    auto rollback = connection.rollback();

    ASSERT_FALSE(rollback.has_value());
}
