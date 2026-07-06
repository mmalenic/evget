#include "common/database.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <system_error>
#include <thread>

#include "evget/database/connection.h"
#include "evget/database/sqlite/connection.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/database_storage.h"

test::DatabaseTest::DatabaseTest()
    : directory_{std::filesystem::temp_directory_path()}, database_file_{directory_ / TestDatabaseName()} {
    evget::SQLiteConnection connection{};

    auto connect = connection.Connect(database_file_, evget::ConnectOptions::kReadWriteCreate);

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
    std::error_code error_code{};
    std::filesystem::remove_all(database_file_, error_code);
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

evget::DatabaseStorage test::DatabaseTest::MakeStorage() const {
    return {std::make_unique<evget::SQLiteConnection>(), DatabaseFile()};
}

evget::Result<void> test::ConcurrencyStore::StoreEvent(evget::Data event) {
    if (in_flight_.fetch_add(1) != 0) {
        concurrent_.store(true);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
    in_flight_.fetch_sub(1);

    total_entries_.fetch_add(event.Entries().size());
    return {};
}

void test::ConcurrencyStore::WaitForEntries(std::size_t count) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    for (int i = 0; i < 5000 && total_entries_.load() < count; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
}

bool test::ConcurrencyStore::Concurrent() const {
    return concurrent_.load();
}

std::size_t test::ConcurrencyStore::TotalEntries() const {
    return total_entries_.load();
}
