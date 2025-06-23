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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>

#include "Query.h"
#include "evgetcore/Error.h"

namespace EvgetCore {

/**
 * \brief Connection options for connecting to a database.
 */
enum class ConnectOptions : std::uint8_t {
    /**
     * Read only connection.
     */
    READ_ONLY,
    /**
     * Write connection, does not create database.
     */
    READ_WRITE,
    /**
     * Write connection, creates the database if it does not exist.
     */
    READ_WRITE_CREATE,
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
    virtual Result<void> connect(std::string database, ConnectOptions options) = 0;

    /**
     * \brief Start a transaction. Only one transaction is allowed per connection, however this can be called again
     * after the transaction is committed.
     * \return a result indicating whether creating the transaction was successful.
     */
    virtual Result<void> transaction() = 0;

    /**
     * \brief Commit the transaction.
     * \return a result indicating whether committing the transaction was successful.
     */
    virtual Result<void> commit() = 0;

    /**
     * \brief Rollback the transaction.
     * \return a result indicating whether rollback was successful.
     */
    virtual Result<void> rollback() = 0;

    /**
     * \brief Create a query using this connection.
     * \param query the query string.
     * \return a pointer to a query object.
     */
    virtual std::unique_ptr<Query> buildQuery(std::string query) = 0;

    Connection() = default;

    virtual ~Connection() = default;

    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;
};
}  // namespace EvgetCore

#endif  // CONNECTION_H
