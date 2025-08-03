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

#include "evget/database/sqlite/Connection.h"

#include <SQLiteCpp/Database.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "evget/Error.h"
#include "evget/database/Connection.h"
#include "evget/database/Query.h"
#include "evget/database/sqlite/Query.h"

evget::Result<void> evget::SQLiteConnection::connect(std::string database, ConnectOptions options) {
    try {
        switch (options) {
            case ConnectOptions::READ_ONLY:
                this->database_ = {database, ::SQLite::OPEN_READONLY};
                break;
            case ConnectOptions::READ_WRITE:
                this->database_ = {database, ::SQLite::OPEN_READWRITE};
                break;
            case ConnectOptions::READ_WRITE_CREATE:
                // NOLINTBEGIN(hicpp-signed-bitwise)
                this->database_ = {database, ::SQLite::OPEN_READWRITE | ::SQLite::OPEN_CREATE};
                // NOLINTEND(hicpp-signed-bitwise)
                break;
        }

        spdlog::info("connected to SQLite database: {}", database);
        return {};
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error connecting to SQLite database: {}", what);
        return connectError(what);
    }
}

std::optional<std::reference_wrapper<::SQLite::Database>> evget::SQLiteConnection::database() {
    return database_.transform([](::SQLite::Database& database) { return std::ref(database); });
}

evget::Result<void> evget::SQLiteConnection::commit() {
    if (!transaction_.has_value()) {
        return connectError("transaction not started");
    }

    try {
        transaction_->commit();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error committing transaction: {}", what);
        return connectError(what);
    }

    return {};
}

evget::Result<void> evget::SQLiteConnection::transaction() {
    if (!this->database_.has_value()) {
        return connectError("no database connected");
    }

    try {
        this->transaction_.emplace(*this->database_);
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error creating transaction: {}", what);
        return connectError(what);
    }

    return {};
}

std::unique_ptr<evget::Query> evget::SQLiteConnection::buildQuery(std::string query) {
    return std::make_unique<evget::SQLiteQuery>(*this, query);
}

evget::Err evget::SQLiteConnection::connectError(const char* message) {
    return Err{Error{.errorType = ErrorType::DatabaseError, .message = message}};
}

evget::Result<void> evget::SQLiteConnection::rollback() {
    if (!transaction_.has_value()) {
        return connectError("transaction not started");
    }

    try {
        transaction_->rollback();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error rolling back transaction: {}", what);
        return connectError(what);
    }

    return {};
}
