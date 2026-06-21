#ifndef EVGET_TEST_COMMON_DATABASE_H
#define EVGET_TEST_COMMON_DATABASE_H

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/database_storage.h"
#include "evget/storage/store.h"

namespace test {

/**
 * \brief Creates a temporary directory and database file for database tests.
 */
class DatabaseTest : public testing::Test {
public:
    constexpr static std::string_view kTestTableName = "test_table";
    constexpr static std::string_view kTestTableColumn = "test_column";
    constexpr static std::string_view kTestTableValue = "test_value";

    DatabaseTest();
    ~DatabaseTest() override;

    DatabaseTest(const DatabaseTest&) = delete;
    DatabaseTest(DatabaseTest&&) noexcept = delete;
    DatabaseTest& operator=(const DatabaseTest&) = delete;
    DatabaseTest& operator=(DatabaseTest&&) noexcept = delete;

    [[nodiscard]] std::filesystem::path Directory() const;
    [[nodiscard]] std::filesystem::path DatabaseFile() const;
    [[nodiscard]] evget::DatabaseStorage MakeStorage() const;

private:
    std::filesystem::path directory_;
    std::filesystem::path database_file_;

    static std::string TestDatabaseName();
};

/**
 * \brief Store that detects overlapping writes and counts stored entries.
 */
class ConcurrencyStore : public evget::Store {
public:
    evget::Result<void> StoreEvent(evget::Data event) override;
    void WaitForEntries(std::size_t count);
    [[nodiscard]] bool Concurrent() const;
    [[nodiscard]] std::size_t TotalEntries() const;

private:
    std::atomic<int> in_flight_{0};
    std::atomic<bool> concurrent_{false};
    std::atomic<std::size_t> total_entries_{0};
};

} // namespace test

#endif
