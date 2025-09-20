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
class SQLiteConnection : public Connection {
public:
    SQLiteConnection() = default;

    Result<void> Connect(std::string database, ConnectOptions options) override;
    Result<void> Transaction() override;
    Result<void> Commit() override;
    Result<void> Rollback() override;
    std::unique_ptr<Query> BuildQuery(std::string query) override;

    /**
     * \brief Get the underyling database.
     * \return the database.
     */
    [[nodiscard]] std::optional<std::reference_wrapper<SQLite::Database>> Database();

private:
    std::optional<SQLite::Database> database_;
    std::optional<SQLite::Transaction> transaction_;

    static Err ConnectError(const char* message);
};
} // namespace evget

#endif
