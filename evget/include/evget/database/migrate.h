/**
 * \file migrate.h
 * \brief Database migration management for schema versioning and updates.
 */

#ifndef EVGET_DATABASE_MIGRATE_H
#define EVGET_DATABASE_MIGRATE_H

#include <functional>
#include <string>
#include <vector>

#include "evget/database/connection.h"
#include "evget/error.h"

namespace evget {

/**
 * \brief Represents a database migration with version, description, SQL, and execution flag.
 */
struct Migration {
    int version; ///< Migration version number
    std::string description; ///< Human-readable description of the migration
    std::string sql; ///< SQL statements to execute for this migration
    bool exec; ///< Whether this migration should be executed
};

/**
 * \brief Represents a migration that has been applied to the database.
 */
struct AppliedMigration {
    int version; ///< Version number of the applied migration
    std::string checksum; ///< Checksum of the migration
};

/**
 * \brief Manages database migrations, applying them in order and tracking applied migrations.
 */
class Migrate {
public:
    /**
     * \brief Construct a migration manager.
     * \param connection Database connection to use for migrations
     * \param migrations Vector of migrations to manage
     */
    Migrate(Connection& connection, const std::vector<Migration>& migrations);

    /**
     * \brief Apply the migrations.
     * \return a result indicating whether the migration was successful
     */
    Result<void> ApplyMigrations();

private:
    std::reference_wrapper<Connection> connection_;
    std::vector<Migration> migrations_;

    [[nodiscard]] Result<std::vector<AppliedMigration>> GetAppliedMigrations() const;
    [[nodiscard]] Result<void> CreateMigrationsTable() const;
    [[nodiscard]] Result<void> ApplyMigration(const Migration& migration, const std::string& checksum) const;
    [[nodiscard]] Result<void> ApplyMigrationSql(const Migration& migration) const;

    static std::string Checksum(const Migration& migration);
};
} // namespace evget

#endif
