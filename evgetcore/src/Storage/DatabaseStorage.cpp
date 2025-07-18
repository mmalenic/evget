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

#include "evgetcore/Storage/DatabaseStorage.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "evgetcore/Error.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Entry.h"
#include "evgetcore/database/Connection.h"
#include "evgetcore/database/Migrate.h"
#include "evgetcore/database/Query.h"
#include "queries/insert_key.h"
#include "queries/insert_key_modifier.h"
#include "queries/insert_mouse_click.h"
#include "queries/insert_mouse_click_modifier.h"
#include "queries/insert_mouse_move.h"
#include "queries/insert_mouse_move_modifier.h"
#include "queries/insert_mouse_scroll.h"
#include "queries/insert_mouse_scroll_modifier.h"
#include "schema/initialize.h"

namespace uuids = boost::uuids;

EvgetCore::Storage::DatabaseStorage::DatabaseStorage(
    std::unique_ptr<::EvgetCore::Connection> connection,
    std::string database
)
    : connection{std::move(connection)}, database{std::move(database)} {}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseStorage::store(Event::Data event) {
    return connection->connect(database, ::EvgetCore::ConnectOptions::READ_WRITE_CREATE)
        .and_then([this] { return connection->transaction(); })
        .transform_error([](const Error<::EvgetCore::ErrorType>& error) {
            return Error{.errorType = ErrorType::DatabaseError, .message = error.message};
        })
        .and_then([this, &event] {
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertKey{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertKeyModifier{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseMove{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseMoveModifier{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseClick{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseClickModifier{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseScroll{};
            std::optional<std::unique_ptr<::EvgetCore::Query>> insertMouseScrollModifier{};

            for (const auto& entry : event.entries()) {
                if (entry.data().empty()) {
                    continue;
                }

                Result<void> result;
                switch (entry.type()) {
                    case Event::EntryType::Key:
                        result = insertEvents(
                            entry,
                            insertKey,
                            insertKeyModifier,
                            Database::detail::insert_key,
                            Database::detail::insert_key_modifier
                        );
                        break;
                    case Event::EntryType::MouseClick:
                        result = insertEvents(
                            entry,
                            insertMouseClick,
                            insertMouseClickModifier,
                            Database::detail::insert_mouse_click,
                            Database::detail::insert_mouse_click_modifier
                        );
                        break;
                    case Event::EntryType::MouseMove:
                        result = insertEvents(
                            entry,
                            insertMouseMove,
                            insertMouseMoveModifier,
                            Database::detail::insert_mouse_move,
                            Database::detail::insert_mouse_move_modifier
                        );
                        break;
                    case Event::EntryType::MouseScroll:
                        result = insertEvents(
                            entry,
                            insertMouseScroll,
                            insertMouseScrollModifier,
                            Database::detail::insert_mouse_scroll,
                            Database::detail::insert_mouse_scroll_modifier
                        );
                        break;
                }

                if (!result.has_value()) {
                    return result;
                }
            }

            return this->connection->commit().transform_error([](const Error<::EvgetCore::ErrorType>& error) {
                return Error{.errorType = ErrorType::DatabaseError, .message = error.message};
            });
        });
}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseStorage::init() {
    auto result = connection->connect(database, ::EvgetCore::ConnectOptions::READ_WRITE_CREATE)
                      .transform_error([](const Error<::EvgetCore::ErrorType>& error) {
                          return Error{.errorType = ErrorType::DatabaseError, .message = error.message};
                      })
                      .and_then([this] {
                          auto migrations = std::vector{::EvgetCore::Migration{
                              .version = 1,
                              .description = "initialize database tables",
                              .sql = Database::detail::initialize,
                              .exec = true,
                          }};
                          auto migrate = ::EvgetCore::Migrate{*this->connection, migrations};

                          return migrate.migrate().transform_error([](const Error<::EvgetCore::ErrorType>& error) {
                              return Error{.errorType = ErrorType::DatabaseError, .message = error.message};
                          });
                      });

    return result;
}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseStorage::insertEvents(
    const Event::Entry& entry,
    std::optional<std::unique_ptr<::EvgetCore::Query>>& insertStatement,
    std::optional<std::unique_ptr<::EvgetCore::Query>>& insertModifierStatement,
    std::string insertQuery,
    std::string insertModifierQuery
) {
    setOptionalStatement(insertStatement, std::move(insertQuery));
    setOptionalStatement(insertModifierStatement, std::move(insertModifierQuery));

    auto entryUuid = to_string(uuids::random_generator()());

    // Optional is set in previous lines.
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    return bindValues(*insertStatement, entry.data(), entryUuid)
        .and_then([this, &insertModifierStatement, &entry, &entryUuid] {
            return bindValuesModifier(*insertModifierStatement, entry.modifiers(), entryUuid);
        });
    // NOLINTEND(bugprone-unchecked-optional-access)
}

void EvgetCore::Storage::DatabaseStorage::setOptionalStatement(
    std::optional<std::unique_ptr<::EvgetCore::Query>>& query,
    std::string queryString
) const {
    if (!query.has_value()) {
        query = {connection->buildQuery(std::move(queryString))};
    }
}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseStorage::bindValues(
    std::unique_ptr<::EvgetCore::Query>& query,
    const std::vector<std::string>& data,
    const std::string& entryUuid
) {
    query->bindChars(0, entryUuid.c_str());
    for (auto i = 0; i < data.size(); i++) {
        query->bindChars(i + 1, data[i].c_str());
    }

    return query->nextWhile().and_then(
                                 [&query] { return (*query).reset(); }
    ).transform_error([](const Error<::EvgetCore::ErrorType>& error) {
        return Error{.errorType = ErrorType::DatabaseError, .message = error.message};
    });
}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseStorage::bindValuesModifier(
    std::unique_ptr<::EvgetCore::Query>& query,
    const std::vector<std::string>& modifiers,
    const std::string& entryUuid
) {
    for (const auto& modifier : modifiers) {
        auto modifierUuid = to_string(uuids::random_generator()());
        query->bindChars(0, modifierUuid.c_str());
        query->bindChars(1, entryUuid.c_str());
        query->bindChars(2, modifier.c_str());

        auto result = query->nextWhile().and_then([&query] { return (*query).reset(); });
        if (!result.has_value()) {
            return Err{{.errorType = ErrorType::DatabaseError, .message = result.error().message}};
        }
    }

    return {};
}
