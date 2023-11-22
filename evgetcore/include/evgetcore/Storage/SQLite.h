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
#include "Storage.h"
#include "evgetcore/Event/Schema.h"

namespace EvgetCore::Storage {

/**
 * A storage class which stores events in an SQLite database.
 */
class SQLite : public Storage {
public:
    explicit SQLite(std::string database = "evget.sqlite");

    void store(Event::Data event) override;

    /**
     * \brief Iniitalize the database with tables.
     * \return A result indicating void if successful or an error otherwise.
     */
    Result<void> init();

private:
    std::string database;
};
}

#endif //SQLITE_H
