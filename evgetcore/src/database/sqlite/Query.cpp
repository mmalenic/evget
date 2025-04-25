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

#include <utility>

EvgetCore::SQLiteQuery::SQLiteQuery(SQLiteConnection& connection, std::string query) : _connection{connection}, query{std::move(query)} {
}

void EvgetCore::SQLiteQuery::bindInt(std::size_t position, int value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindBool(std::size_t position, bool value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindChars(std::size_t position, const char* value) {
    binds[position] = value;
}

void EvgetCore::SQLiteQuery::bindDouble(std::size_t position, double value) {
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
        auto what = e.what();
        spdlog::error("error resetting statements: {}", what);
        return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
    }

    return {};
}

EvgetCore::Result<bool> EvgetCore::SQLiteQuery::next() {
    try {
        if (!statement.has_value()) {
            statement = {_connection.get().database()->get(), query};
        }

        for (auto [position, value] : this->binds) {
            std::visit([this, &position](auto&& value) {
                this->statement->bind(position + 1, value);
            }, value);
        }
        this->binds.clear();

        return this->statement->executeStep();
    } catch (std::exception& e) {
        auto what = e.what();
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
        _connection.get().database()->get().exec(query);
        return {};
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
    }
}


EvgetCore::Result<bool> EvgetCore::SQLiteQuery::asBool(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at).getInt();
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<double> EvgetCore::SQLiteQuery::asDouble(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<int> EvgetCore::SQLiteQuery::asInt(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Result<std::string> EvgetCore::SQLiteQuery::asString(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

EvgetCore::Err EvgetCore::SQLiteQuery::statementError() {
    auto what = "statement has not been initialized";
    spdlog::error(what);
    return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
}

EvgetCore::Err EvgetCore::SQLiteQuery::asError(std::exception& e) {
    auto what = e.what();
    spdlog::error("error getting column value: {}", what);
    return Err{{.errorType = ErrorType::DatabaseError, .message = what}};
}

