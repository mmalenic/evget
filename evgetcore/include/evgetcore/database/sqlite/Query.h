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

#ifndef SQLITE_QUERY_H
#define SQLITE_QUERY_H

#include <SQLiteCpp/Database.h>
#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <variant>

#include "evgetcore/database/Query.h"
#include "evgetcore/database/sqlite/Connection.h"

namespace EvgetCore {
class SQLiteQuery : public Query {
public:
    SQLiteQuery(SQLiteConnection& connection, std::string query);

    void bindInt(int position, int value) override;
    void bindDouble(int position, double value) override;
    void bindChars(int position, const char* value) override;
    void bindBool(int position, bool value) override;
    Result<void> reset() override;
    Result<bool> next() override;
    Result<void> nextWhile() override;
    Result<void> exec() override;
    Result<bool> asBool(int pos) override;
    Result<double> asDouble(int pos) override;
    Result<int> asInt(int pos) override;
    Result<std::string> asString(int pos) override;

private:
    static Err asError(const std::exception& error);
    static Err statementError();

    std::reference_wrapper<SQLiteConnection> _connection;
    std::map<int, std::variant<int, double, const char*, bool>> binds;
    std::optional<::SQLite::Statement> statement;
    std::string query;
};

}  // namespace EvgetCore

#endif  // SQLITE_QUERY_H
