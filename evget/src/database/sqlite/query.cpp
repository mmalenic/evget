#include "evget/database/sqlite/query.h"

#include <spdlog/spdlog.h>

#include <exception>
#include <string>
#include <utility>
#include <variant>

#include "evget/database/sqlite/connection.h"
#include "evget/error.h"

evget::SQLiteQuery::SQLiteQuery(SQLiteConnection& connection, std::string query)
    : connection_{connection}, query_{std::move(query)} {}

void evget::SQLiteQuery::BindInt(int position, int value) {
    binds_[position] = value;
}

void evget::SQLiteQuery::BindBool(int position, bool value) {
    binds_[position] = value;
}

void evget::SQLiteQuery::BindChars(int position, const char* value) {
    binds_[position] = value;
}

void evget::SQLiteQuery::BindDouble(int position, double value) {
    binds_[position] = value;
}

evget::Result<void> evget::SQLiteQuery::Reset() {
    this->binds_.clear();

    try {
        if (statement_.has_value()) {
            statement_->reset();
            statement_->clearBindings();
        }
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error resetting statements: {}", what);
        return Err{{.error_type = ErrorType::kDatabaseError, .message = what}};
    }

    return {};
}

evget::Result<bool> evget::SQLiteQuery::Next() {
    try {
        auto database = connection_.get().Database();
        if (!database.has_value()) {
            return Err{{.error_type = ErrorType::kDatabaseError, .message = "database not set"}};
        }
        if (!statement_.has_value()) {
            statement_.emplace(database->get(), query_);
        }

        auto& statement = statement_.value();
        for (auto& [position, bind_value] : this->binds_) {
            std::visit([&statement, pos = position](auto&& inner) { statement.bind(pos + 1, inner); }, bind_value);
        }
        this->binds_.clear();

        return statement.executeStep();
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.error_type = ErrorType::kDatabaseError, .message = what}};
    }
}

evget::Result<void> evget::SQLiteQuery::NextWhile() {
    auto next = this->Next();
    while (next.has_value() && *next) {
        next = this->Next();
    }

    if (!next.has_value()) {
        return Err{next.error()};
    }

    return {};
}

evget::Result<void> evget::SQLiteQuery::Exec() {
    try {
        auto database = connection_.get().Database();
        if (!database.has_value()) {
            return Err{{.error_type = ErrorType::kDatabaseError, .message = "database not set"}};
        }

        database->get().exec(query_);
        return {};
    } catch (std::exception& e) {
        const auto* what = e.what();
        spdlog::error("error building query: {}", what);
        return Err{{.error_type = ErrorType::kDatabaseError, .message = what}};
    }
}

evget::Result<bool> evget::SQLiteQuery::AsBool(int pos) {
    if (!this->statement_.has_value()) {
        return StatementError();
    }

    try {
        return this->statement_->getColumn(pos).getInt();
    } catch (std::exception& e) {
        return AsError(e);
    }
}

evget::Result<double> evget::SQLiteQuery::AsDouble(int pos) {
    if (!this->statement_.has_value()) {
        return StatementError();
    }

    try {
        return this->statement_->getColumn(pos);
    } catch (std::exception& e) {
        return AsError(e);
    }
}

evget::Result<int> evget::SQLiteQuery::AsInt(int pos) {
    if (!this->statement_.has_value()) {
        return StatementError();
    }

    try {
        return this->statement_->getColumn(pos);
    } catch (std::exception& e) {
        return AsError(e);
    }
}

evget::Result<std::string> evget::SQLiteQuery::AsString(int pos) {
    if (!this->statement_.has_value()) {
        return StatementError();
    }

    try {
        return this->statement_->getColumn(pos).getString();
    } catch (std::exception& e) {
        return AsError(e);
    }
}

evget::Err evget::SQLiteQuery::StatementError() {
    const auto* what = "statement has not been initialized";
    spdlog::error(what);
    return Err{{.error_type = ErrorType::kDatabaseError, .message = what}};
}

evget::Err evget::SQLiteQuery::AsError(const std::exception& error) {
    const auto* what = error.what();
    spdlog::error("error getting column value: {}", what);
    return Err{{.error_type = ErrorType::kDatabaseError, .message = what}};
}
