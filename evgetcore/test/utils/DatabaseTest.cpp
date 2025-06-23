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

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fcntl.h>

#include <format>

#include "evgetcore/database/sqlite/Connection.h"

namespace uuids = boost::uuids;

Test::Database::DatabaseTest::DatabaseTest()
    : directory{std::filesystem::temp_directory_path()}, databaseFile{directory / testDatabaseName()} {
    EvgetCore::SQLiteConnection connection{};

    auto connect = connection.connect(databaseFile.string(), EvgetCore::ConnectOptions::READ_WRITE_CREATE);

    auto createTable = std::format(
        "create table if not exists {} ("
        "id integer primary key autoincrement,"
        "{} text"
        ");",
        testTableName,
        testTableColumn
    );
    auto insert = std::format("insert into {} ({}) values (\"{}\");", testTableName, testTableColumn, testTableValue);

    auto createTableQuery = connection.buildQuery(createTable.c_str())->exec();
    auto insertQuery = connection.buildQuery(insert.c_str())->exec();
}

Test::Database::DatabaseTest::~DatabaseTest() {
    try {
        remove_all(databaseFile);
    } catch (std::exception& _) {
        // Ok, no throw in destructor.
    }
}

std::string Test::Database::DatabaseTest::testDatabaseName() {
    auto uuid = uuids::to_string(uuids::random_generator()());
    return std::format("test-database-{}", uuid);
}
