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

#include "DatabaseTest.h"

#include <format>

#include "database/sqlite/Connection.h"

Test::Database::DatabaseTest::DatabaseTest() : directory{std::filesystem::temp_directory_path()}, databaseFile{directory / "test_database"} {
    ::Database::SQLite::Connection connection{};

    auto connect = connection.connect(databaseFile.string(), ::Database::ConnectOptions::READ_WRITE_CREATE);

    auto createTable = std::format(
            "create table if not exists {} ("
                "id integer primary key auto increment,"
                "{} text"
            ");",
            testTableName, testTableColumn);
    auto insert = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue);

    auto createTableQuery = connection.buildQuery(createTable.c_str())->exec();
    auto insertQuery = connection.buildQuery(insert.c_str())->exec();
}

Test::Database::DatabaseTest::~DatabaseTest() {
    try {
        remove_all(directory);
    } catch (std::exception& _) {
        // Ok, no throw in destructor.
    }
}
