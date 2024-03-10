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

#include <filesystem>
#include <gtest/gtest.h>

namespace Test::Database {

/**
 * \brief Creates a temporary directory and database file for database tests.
 */
class DatabaseTest : public testing::Test {
protected:
    const std::string_view testTableName = "table";
    const std::string_view testTableColumn = "column";
    const std::string_view testTableValue = "value";

    DatabaseTest();
    ~DatabaseTest() override;

    std::filesystem::path directory;
    std::filesystem::path databaseFile;
};

}


#endif //CONNECTIONTEST_H
