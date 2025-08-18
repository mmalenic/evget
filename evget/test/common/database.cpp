#include "common/database.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <filesystem>
#include <format>
#include <string>

#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"

test::DatabaseTest::DatabaseTest()
    : directory_{std::filesystem::temp_directory_path()}, database_file_{directory_ / TestDatabaseName()} {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(database_file_.string(), evget::ConnectOptions::kReadWriteCreate);

    auto create_table = std::format(
        "create table if not exists {} ("
        "id integer primary key autoincrement,"
        "{} text"
        ");",
        kTestTableName,
        kTestTableColumn
    );
    auto insert =
        std::format("insert into {} ({}) values (\"{}\");", kTestTableName, kTestTableColumn, kTestTableValue);

    auto create_table_query = connection.BuildQuery(create_table)->Exec();
    auto insert_query = connection.BuildQuery(insert)->Exec();
}

test::DatabaseTest::~DatabaseTest() {
    std::filesystem::remove_all(DatabaseFile());
}

std::filesystem::path test::DatabaseTest::Directory() const {
    return directory_;
}

std::filesystem::path test::DatabaseTest::DatabaseFile() const {
    return database_file_;
}

std::string test::DatabaseTest::TestDatabaseName() {
    auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return std::format("test-database-{}", uuid);
}
