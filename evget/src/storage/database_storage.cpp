#include "evget/storage/database_storage.h"

#include <boost/uuid/uuid_io.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
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

namespace {

constexpr std::array<std::string_view, 3> kNotNullFields{"timestamp", "device_type", "button_action"};

std::span<const std::string_view> FieldNames(evget::EntryType type) {
    switch (type) {
        case evget::EntryType::kKey:
            return evget::detail::kKeyFields;
        case evget::EntryType::kMouseClick:
            return evget::detail::kMouseClickFields;
        case evget::EntryType::kMouseMove:
            return evget::detail::kMouseMoveFields;
        case evget::EntryType::kMouseScroll:
            return evget::detail::kMouseScrollFields;
    }
    return {};
}

std::optional<std::string_view> AbsentNotNullField(const evget::Entry& entry) {
    const auto& data = entry.Data();
    for (const auto& [index, name] : std::views::enumerate(FieldNames(entry.Type()))) {
        const auto absent = std::cmp_greater_equal(index, data.size()) || !data.at(index).has_value();
        if (absent && std::ranges::contains(kNotNullFields, name)) {
            return name;
        }
    }
    return std::nullopt;
}

evget::Result<void> BindValues(
    std::unique_ptr<evget::Query>& query,
    const std::vector<evget::EntryValue>& data,
    const std::string& entry_uuid
) {
    query->BindChars(0, entry_uuid.c_str());
    for (const auto& [index, value] : std::views::enumerate(data)) {
        const auto position = static_cast<int>(index) + 1;
        if (value.has_value()) {
            query->BindChars(position, value->c_str());
        } else {
            query->BindNull(position);
        }
    }

    return query->NextWhile().and_then(
                                 [&query] { return (*query).Reset(); }
    ).transform_error([](const evget::Error<evget::ErrorType>& error) {
        return evget::Error{.error_type = evget::ErrorType::kDatabaseError, .message = error.message};
    });
}

} // namespace

evget::DatabaseStorage::DatabaseStorage(std::unique_ptr<Connection> connection, std::filesystem::path database)
    : connection_{std::move(connection)}, database_{std::move(database)} {}

evget::Result<void> evget::DatabaseStorage::EnsureConnected() {
    if (connected_) {
        return {};
    }

    return connection_->Connect(database_, ConnectOptions::kReadWriteCreate)
        .transform_error([](const Error<ErrorType>& error) {
            return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
        })
        .transform([this] { connected_ = true; });
}

evget::Result<void> evget::DatabaseStorage::ApplyPragmas() const {
    auto query = connection_->BuildQuery(
        "pragma journal_mode=WAL;"
        "pragma synchronous=NORMAL;"
        "pragma temp_store=MEMORY;"
    );
    return query->Exec().transform_error([](const Error<ErrorType>& error) {
        return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
    });
}

evget::Result<void> evget::DatabaseStorage::StoreEvent(Data events) {
    if (events.Empty()) {
        return {};
    }

    return EnsureConnected()
        .and_then([this] { return connection_->Transaction(); })
        .transform_error([](const Error<ErrorType>& error) {
            return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
        })
        .and_then([this, &events] {
            auto inserted = InsertEntries(events);
            if (!inserted.has_value()) {
                static_cast<void>(connection_->Rollback());
                return inserted;
            }

            return connection_->Commit().transform_error([](const Error<ErrorType>& error) {
                return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
            });
        });
}

evget::Result<void> evget::DatabaseStorage::InsertEntries(const Data& events) {
    std::optional<std::unique_ptr<Query>> insert_key{};
    std::optional<std::unique_ptr<Query>> insert_key_modifier{};
    std::optional<std::unique_ptr<Query>> insert_mouse_move{};
    std::optional<std::unique_ptr<Query>> insert_mouse_move_modifier{};
    std::optional<std::unique_ptr<Query>> insert_mouse_click{};
    std::optional<std::unique_ptr<Query>> insert_mouse_click_modifier{};
    std::optional<std::unique_ptr<Query>> insert_mouse_scroll{};
    std::optional<std::unique_ptr<Query>> insert_mouse_scroll_modifier{};

    std::size_t dropped = 0;
    std::optional<std::string_view> dropped_field{};
    for (const auto& entry : events.Entries()) {
        if (entry.Data().empty()) {
            continue;
        }

        if (auto absent = AbsentNotNullField(entry)) {
            ++dropped;
            if (!dropped_field.has_value()) {
                dropped_field = absent;
            }
            continue;
        }

        Result<void> result;
        switch (entry.Type()) {
            case EntryType::kKey:
                result = InsertEvents(
                    entry,
                    insert_key,
                    insert_key_modifier,
                    detail::insert_key,
                    detail::insert_key_modifier
                );
                break;
            case EntryType::kMouseClick:
                result = InsertEvents(
                    entry,
                    insert_mouse_click,
                    insert_mouse_click_modifier,
                    detail::insert_mouse_click,
                    detail::insert_mouse_click_modifier
                );
                break;
            case EntryType::kMouseMove:
                result = InsertEvents(
                    entry,
                    insert_mouse_move,
                    insert_mouse_move_modifier,
                    detail::insert_mouse_move,
                    detail::insert_mouse_move_modifier
                );
                break;
            case EntryType::kMouseScroll:
                result = InsertEvents(
                    entry,
                    insert_mouse_scroll,
                    insert_mouse_scroll_modifier,
                    detail::insert_mouse_scroll,
                    detail::insert_mouse_scroll_modifier
                );
                break;
        }

        if (!result.has_value()) {
            return result;
        }
    }

    if (dropped_field.has_value()) {
        spdlog::error("dropped {} events missing a required value, the first missing {}", dropped, *dropped_field);
    }
    return {};
}

evget::Result<void> evget::DatabaseStorage::Init() {
    auto result = EnsureConnected().and_then([this] { return ApplyPragmas(); }).and_then([this] {
        auto migrations = std::vector{Migration{
            .version = 1,
            .description = "initialize database tables",
            .sql = detail::initialize,
            .exec = true,
        }};
        auto apply_migrations = Migrate{*this->connection_, migrations};

        return apply_migrations.ApplyMigrations().transform_error([](const Error<ErrorType>& error) {
            return Error{.error_type = ErrorType::kDatabaseError, .message = error.message};
        });
    });

    return result;
}

evget::Result<void> evget::DatabaseStorage::InsertEvents(
    const Entry& entry,
    std::optional<std::unique_ptr<Query>>& insert_statement,
    std::optional<std::unique_ptr<Query>>& insert_modifier_statement,
    std::string insert_query,
    std::string insert_modifier_query
) {
    SetOptionalStatement(insert_statement, std::move(insert_query));
    SetOptionalStatement(insert_modifier_statement, std::move(insert_modifier_query));

    auto entry_uuid = to_string(generator_());

    // Optional is set in previous lines.
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    return BindValues(*insert_statement, entry.Data(), entry_uuid)
        .and_then([this, &insert_modifier_statement, &entry, &entry_uuid] {
            return BindValuesModifier(*insert_modifier_statement, entry.Modifiers(), entry_uuid);
        });
    // NOLINTEND(bugprone-unchecked-optional-access)
}

void evget::DatabaseStorage::SetOptionalStatement(
    std::optional<std::unique_ptr<Query>>& query,
    std::string query_string
) const {
    if (!query.has_value()) {
        query = {connection_->BuildQuery(std::move(query_string))};
    }
}

evget::Result<void> evget::DatabaseStorage::BindValuesModifier(
    std::unique_ptr<Query>& query,
    const std::vector<std::string>& modifiers,
    const std::string& entry_uuid
) {
    for (const auto& modifier : modifiers) {
        auto modifier_uuid = boost::uuids::to_string(generator_());
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
