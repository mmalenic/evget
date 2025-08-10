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

#include <SQLiteCpp/Statement.h>

#include <exception>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "evget/Error.h"
#include "evget/database/Query.h"
#include "evget/database/sqlite/Connection.h"

namespace evget {
class SQLiteQuery : public Query {
public:
    SQLiteQuery(SQLiteConnection& connection, std::string query);

    void BindInt(int position, int value) override;
    void BindDouble(int position, double value) override;
    void BindChars(int position, const char* value) override;
    void BindBool(int position, bool value) override;
    Result<void> Reset() override;
    Result<bool> Next() override;
    Result<void> NextWhile() override;
    Result<void> Exec() override;
    Result<bool> AsBool(int pos) override;
    Result<double> AsDouble(int pos) override;
    Result<int> AsInt(int pos) override;
    Result<std::string> AsString(int pos) override;

private:
    static Err AsError(const std::exception& error);
    static Err StatementError();

    std::reference_wrapper<SQLiteConnection> connection_;
    std::map<int, std::variant<int, double, const char*, bool>> binds_;
    std::optional<::SQLite::Statement> statement_;
    std::string query_;
};

}  // namespace evget

#endif  // SQLITE_QUERY_H
