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

#include "database/sqlite/Builder.h"

#include <spdlog/spdlog.h>

Database::SQLite::Builder::Builder(Connection& connection) : _connection{connection} {
}

Database::Result<std::reference_wrapper<Database::Query::RowIterator>> Database::SQLite::Builder::build() {
    try {
        this->transaction.emplace(this->_connection.get().database().value());

        for (auto [position, value] : this->ints) {
            this->statement->bind(position, value);
        }
        for (auto [position, value] : this->bools) {
            this->statement->bind(position, value);
        }
        for (auto [position, value] : this->chars) {
            this->statement->bind(position, value);
        }
        for (auto [position, value] : this->doubles) {
            this->statement->bind(position, value);
        }

        this->statement->exec();
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.errorType = ErrorType::BuildError, .message = what}};
    }

    // todo
    return Query::RowIterator{this->statement.value()};
}

Database::Result<void> Database::SQLite::Builder::reset() {
    bools.clear();
    chars.clear();
    doubles.clear();
    ints.clear();

    try {
        if (statement.has_value()) {
            statement->reset();
        }
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error resetting statements: {}", what);
        return Err{{.errorType = ErrorType::BuildError, .message = what}};
    }

    return {};
}

void Database::SQLite::Builder::bindBool(std::size_t position, bool value) {
    bools[position] = value;
}

void Database::SQLite::Builder::bindChars(std::size_t position, const char* value) {
    chars[position] = value;
}

void Database::SQLite::Builder::bindDouble(std::size_t position, double value) {
    doubles[position] = value;
}

void Database::SQLite::Builder::bindInt(std::size_t position, int value) {
    ints[position] = value;
}
