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

#include "Error.h"
#include "Store.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Schema.h"

#include <boost/asio.hpp>
#include "boost/uuid/uuid.hpp"

namespace EvgetCore::Storage {

namespace asio = boost::asio;
namespace uuids = boost::uuids;

/**
 * A storage class which stores events in an SQLite database.
 */
class SQLite : public Store {
public:
    explicit SQLite(std::string database = "evget.sqlite");

    asio::awaitable<Result<void>> store(Event::Data events) override;

    /**
     * \brief Iniitalize the database with tables.
     * \return A result indicating void if successful or an error otherwise.
     */
    Result<void> init();

private:
    std::string database;

    void setOptionalStatement(::SQLite::Database& database, std::optional<::SQLite::Statement>& statement, const char* query);
    std::string bindValues(::SQLite::Statement& statement, std::vector<std::string> data);
    void bindValuesModifier(::SQLite::Statement& statement, std::vector<std::string> modifiers, std::string entryUuid);
};
}

#endif //SQLITE_H
