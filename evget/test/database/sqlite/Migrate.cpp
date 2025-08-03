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

#include "evget/database/Migrate.h"

#include <gtest/gtest.h>

#include <format>
#include <vector>

#include "evget/database/Connection.h"
#include "evget/database/sqlite/Connection.h"
#include "utils/DatabaseTest.h"

using DatabaseTest = Test::Database::DatabaseTest;

TEST_F(DatabaseTest, Migrate) {
    evget::SQLiteConnection connection{};
    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto migration = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrations{migration};

    evget::Migrate migrate{connection, migrations};

    auto migrateResult = migrate.migrate();

    ASSERT_TRUE(migrateResult.has_value());

    auto selectMigrationQuery = connection.buildQuery(std::format("select * from _migrations;"));

    selectMigrationQuery->next().value();

    auto version = selectMigrationQuery->asString(0);
    auto description = selectMigrationQuery->asString(1);
    auto installedOn = selectMigrationQuery->asString(2);
    auto checksum = selectMigrationQuery->asString(3);

    ASSERT_EQ(version.value(), "1");
    ASSERT_EQ(description.value(), "description");
    ASSERT_TRUE(installedOn.has_value());
    ASSERT_TRUE(checksum.has_value());

    auto selectQuery = connection.buildQuery(std::format("select * from {};", testTableName));

    selectQuery->next().value();

    auto pos = selectQuery->asString(0);
    auto entry = selectQuery->asString(1);

    ASSERT_EQ(pos.value(), "1");
    ASSERT_EQ(entry.value(), testTableValue);
}

TEST_F(DatabaseTest, MigrateRepeatMigration) {
    evget::SQLiteConnection connection{};
    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrationsOne{migrationOne};

    evget::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();

    ASSERT_TRUE(migrateOneResult.has_value());

    auto migrationTwo = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrationsTwo{migrationTwo};

    evget::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();

    ASSERT_TRUE(migrateTwoResult.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingChecksum) {
    evget::SQLiteConnection connection{};
    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrationsOne{migrationOne};

    evget::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();
    ASSERT_TRUE(migrateOneResult.has_value());

    auto selectOneQuery = connection.buildQuery(std::format("select * from {};", testTableName));
    auto selectOneResult = selectOneQuery->next();
    ASSERT_TRUE(selectOneResult.has_value());

    auto migrationTwo = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"value1\");", testTableName, testTableColumn),
        .exec = true,
    };
    const std::vector migrationsTwo{migrationTwo};

    evget::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();
    ASSERT_FALSE(migrateTwoResult.has_value());

    auto selectTwoQuery = connection.buildQuery(std::format("select * from {};", testTableName));
    auto selectTwoResult = selectTwoQuery->next();
    ASSERT_TRUE(selectTwoResult.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingVersion) {
    evget::SQLiteConnection connection{};
    auto connect = connection.connect(database_file(), evget::ConnectOptions::READ_WRITE_CREATE);

    auto migrationOne = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrationsOne{migrationOne};

    evget::Migrate migrateOne{connection, migrationsOne};

    auto migrateOneResult = migrateOne.migrate();
    ASSERT_TRUE(migrateOneResult.has_value());

    auto selectOneQuery = connection.buildQuery(std::format("select * from {};", testTableName));
    auto selectOneResult = selectOneQuery->next();
    ASSERT_TRUE(selectOneResult.has_value());

    auto migrationTwo = evget::Migration{
        .version = 2,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue),
        .exec = true,
    };
    const std::vector migrationsTwo{migrationTwo};

    evget::Migrate migrateTwo{connection, migrationsTwo};

    auto migrateTwoResult = migrateTwo.migrate();
    ASSERT_FALSE(migrateTwoResult.has_value());

    auto selectTwoQuery = connection.buildQuery(std::format("select * from {};", testTableName));
    auto selectTwoResult = selectTwoQuery->next();
    ASSERT_TRUE(selectTwoResult.has_value());
}
