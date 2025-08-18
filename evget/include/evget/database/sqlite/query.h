#ifndef EVGET_DATABASE_SQLITE_QUERY_H
#define EVGET_DATABASE_SQLITE_QUERY_H

#include <SQLiteCpp/Statement.h>

#include <exception>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "evget/database/query.h"
#include "evget/database/sqlite/connection.h"
#include "evget/error.h"

namespace evget {
class SQLiteQuery : public Query {
public:
    SQLiteQuery(SQLiteConnection& connection, std::string query);

    void BindInt(int position, int value) override;
    void BindDouble(int position, double value) override;
    void BindChars(int position, const char* value) override;
    void BindBool(int position, bool value) override;
    Result<void> Reset() override;
    Result<bool> Next() override;
    Result<void> NextWhile() override;
    Result<void> Exec() override;
    Result<bool> AsBool(int pos) override;
    Result<double> AsDouble(int pos) override;
    Result<int> AsInt(int pos) override;
    Result<std::string> AsString(int pos) override;

private:
    static Err AsError(const std::exception& error);
    static Err StatementError();

    std::reference_wrapper<SQLiteConnection> connection_;
    std::map<int, std::variant<int, double, const char*, bool>> binds_;
    std::optional<::SQLite::Statement> statement_;
    std::string query_;
};

}  // namespace evget

#endif
