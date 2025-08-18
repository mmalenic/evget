#include "evget/database/sqlite/connection.h"

#include <SQLiteCpp/Database.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "evget/database/connection.h"
#include "evget/database/query.h"
#include "evget/database/sqlite/query.h"
#include "evget/error.h"

evget::Result<void> evget::SQLiteConnection::Connect(std::string database, evget::ConnectOptions options) {
    try {
        switch (options) {
            case evget::ConnectOptions::kReadOnly:
                this->database_ = {database, ::SQLite::OPEN_READONLY};
                break;
            case evget::ConnectOptions::kReadWrite:
                this->database_ = {database, ::SQLite::OPEN_READWRITE};
                break;
            case evget::ConnectOptions::kReadWriteCreate:
                // NOLINTBEGIN(hicpp-signed-bitwise)
                this->database_ = {database, ::SQLite::OPEN_READWRITE | ::SQLite::OPEN_CREATE};
                // NOLINTEND(hicpp-signed-bitwise)
                break;
        }

        spdlog::info("connected to SQLite database: {}", database);
        return {};
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error connecting to SQLite database: {}", what);
        return ConnectError(what);
    }
}

std::optional<std::reference_wrapper<::SQLite::Database>> evget::SQLiteConnection::Database() {
    return database_.transform([](::SQLite::Database& database) { return std::ref(database); });
}

evget::Result<void> evget::SQLiteConnection::Commit() {
    if (!transaction_.has_value()) {
        return ConnectError("transaction not started");
    }

    try {
        transaction_->commit();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error committing transaction: {}", what);
        return ConnectError(what);
    }

    return {};
}

evget::Result<void> evget::SQLiteConnection::Transaction() {
    if (!this->database_.has_value()) {
        return ConnectError("no database connected");
    }

    try {
        this->transaction_.emplace(*this->database_);
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error creating transaction: {}", what);
        return ConnectError(what);
    }

    return {};
}

std::unique_ptr<evget::Query> evget::SQLiteConnection::BuildQuery(std::string query) {
    return std::make_unique<evget::SQLiteQuery>(*this, std::move(query));
}

evget::Err evget::SQLiteConnection::ConnectError(const char* message) {
    return Err{Error{.error_type = ErrorType::kDatabaseError, .message = message}};
}

evget::Result<void> evget::SQLiteConnection::Rollback() {
    if (!transaction_.has_value()) {
        return ConnectError("transaction not started");
    }

    try {
        transaction_->rollback();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error rolling back transaction: {}", what);
        return ConnectError(what);
    }

    return {};
}
