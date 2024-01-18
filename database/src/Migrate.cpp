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

#include "database/Migrate.h"
#include <filesystem>

Database::Migrate::Migrate(Connection& connection, std::vector<Migration> migrations) : connection{connection}, migrations{std::move(migrations)} {
}

Database::Result<void> Database::Migrate::createMigrationsTable() {
    auto query = this->connection.get().buildQuery(
        "CREATE TABLE IF NOT EXISTS _migrations ("
            "version integer primary key,"
            "description text not null,"
            "installed_on timestamp not null default current_timestamp,"
            "checksum text not null"
        ");"
    );

    auto next = query->next();
    while (next.has_value() && *next) {
        next = query->next();
    }

    if (!next.has_value()) {
        return Err{{.errorType = ErrorType::MigrateError, .message = next.error().message}};
    }

    auto reset = query->reset();
    if (!reset.has_value()) {
        return Err{{.errorType = ErrorType::MigrateError, .message = reset.error().message}};
    }

    return {};
}

Database::Result<std::vector<Database::Migration>> Database::Migrate::getMigrations() {
    auto query = this->connection.get().buildQuery(
        "select version, description, installed_on, checksum from _migrations order by version;"
    );
}


