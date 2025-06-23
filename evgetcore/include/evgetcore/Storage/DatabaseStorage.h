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

#include "Store.h"
#include "boost/uuid/uuid.hpp"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Schema.h"
#include "evgetcore/database/Connection.h"

namespace EvgetCore::Storage {

/**
 * A storage class which stores events in a database.
 */
class DatabaseStorage : public Store {
public:
    DatabaseStorage(std::unique_ptr<EvgetCore::Connection> connection, std::string database);

    Result<void> store(Event::Data events) override;

    /**
     * \brief Iniitalize the database with tables.
     * \return A result indicating void if successful or an error otherwise.
     */
    Result<void> init();

private:
    std::unique_ptr<EvgetCore::Connection> connection;
    std::string database;

    Result<void> insertEvents(
        const Event::Entry& entry,
        std::optional<std::unique_ptr<::EvgetCore::Query>>& insertStatement,
        std::optional<std::unique_ptr<::EvgetCore::Query>>& insertModifierStatement,
        std::string insertQuery,
        std::string insertModifierQuery
    );
    void setOptionalStatement(std::optional<std::unique_ptr<::EvgetCore::Query>>& query, std::string queryString);
    Result<void>
    bindValues(std::unique_ptr<::EvgetCore::Query>& query, std::vector<std::string> data, std::string entryUuid);
    Result<void> bindValuesModifier(
        std::unique_ptr<::EvgetCore::Query>& query,
        std::vector<std::string> modifiers,
        std::string entryUuid
    );
};
}  // namespace EvgetCore::Storage

#endif  // DATABASE_STORAGE_H
