#include <gtest/gtest.h>

#include <format>
#include <vector>

#include "common/database.h"
#include "evget/database/connection.h"
// clang-format off
#include "evget/database/migrate.h"
// clang-format on
#include "evget/database/sqlite/connection.h"

using DatabaseTest = test::DatabaseTest;

TEST_F(DatabaseTest, Migrate) {
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto migration = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations{migration};

    evget::Migrate migrate{connection, migrations};

    auto migrate_result = migrate.ApplyMigrations();

    ASSERT_TRUE(migrate_result.has_value());

    auto select_migration_query = connection.BuildQuery(std::format("select * from _migrations;"));

    select_migration_query->Next().value();

    auto version = select_migration_query->AsString(0);
    auto description = select_migration_query->AsString(1);
    auto installed_on = select_migration_query->AsString(2);
    auto checksum = select_migration_query->AsString(3);

    ASSERT_EQ(version.value(), "1");
    ASSERT_EQ(description.value(), "description");
    ASSERT_TRUE(installed_on.has_value());
    ASSERT_TRUE(checksum.has_value());

    auto select_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));

    select_query->Next().value();

    auto pos = select_query->AsString(0);
    auto entry = select_query->AsString(1);

    ASSERT_EQ(pos.value(), "1");
    ASSERT_EQ(entry.value(), kTestTableValue);
}

TEST_F(DatabaseTest, MigrateRepeatMigration) {
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto migration_one = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations_one{migration_one};

    evget::Migrate migrate_one{connection, migrations_one};

    auto migrate_one_result = migrate_one.ApplyMigrations();

    ASSERT_TRUE(migrate_one_result.has_value());

    auto migration_two = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations_two{migration_two};

    evget::Migrate migrate_two{connection, migrations_two};

    auto migrate_two_result = migrate_two.ApplyMigrations();

    ASSERT_TRUE(migrate_two_result.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingChecksum) {
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto migration_one = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations_one{migration_one};

    evget::Migrate migrate_one{connection, migrations_one};

    auto migrate_one_result = migrate_one.ApplyMigrations();
    ASSERT_TRUE(migrate_one_result.has_value());

    auto select_one_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));
    auto select_one_result = select_one_query->Next();
    ASSERT_TRUE(select_one_result.has_value());

    auto migration_two = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"value1\");", kTestTableName, kTestTableColumn),
        .exec = true,
    };
    const std::vector migrations_two{migration_two};

    evget::Migrate migrate_two{connection, migrations_two};

    auto migrate_two_result = migrate_two.ApplyMigrations();
    ASSERT_FALSE(migrate_two_result.has_value());

    auto select_two_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));
    auto select_two_result = select_two_query->Next();
    ASSERT_TRUE(select_two_result.has_value());
}

TEST_F(DatabaseTest, MigrateConflictingVersion) {
    evget::SQLiteConnection connection{};
    auto connect = connection.Connect(DatabaseFile(), evget::ConnectOptions::kReadWriteCreate);

    auto migration_one = evget::Migration{
        .version = 1,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations_one{migration_one};

    evget::Migrate migrate_one{connection, migrations_one};

    auto migrate_one_result = migrate_one.ApplyMigrations();
    ASSERT_TRUE(migrate_one_result.has_value());

    auto select_one_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));
    auto select_one_result = select_one_query->Next();
    ASSERT_TRUE(select_one_result.has_value());

    auto migration_two = evget::Migration{
        .version = 2,
        .description = "description",
        .sql = std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue),
        .exec = true,
    };
    const std::vector migrations_two{migration_two};

    evget::Migrate migrate_two{connection, migrations_two};

    auto migrate_two_result = migrate_two.ApplyMigrations();
    ASSERT_FALSE(migrate_two_result.has_value());

    auto select_two_query = connection.BuildQuery(std::format("select * from {};", kTestTableName));
    auto select_two_result = select_two_query->Next();
    ASSERT_TRUE(select_two_result.has_value());
}
