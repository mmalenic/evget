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

#ifndef DATABASE_MIGRATE_H
#define DATABASE_MIGRATE_H

#include <string>
#include <vector>

#include "Connection.h"

namespace EvgetCore {

struct Migration {
    int version;
    std::string description;
    std::string sql;
    bool exec;
};

struct AppliedMigration {
    int version;
    std::string checksum;
};

class Migrate {
public:
    Migrate(Connection& connection, std::vector<Migration> migrations);

    /**
     * \brief Apply the migrations.
     * \return a result indicating whether the migration was successful.
     */
    Result<void> migrate();

private:
    [[nodiscard]] Result<std::vector<AppliedMigration>> getAppliedMigrations() const;
    [[nodiscard]] Result<void> createMigrationsTable() const;
    [[nodiscard]] Result<void> applyMigration(const Migration& migration, const std::string& checksum);
    [[nodiscard]] Result<void> applyMigrationSql(const Migration& migration) const;

    static std::string checksum(const Migration& migration);

    std::reference_wrapper<Connection> connection;
    std::vector<Migration> migrations;
};
}  // namespace EvgetCore

#endif  // DATABASE_MIGRATE_H
