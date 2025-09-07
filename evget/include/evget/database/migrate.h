#ifndef EVGET_DATABASE_MIGRATE_H
#define EVGET_DATABASE_MIGRATE_H

#include <functional>
#include <string>
#include <vector>

#include "evget/database/connection.h"
#include "evget/error.h"

namespace evget {

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
    Migrate(Connection& connection, const std::vector<Migration>& migrations);

    /**
     * \brief Apply the migrations.
     * \return a result indicating whether the migration was successful.
     */
    Result<void> ApplyMigrations();

private:
    std::reference_wrapper<Connection> connection_;
    std::vector<Migration> migrations_;

    [[nodiscard]] Result<std::vector<AppliedMigration>> GetAppliedMigrations() const;
    [[nodiscard]] Result<void> CreateMigrationsTable() const;
    [[nodiscard]] Result<void> ApplyMigration(const Migration& migration, const std::string& checksum);
    [[nodiscard]] Result<void> ApplyMigrationSql(const Migration& migration) const;

    static std::string Checksum(const Migration& migration);
};
} // namespace evget

#endif
