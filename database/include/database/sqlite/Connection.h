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

#ifndef SQLITE_H
#define SQLITE_H

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include <optional>

#include "database/Connection.h"
#include "database/Query.h"

namespace Database::SQLite {
class Connection : public Database::Connection {
public:
    Connection() = default;

    Result<void> connect(std::string database, ConnectOptions options) override;
    Result<void> transaction() override;
    Result<void> commit() override;
    Result<void> rollback() override;
    std::unique_ptr<Query> buildQuery(const char* query) override;

    Result<void> lock() override;
    Result<void> unlock() override;

    /**
     * \brief Get the underyling database.
     * \return the database.
     */
    [[nodiscard]] std::optional<std::reference_wrapper<::SQLite::Database>> database();

private:
    std::optional<::SQLite::Database> database_;
    std::optional<::SQLite::Transaction> transaction_;

    static Err connectError(const char* message);
};
}

#endif //SQLITE_H
