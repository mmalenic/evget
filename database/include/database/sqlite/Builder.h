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

#ifndef SQLITE_BUILDER_H
#define SQLITE_BUILDER_H

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include "database/query/Builder.h"
#include "database/sqlite/Connection.h"

namespace Database::SQLite {
class Builder : Query::Builder {
public:
    explicit Builder(Connection& connection);

    void bindInt(std::size_t position, int value) override;
    void bindDouble(std::size_t position, double value) override;
    void bindChars(std::size_t position, const char* value) override;
    void bindBool(std::size_t position, bool value) override;
    Result<void> reset() override;
    Result<std::reference_wrapper<Query::RowIterator>> build() override;
private:
    std::reference_wrapper<Connection> _connection;

    std::map<std::size_t, int> ints;
    std::map<std::size_t, double> doubles;
    std::map<std::size_t, const char*> chars;
    std::map<std::size_t, bool> bools;

    std::optional<::SQLite::Transaction> transaction{};
    std::optional<::SQLite::Statement> statement{};
};
}


#endif // SQLITE_BUILDER_H
