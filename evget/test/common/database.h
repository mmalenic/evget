#ifndef EVGET_TEST_COMMON_DATABASE_H
#define EVGET_TEST_COMMON_DATABASE_H

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <string_view>

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

private:
    std::filesystem::path directory_;
    std::filesystem::path database_file_;

    static std::string TestDatabaseName();
};

}  // namespace test

#endif
