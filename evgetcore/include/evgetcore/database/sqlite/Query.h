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
#include <SQLiteCpp/Transaction.h>
#include <spdlog/spdlog.h>

#include <variant>

#include "database/Query.h"
#include "database/sqlite/Connection.h"

namespace Database::SQLite {
class Query : public Database::Query {
public:
    Query(Connection& connection, std::string query);

    void bindInt(std::size_t position, int value) override;
    void bindDouble(std::size_t position, double value) override;
    void bindChars(std::size_t position, const char* value) override;
    void bindBool(std::size_t position, bool value) override;
    Result<void> reset() override;
    Result<bool> next() override;
    Result<void> nextWhile() override;
    Result<void> exec() override;
    Result<bool> asBool(std::size_t at) override;
    Result<double> asDouble(std::size_t at) override;
    Result<int> asInt(std::size_t at) override;
    Result<std::string> asString(std::size_t at) override;

private:
    static Err asError(std::exception& e);
    static Err statementError();

    std::reference_wrapper<Connection> _connection;
    std::map<std::size_t, std::variant<int, double, const char*, bool>> binds;
    std::optional<::SQLite::Statement> statement;
    std::string query;
};

}


#endif // SQLITE_QUERY_H
