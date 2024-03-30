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

#include <SQLiteCpp/SQLiteCpp.h>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <spdlog/spdlog.h>

#include <utility>

#include "evgetcore/Storage/DatabaseStorage.h"
#include "evgetcore/Storage/Error.h"
#include "queries/insert_key.h"
#include "queries/insert_key_modifier.h"
#include "queries/insert_mouse_click.h"
#include "queries/insert_mouse_click_modifier.h"
#include "queries/insert_mouse_move.h"
#include "queries/insert_mouse_move_modifier.h"
#include "queries/insert_mouse_scroll.h"
#include "queries/insert_mouse_scroll_modifier.h"
#include "schema/initialize.h"

EvgetCore::Storage::DatabaseStorage::DatabaseStorage(std::reference_wrapper<::Database::Connection> connection, std::string database)
    : connection{connection}, database{std::move(database)} {
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseStorage::store(Event::Data event) {
    return connection.get().connect(database, ::Database::ConnectOptions::READ_WRITE_CREATE).and_then([this] {
        return connection.get().transaction();
    }).transform_error([](Util::Error<::Database::ErrorType> error) {
        return Util::Error{.errorType = ErrorType::DatabaseError, .message = error.message};
    }).and_then([this, &event] {
        std::optional<std::unique_ptr<::Database::Query>>insertKey{};
        std::optional<std::unique_ptr<::Database::Query>> insertKeyModifier{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseMove{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseMoveModifier{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseClick{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseClickModifier{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseScroll{};
        std::optional<std::unique_ptr<::Database::Query>> insertMouseScrollModifier{};

        for (auto entry : event.entries()) {
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
                        insertMouseClickModifier,
                        Database::detail::insert_mouse_scroll,
                        Database::detail::insert_mouse_scroll_modifier
                    );
                    break;
            }

            if (!result.has_value()) {
                return result;
            }
        }

        return Result<void>{};
    });
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseStorage::init() {
    try {
        ::SQLite::Database db{this->database, ::SQLite::OPEN_READWRITE | ::SQLite::OPEN_CREATE};

        ::SQLite::Transaction transaction{db};

        db.exec(Database::detail::initialize);

        transaction.commit();

        spdlog::info("initialized sqlite database at: {}", this->database);
        return {};
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error initializing SQLite database: {}", what);
        return Err{{.errorType = ErrorType::SQLiteError, .message = what}};
    }
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseStorage::insertEvents(
    const Event::Entry& entry,
    std::optional<std::unique_ptr<::Database::Query>>& insertStatement,
    std::optional<std::unique_ptr<::Database::Query>>& insertModifierStatement,
    std::string insertQuery,
    std::string insertModifierQuery
) {
    setOptionalStatement(insertStatement, std::move(insertQuery));
    setOptionalStatement(insertModifierStatement, std::move(insertModifierQuery));

    auto entryUuid = to_string(uuids::random_generator()());

    return bindValues(*insertStatement, entry.data(), entryUuid).and_then([this, &insertModifierStatement, &entry, &entryUuid] {
        return bindValuesModifier(*insertModifierStatement, entry.modifiers(), entryUuid);
    });
}

void EvgetCore::Storage::DatabaseStorage::setOptionalStatement(
    std::optional<std::unique_ptr<::Database::Query>>& query,
    std::string queryString
) {
    if (!query.has_value()) {
        query = {connection.get().buildQuery(std::move(queryString))};
    }
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseStorage::bindValues(
    std::unique_ptr<::Database::Query>& query,
    std::vector<std::string> data,
    std::string entryUuid) {
    query->bindChars(0, entryUuid.c_str());
    for (auto i = 0; i < data.size(); i++) {
        query->bindChars(i + 1, data[i].c_str());
    }

    return query->exec().and_then([&query] {
        return query->reset();
    }).transform_error([](Util::Error<::Database::ErrorType> error) {
        return Util::Error{.errorType = ErrorType::DatabaseError, .message = error.message};
    });
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseStorage::bindValuesModifier(std::unique_ptr<::Database::Query>& query,
    std::vector<std::string> modifiers,
    std::string entryUuid) {
    for (const auto& modifier : modifiers) {
        auto modifierUuid = to_string(uuids::random_generator()());
        query->bindChars(0, modifierUuid.c_str());
        query->bindChars(1, entryUuid.c_str());
        query->bindChars(2, modifier.c_str());

        auto result = query->exec().and_then([&query] {
            return query->reset();
        });
        if (!result.has_value()) {
            return Err{{.errorType = ErrorType::DatabaseError, .message = result.error().message}};
        }
    }

    return {};
}
