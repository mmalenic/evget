#include "evget/database/sqlite/connection.h"

#include <gtest/gtest.h>

#include "common/database.h"
#include "evget/database/connection.h"

using DatabaseTest = test::DatabaseTest;

TEST_F(DatabaseTest, Connect) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    ASSERT_TRUE(connect.has_value());
}

TEST_F(DatabaseTest, Transaction) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);
    auto transaction = connection.Transaction();

    ASSERT_TRUE(transaction.has_value());
}

TEST_F(DatabaseTest, TransactionNoConnection) {
    evget::SQLiteConnection connection{};

    auto transaction = connection.Transaction();

    ASSERT_FALSE(transaction.has_value());
}

TEST_F(DatabaseTest, Commit) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);
    auto transaction = connection.Transaction();
    auto commit = connection.Commit();

    ASSERT_TRUE(commit.has_value());
}

TEST_F(DatabaseTest, CommitNoTransaction) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);
    auto commit = connection.Commit();

    ASSERT_FALSE(commit.has_value());
}

TEST_F(DatabaseTest, Rollback) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);
    auto transaction = connection.Transaction();
    auto rollback = connection.Rollback();

    ASSERT_TRUE(rollback.has_value());
}

TEST_F(DatabaseTest, RollbackNoTransaction) {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);
    auto rollback = connection.Rollback();

    ASSERT_FALSE(rollback.has_value());
}
