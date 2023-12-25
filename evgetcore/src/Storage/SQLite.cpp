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

#include "evgetcore/Storage/SQLite.h"
#include "schema/initialize.h"
#include "queries/insert_key.h"
#include "queries/insert_key_modifier.h"
#include "queries/insert_mouse_move.h"
#include "queries/insert_mouse_move_modifier.h"
#include "queries/insert_mouse_click.h"
#include "queries/insert_mouse_click_modifier.h"
#include "queries/insert_mouse_scroll.h"
#include "queries/insert_mouse_scroll_modifier.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <spdlog/spdlog.h>
#include "evgetcore/Storage/Error.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include <utility>

EvgetCore::Storage::SQLite::SQLite(std::string database)
    : database{std::move(database)} {
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::SQLite::store(Event::Data event) {
    try {
        ::SQLite::Database database{this->database, ::SQLite::OPEN_READWRITE};

        ::SQLite::Transaction transaction{database};

        std::optional<::SQLite::Statement> insertKey{};
        std::optional<::SQLite::Statement> insertKeyModifier{};
        std::optional<::SQLite::Statement> insertMouseMove{};
        std::optional<::SQLite::Statement> insertMouseMoveModifier{};
        std::optional<::SQLite::Statement> insertMouseClick{};
        std::optional<::SQLite::Statement> insertMouseClickModifier{};
        std::optional<::SQLite::Statement> insertMouseScroll{};
        std::optional<::SQLite::Statement> insertMouseScrollModifier{};

        for (auto entry : event.entries()) {
            if (entry.data().empty()) {
                continue;
            }

            std::string entryUuid;
            switch (entry.type()) {
                case Event::EntryType::Key:
                    insertEvents(
                        database,
                        entry,
                        insertKey,
                        insertKeyModifier,
                        Database::detail::insert_key,
                        Database::detail::insert_key_modifier
                    );
                    break;
                case Event::EntryType::MouseClick:
                    insertEvents(
                        database,
                        entry,
                        insertMouseClick,
                        insertMouseClickModifier,
                        Database::detail::insert_mouse_click,
                        Database::detail::insert_mouse_click_modifier
                    );
                    break;
                case Event::EntryType::MouseMove:
                    insertEvents(
                        database,
                        entry,
                        insertMouseMove,
                        insertMouseMoveModifier,
                        Database::detail::insert_mouse_move,
                        Database::detail::insert_mouse_move_modifier
                    );
                    break;
                case Event::EntryType::MouseScroll:
                    insertEvents(
                        database,
                        entry,
                        insertMouseScroll,
                        insertMouseClickModifier,
                        Database::detail::insert_mouse_scroll,
                        Database::detail::insert_mouse_scroll_modifier
                    );
                    break;
            }
        }

        transaction.commit();

        return Result<void>{};
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error inserting event data: {}", what);
        return Err{{.errorType = ErrorType::SQLiteError, .message = what}};
    }
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::SQLite::init() {
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

void EvgetCore::Storage::SQLite::insertEvents(
    ::SQLite::Database& database,
    const Event::Entry& entry,
    std::optional<::SQLite::Statement>& insertStatement,
    std::optional<::SQLite::Statement>& insertModifierStatement,
    const char* insertQuery,
    const char* insertModifierQuery
) {
    setOptionalStatement(database, insertStatement, insertQuery);
    setOptionalStatement(database, insertModifierStatement, insertModifierQuery);

    auto entryUuid = bindValues(*insertStatement, entry.data());
    bindValuesModifier(*insertModifierStatement, entry.modifiers(), entryUuid);
}

void EvgetCore::Storage::SQLite::setOptionalStatement(::SQLite::Database& database,
    std::optional<::SQLite::Statement>& statement,
    const char* query) {
    if (!statement.has_value()) {
        statement = {database, query};
    }
}

std::string EvgetCore::Storage::SQLite::bindValues(::SQLite::Statement& statement, std::vector<std::string> data) {
    auto entryUuid = to_string(uuids::random_generator()());

    statement.bind(1, entryUuid);
    for (auto i = 0; i < data.size(); i++) {
        statement.bind(i + 2, data[i]);
    }

    statement.exec();
    statement.reset();

    return entryUuid;
}

void EvgetCore::Storage::SQLite::bindValuesModifier(::SQLite::Statement& statement,
    std::vector<std::string> modifiers,
    std::string entryUuid) {
    for (const auto& modifier : modifiers) {
        auto modifierUuid = to_string(uuids::random_generator()());
        statement.bind(1, modifierUuid);
        statement.bind(2, entryUuid);
        statement.bind(3, modifier);

        statement.exec();
        statement.reset();
    }
}
