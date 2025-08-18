#include "evget/storage/database_storage.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "evget/database/connection.h"
#include "evget/database/migrate.h"
#include "evget/database/query.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/entry.h"
#include "queries/insert_key.h"
#include "queries/insert_key_modifier.h"
#include "queries/insert_mouse_click.h"
#include "queries/insert_mouse_click_modifier.h"
#include "queries/insert_mouse_move.h"
#include "queries/insert_mouse_move_modifier.h"
#include "queries/insert_mouse_scroll.h"
#include "queries/insert_mouse_scroll_modifier.h"
#include "schema/initialize.h"

evget::DatabaseStorage::DatabaseStorage(std::unique_ptr<::evget::Connection> connection, std::string database)
    : connection_{std::move(connection)}, database_{std::move(database)} {}

evget::Result<void> evget::DatabaseStorage::StoreEvent(Data events) {
    return connection_->Connect(database_, ::evget::ConnectOptions::kReadWriteCreate)
        .and_then([this] { return connection_->Transaction(); })
        .transform_error([](const Error<::evget::ErrorType>& error) {
            return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
        })
        .and_then([this, &events] {
            std::optional<std::unique_ptr<::evget::Query>> insert_key{};
            std::optional<std::unique_ptr<::evget::Query>> insert_key_modifier{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_move{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_move_modifier{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_click{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_click_modifier{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_scroll{};
            std::optional<std::unique_ptr<::evget::Query>> insert_mouse_scroll_modifier{};

            for (const auto& entry : events.Entries()) {
                if (entry.Data().empty()) {
                    continue;
                }

                Result<void> result;
                switch (entry.Type()) {
                    case EntryType::kKey:
                        result = InsertEvents(
                            entry,
                            insert_key,
                            insert_key_modifier,
                            evget::detail::insert_key,
                            evget::detail::insert_key_modifier
                        );
                        break;
                    case EntryType::kMouseClick:
                        result = InsertEvents(
                            entry,
                            insert_mouse_click,
                            insert_mouse_click_modifier,
                            evget::detail::insert_mouse_click,
                            evget::detail::insert_mouse_click_modifier
                        );
                        break;
                    case EntryType::kMouseMove:
                        result = InsertEvents(
                            entry,
                            insert_mouse_move,
                            insert_mouse_move_modifier,
                            evget::detail::insert_mouse_move,
                            evget::detail::insert_mouse_move_modifier
                        );
                        break;
                    case EntryType::kMouseScroll:
                        result = InsertEvents(
                            entry,
                            insert_mouse_scroll,
                            insert_mouse_scroll_modifier,
                            evget::detail::insert_mouse_scroll,
                            evget::detail::insert_mouse_scroll_modifier
                        );
                        break;
                }

                if (!result.has_value()) {
                    return result;
                }
            }

            return this->connection_->Commit().transform_error([](const Error<::evget::ErrorType>& error) {
                return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
            });
        });
}

evget::Result<void> evget::DatabaseStorage::Init() {
    auto result =
        connection_->Connect(database_, ::evget::ConnectOptions::kReadWriteCreate)
            .transform_error([](const Error<::evget::ErrorType>& error) {
                return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
            })
            .and_then([this] {
                auto migrations = std::vector{::evget::Migration{
                    .version = 1,
                    .description = "initialize database tables",
                    .sql = evget::detail::initialize,
                    .exec = true,
                }};
                auto apply_migrations = Migrate{*this->connection_, migrations};

                return apply_migrations.ApplyMigrations().transform_error([](const Error<::evget::ErrorType>& error) {
                    return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
                });
            });

    return result;
}

evget::Result<void> evget::DatabaseStorage::InsertEvents(
    const Entry& entry,
    std::optional<std::unique_ptr<::evget::Query>>& insert_statement,
    std::optional<std::unique_ptr<::evget::Query>>& insert_modifier_statement,
    std::string insert_query,
    std::string insert_modifier_query
) {
    SetOptionalStatement(insert_statement, std::move(insert_query));
    SetOptionalStatement(insert_modifier_statement, std::move(insert_modifier_query));

    auto entry_uuid = to_string(boost::uuids::random_generator()());

    // Optional is set in previous lines.
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    return BindValues(*insert_statement, entry.Data(), entry_uuid)
        .and_then([this, &insert_modifier_statement, &entry, &entry_uuid] {
            return BindValuesModifier(*insert_modifier_statement, entry.Modifiers(), entry_uuid);
        });
    // NOLINTEND(bugprone-unchecked-optional-access)
}

void evget::DatabaseStorage::SetOptionalStatement(
    std::optional<std::unique_ptr<::evget::Query>>& query,
    std::string query_string
) {
    if (!query.has_value()) {
        query = {connection_->BuildQuery(std::move(query_string))};
    }
}

evget::Result<void> evget::DatabaseStorage::BindValues(
    std::unique_ptr<::evget::Query>& query,
    const std::vector<std::string>& data,
    const std::string& entry_uuid
) {
    query->BindChars(0, entry_uuid.c_str());
    for (auto i = 0; i < data.size(); i++) {
        query->BindChars(i + 1, data[i].c_str());
    }

    return query->NextWhile().and_then(
                                 [&query] { return (*query).Reset(); }
    ).transform_error([](const Error<::evget::ErrorType>& error) {
        return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
    });
}

evget::Result<void> evget::DatabaseStorage::BindValuesModifier(
    std::unique_ptr<::evget::Query>& query,
    const std::vector<std::string>& modifiers,
    const std::string& entry_uuid
) {
    for (const auto& modifier : modifiers) {
        auto modifier_uuid = to_string(boost::uuids::random_generator()());
        query->BindChars(0, modifier_uuid.c_str());
        query->BindChars(1, entry_uuid.c_str());
        query->BindChars(2, modifier.c_str());

        auto result = query->NextWhile().and_then([&query] { return (*query).Reset(); });
        if (!result.has_value()) {
            return Err{{.error_type = ErrorType::kDatabaseError, .message = result.error().message}};
        }
    }

    return {};
}
