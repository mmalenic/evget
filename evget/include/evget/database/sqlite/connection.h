/**
 * \file connection.h
 * \brief SQLite database connection implementation.
 */

#ifndef EVGET_DATABASE_SQLITE_CONNECTION_H
#define EVGET_DATABASE_SQLITE_CONNECTION_H

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "evget/database/connection.h"
#include "evget/database/query.h"
#include "evget/error.h"

namespace evget {
/**
 * \brief SQLite implementation of the `Connection` interface, providing SQLite-specific database connection
 *        functionality.
 */
class SQLiteConnection : public Connection {
public:
    /**
     * \brief Default constructor for SQLiteConnection.
     */
    SQLiteConnection() = default;

    /**
     * \brief Connect to an SQLite database.
     * \param database path to the SQLite database file
     * \param options connection options
     * \return result indicating success or failure
     */
    Result<void> Connect(std::string database, ConnectOptions options) override;
    
    /**
     * \brief Start a new transaction.
     * \return result indicating success or failure
     */
    Result<void> Transaction() override;
    
    /**
     * \brief Commit the current transaction.
     * \return result indicating success or failure
     */
    Result<void> Commit() override;
    
    /**
     * \brief Rollback the current transaction.
     * \return result indicating success or failure
     */
    Result<void> Rollback() override;
    
    /**
     * \brief Build a query using this connection.
     * \param query SQL query string
     * \return pointer to a query object
     */
    std::unique_ptr<Query> BuildQuery(std::string query) override;

    /**
     * \brief Get the underlying database.
     * \return the database
     */
    [[nodiscard]] std::optional<std::reference_wrapper<SQLite::Database>> Database();

private:
    std::optional<SQLite::Database> database_;
    std::optional<SQLite::Transaction> transaction_;

    static Err ConnectError(const char* message);
};
} // namespace evget

#endif
