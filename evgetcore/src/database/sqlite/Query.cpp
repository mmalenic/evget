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

#include "evgetcore/database/sqlite/Query.h"

#include <spdlog/spdlog.h>

#include <exception>
#include <expected>
#include <functional>
#include <string>
#include <utility>
#include <variant>

#include "evgetcore/Error.h"
#include "evgetcore/database/sqlite/Connection.h"

EvgetCore::SQLiteQuery::SQLiteQuery(SQLiteConnection& connection, std::string query)
    : _connection{connection}, query{std::move(query)} {}

void EvgetCore::SQLiteQuery::bindInt(int position, int value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindBool(int position, bool value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindChars(int position, const char* value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindDouble(int position, double value) {
    binds[position] = value;
}

EvgetCore::Result<void> EvgetCore::SQLiteQuery::reset() {
    this->binds.clear();

    try {
        if (statement.has_value()) {
            statement->reset();
            statement->clearBindings();
        }
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error resetting statements: {}", what);
        return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
    }

    return {};
}

EvgetCore::Result<bool> EvgetCore::SQLiteQuery::next() {
    try {
        auto database = _connection.get().database();
        if (!database.has_value()) {
            return Err{{.errorType = ErrorType::DatabaseError, .message = "database not set"}};
        }
        if (!statement.has_value()) {
            statement = {database->get(), query};
        }

        for (auto [position, value] : this->binds) {
            std::visit([this, &position](auto&& value) { this->statement->bind(position + 1, value); }, value);
        }
        this->binds.clear();

        return this->statement->executeStep();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
    }
}

EvgetCore::Result<void> EvgetCore::SQLiteQuery::nextWhile() {
    auto next = this->next();
    while (next.has_value() && *next) {
        next = this->next();
    }

    if (!next.has_value()) {
        return Err{next.error()};
    }

    return {};
}

EvgetCore::Result<void> EvgetCore::SQLiteQuery::exec() {
    try {
        auto database = _connection.get().database();
        if (!database.has_value()) {
            return Err{{.errorType = ErrorType::DatabaseError, .message = "database not set"}};
        }

        database->get().exec(query);
        return {};
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
    }
}

EvgetCore::Result<bool> EvgetCore::SQLiteQuery::asBool(int pos) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(pos).getInt();
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<double> EvgetCore::SQLiteQuery::asDouble(int pos) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(pos);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<int> EvgetCore::SQLiteQuery::asInt(int pos) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(pos);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<std::string> EvgetCore::SQLiteQuery::asString(int pos) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(pos);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Err EvgetCore::SQLiteQuery::statementError() {
    const auto* what = "statement has not been initialized";
    spdlog::error(what);
    return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
}

EvgetCore::Err EvgetCore::SQLiteQuery::asError(const std::exception& error) {
    const auto* what = error.what();
    spdlog::error("error getting column value: {}", what);
    return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
}
