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

#ifndef SQLITE_CPP_H
#define SQLITE_CPP_H

#include "evget/database/Connection.h"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>
#include <evget/Error.h>
#include <evget/database/Query.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace evget {
class SQLiteConnection : public Connection {
public:
    SQLiteConnection() = default;

    Result<void> Connect(std::string database, ConnectOptions options) override;
    Result<void> Transaction() override;
    Result<void> Commit() override;
    Result<void> Rollback() override;
    std::unique_ptr<Query> BuildQuery(std::string query) override;

    /**
     * \brief Get the underyling database.
     * \return the database.
     */
    [[nodiscard]] std::optional<std::reference_wrapper<::SQLite::Database>> Database();

private:
    std::optional<::SQLite::Database> database_;
    std::optional<::SQLite::Transaction> transaction_;

    static Err ConnectError(const char* message);
};
}  // namespace evget

#endif  // SQLITE_CPP_H
