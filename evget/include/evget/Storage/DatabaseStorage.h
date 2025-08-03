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

#ifndef DATABASE_STORAGE_H
#define DATABASE_STORAGE_H

#include <SQLiteCpp/Database.h>

#include <memory>
#include <string>
#include <vector>

#include "evget/Event/Data.h"
#include "evget/Event/Schema.h"
#include "evget/Storage/Store.h"
#include "evget/database/Connection.h"

namespace evget::Storage {

/**
 * A storage class which stores events in a database.
 */
class DatabaseStorage : public Store {
public:
    DatabaseStorage(std::unique_ptr<evget::Connection> connection, std::string database);

    Result<void> store(Event::Data events) override;

    /**
     * \brief Iniitalize the database with tables.
     * \return A result indicating void if successful or an error otherwise.
     */
    Result<void> init();

private:
    std::unique_ptr<evget::Connection> connection;
    std::string database;

    Result<void> insertEvents(
        const Event::Entry& entry,
        std::optional<std::unique_ptr<::evget::Query>>& insertStatement,
        std::optional<std::unique_ptr<::evget::Query>>& insertModifierStatement,
        std::string insertQuery,
        std::string insertModifierQuery
    );
    void setOptionalStatement(std::optional<std::unique_ptr<::evget::Query>>& query, std::string queryString) const;
    static Result<void> bindValues(
        std::unique_ptr<::evget::Query>& query,
        const std::vector<std::string>& data,
        const std::string& entryUuid
    );
    static Result<void> bindValuesModifier(
        std::unique_ptr<::evget::Query>& query,
        const std::vector<std::string>& modifiers,
        const std::string& entryUuid
    );
};
}  // namespace evget::Storage

#endif  // DATABASE_STORAGE_H
