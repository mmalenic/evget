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

#include "database/Migrate.h"

#include <gtest/gtest.h>

#include <format>

#include "DatabaseTest.h"
#include "database/sqlite/Connection.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, Migrate) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};
    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto migration = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrations{migration};

    Database::Migrate migrate{connection, migrations};

    auto migrateResult = migrate.migrate();

    ASSERT_TRUE(migrateResult.has_value());

    auto selectMigration = std::format("select * from _migrations;");
    auto selectMigrationQuery = connection.buildQuery(selectMigration.c_str());

    selectMigrationQuery->next();

    auto version = selectMigrationQuery->asString(0);
    auto description = selectMigrationQuery->asString(1);
    auto installedOn = selectMigrationQuery->asString(2);
    auto checksum = selectMigrationQuery->asString(3);

    ASSERT_EQ(version.value(), "1");
    ASSERT_EQ(description.value(), "description");
    ASSERT_TRUE(installedOn.has_value());
    ASSERT_TRUE(checksum.has_value());

    auto select = std::format("select * from {};", testTableName);
    auto selectQuery = connection.buildQuery(select.c_str());

    selectQuery->next();

    auto id = selectQuery->asString(0);
    auto entry = selectQuery->asString(1);

    ASSERT_EQ(id.value(), "1");
    ASSERT_EQ(entry.value(), testTableValue);
}

TEST_F(DatabaseTest, MigrateRepeatMigration) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};
    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrationsOne{migrationOne};

    Database::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();

    ASSERT_TRUE(migrateOneResult.has_value());

    auto migrationTwo = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrationsTwo{migrationTwo};

    Database::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();

    ASSERT_TRUE(migrateTwoResult.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingChecksum) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};
    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrationsOne{migrationOne};

    Database::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();

    ASSERT_TRUE(migrateOneResult.has_value());

    auto migrationTwo = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"value1\");", testTableName, testTableColumn),
        .exec = true,
    };
    std::vector migrationsTwo{migrationTwo};

    Database::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();

    ASSERT_FALSE(migrateTwoResult.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingVersion) {  // NOLINT(cert-err58-cpp)
    Database::SQLite::Connection connection{};
    auto connect = connection.connect(databaseFile.string(), Database::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = Database::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrationsOne{migrationOne};

    Database::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();

    ASSERT_TRUE(migrateOneResult.has_value());

    auto migrationTwo = Database::Migration{
        .version = 2,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    std::vector migrationsTwo{migrationTwo};

    Database::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();

    ASSERT_FALSE(migrateTwoResult.has_value());
}