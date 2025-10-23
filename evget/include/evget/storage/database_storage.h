#ifndef EVGET_STORAGE_DATABASE_STORAGE_H
#define EVGET_STORAGE_DATABASE_STORAGE_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "evget/database/connection.h"
#include "evget/database/query.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/entry.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief A storage class which stores events in a database.
 */
class DatabaseStorage : public Store {
public:
    /**
     * \brief Construct a DatabaseStorage with a connection and database path.
     * \param connection unique pointer to the database connection
     * \param database path to the database file
     */
    DatabaseStorage(std::unique_ptr<Connection> connection, std::string database);

    Result<void> StoreEvent(Data events) override;

    /**
     * \brief Initialize the database with tables.
     * \return a result indicating success of failure
     */
    [[nodiscard]] Result<void> Init() const;

private:
    std::unique_ptr<Connection> connection_;
    std::string database_;

    Result<void> InsertEvents(
        const Entry& entry,
        std::optional<std::unique_ptr<Query>>& insert_statement,
        std::optional<std::unique_ptr<Query>>& insert_modifier_statement,
        std::string insert_query,
        std::string insert_modifier_query
    ) const;
    void SetOptionalStatement(std::optional<std::unique_ptr<Query>>& query, std::string query_string) const;
    static Result<void>
    BindValues(std::unique_ptr<Query>& query, const std::vector<std::string>& data, const std::string& entry_uuid);
    static Result<void> BindValuesModifier(
        std::unique_ptr<Query>& query,
        const std::vector<std::string>& modifiers,
        const std::string& entry_uuid
    );
};
} // namespace evget

#endif
