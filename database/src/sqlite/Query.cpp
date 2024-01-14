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

#include "database/sqlite/Query.h"

#include <spdlog/spdlog.h>

Database::SQLite::Query::Query(Connection& connection, const char * query) : _connection{connection}, query{query} {
}

void Database::SQLite::Query::bindInt(std::size_t position, int value) {
    binds[position] = value;
}

void Database::SQLite::Query::bindBool(std::size_t position, bool value) {
    binds[position] = value;
}

void Database::SQLite::Query::bindChars(std::size_t position, const char* value) {
    binds[position] = value;
}

void Database::SQLite::Query::bindDouble(std::size_t position, double value) {
    binds[position] = value;
}

Database::Result<void> Database::SQLite::Query::reset() {
    this->binds.clear();

    try {
        if (statement.has_value()) {
            statement->reset();
        }
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error resetting statements: {}", what);
        return Err{{.errorType = ErrorType::QueryError, .message = what}};
    }

    return {};
}

Database::Result<bool> Database::SQLite::Query::next() {
    try {
        if (!statement.has_value()) {
            statement = {_connection.get().database()->get(), query};
        }

        for (auto [position, value] : this->binds) {
            std::visit([this, &position](auto&& value) {
                this->statement->bind(position, value);
            }, value);
        }
        this->binds.clear();

        return this->statement->executeStep();
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.errorType = ErrorType::QueryError, .message = what}};
    }
}

Database::Result<bool> Database::SQLite::Query::asBool(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at).getInt();
    } catch (std::exception& e) {
        return asError(e);
    }
}

Database::Result<double> Database::SQLite::Query::asDouble(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

Database::Result<int> Database::SQLite::Query::asInt(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

Database::Result<std::string> Database::SQLite::Query::asString(std::size_t at) {
    if (!this->statement.has_value()) {
        return statementError();
    }

    try {
        return this->statement->getColumn(at);
    } catch (std::exception& e) {
        return asError(e);
    }
}

Database::Err Database::SQLite::Query::statementError() {
    auto what = "statement has not been initialized";
    spdlog::error(what);
    return Err{{.errorType = ErrorType::QueryError, .message = what}};
}

Database::Err Database::SQLite::Query::asError(std::exception& e) {
    auto what = e.what();
    spdlog::error("error getting column value: {}", what);
    return Err{{.errorType = ErrorType::QueryError, .message = what}};
}

