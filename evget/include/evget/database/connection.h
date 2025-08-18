#ifndef EVGET_DATABASE_CONNECTION_H
#define EVGET_DATABASE_CONNECTION_H

#include <cstdint>
#include <memory>
#include <string>

#include "evget/database/query.h"
#include "evget/error.h"

namespace evget {

/**
 * \brief Connection options for connecting to a database.
 */
enum class ConnectOptions : std::uint8_t {
    /**
     * Read only connection.
     */
    kReadOnly,
    /**
     * Write connection, does not create database.
     */
    kReadWrite,
    /**
     * Write connection, creates the database if it does not exist.
     */
    kReadWriteCreate,
};

/**
 * \brief An interface representing classes that can connect to databases and perform queries.
 */
class Connection {
public:
    /**
     * \brief Connect to the database.
     * \return a result indicating whether the connection was successful.
     */
    virtual Result<void> Connect(std::string database, ConnectOptions options) = 0;

    /**
     * \brief Start a transaction. Only one transaction is allowed per connection, however this can be called again
     * after the transaction is committed.
     * \return a result indicating whether creating the transaction was successful.
     */
    virtual Result<void> Transaction() = 0;

    /**
     * \brief Commit the transaction.
     * \return a result indicating whether committing the transaction was successful.
     */
    virtual Result<void> Commit() = 0;

    /**
     * \brief Rollback the transaction.
     * \return a result indicating whether rollback was successful.
     */
    virtual Result<void> Rollback() = 0;

    /**
     * \brief Create a query using this connection.
     * \param query the query string.
     * \return a pointer to a query object.
     */
    virtual std::unique_ptr<Query> BuildQuery(std::string query) = 0;

    Connection() = default;

    virtual ~Connection() = default;

    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;
};
}  // namespace evget

#endif
