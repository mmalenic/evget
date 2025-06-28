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

#ifndef CONNECTIONTEST_H
#define CONNECTIONTEST_H

#include <gtest/gtest.h>

#include <filesystem>

namespace Test::Database {

/**
 * \brief Creates a temporary directory and database file for database tests.
 */
class DatabaseTest : public testing::Test {
public:
    constexpr static std::string_view testTableName = "test_table";
    constexpr static std::string_view testTableColumn = "test_column";
    constexpr static std::string_view testTableValue = "test_value";

    DatabaseTest();
    ~DatabaseTest() override;

    DatabaseTest(const DatabaseTest&) = delete;
    DatabaseTest(DatabaseTest&&) noexcept = delete;
    DatabaseTest& operator=(const DatabaseTest&) = delete;
    DatabaseTest& operator=(DatabaseTest&&) noexcept = delete;

    [[nodiscard]] std::filesystem::path directory() const;
    [[nodiscard]] std::filesystem::path database_file() const;

private:
    std::filesystem::path directory_;
    std::filesystem::path databaseFile;

    static std::string testDatabaseName();
};

}  // namespace Test::Database

#endif  // CONNECTIONTEST_H
