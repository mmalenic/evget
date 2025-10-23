/**
 * \file query.h
 * \brief SQLite database query implementation.
 */

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
/**
 * \brief SQLite implementation of the `Query` interface, providing SQLite-specific query functionality.
 */
class SQLiteQuery : public Query {
public:
    /**
     * \brief Construct an SQLite query.
     * \param connection SQLite connection to use
     * \param query SQL query string
     */
    SQLiteQuery(SQLiteConnection& connection, std::string query);

    /**
     * \brief Bind an integer value to a parameter position.
     * \param position parameter position (0-based)
     * \param value integer value to bind
     */
    void BindInt(int position, int value) override;
    
    /**
     * \brief Bind a double value to a parameter position.
     * \param position parameter position (0-based)
     * \param value double value to bind
     */
    void BindDouble(int position, double value) override;
    
    /**
     * \brief Bind a character array to a parameter position.
     * \param position parameter position (0-based)
     * \param value character array to bind
     */
    void BindChars(int position, const char* value) override;
    
    /**
     * \brief Bind a boolean value to a parameter position.
     * \param position parameter position (0-based)
     * \param value boolean value to bind
     */
    void BindBool(int position, bool value) override;
    
    /**
     * \brief Reset the query to its initial state.
     * \return result indicating success or failure
     */
    Result<void> Reset() override;
    
    /**
     * \brief Execute the next step of the query.
     * \return result with boolean indicating if more steps are available
     */
    Result<bool> Next() override;
    
    /**
     * \brief Execute all remaining steps of the query.
     * \return result indicating success or failure
     */
    Result<void> NextWhile() override;
    
    /**
     * \brief Execute the query directly.
     * \return result indicating success or failure
     */
    Result<void> Exec() override;
    
    /**
     * \brief Get a column value as a boolean.
     * \param pos Column position (0-based)
     * \return result with the boolean value
     */
    Result<bool> AsBool(int pos) override;
    
    /**
     * \brief Get a column value as a double.
     * \param pos column position (0-based)
     * \return result with the double value
     */
    Result<double> AsDouble(int pos) override;
    
    /**
     * \brief Get a column value as an integer.
     * \param pos column position (0-based)
     * \return result with the integer value
     */
    Result<int> AsInt(int pos) override;
    
    /**
     * \brief Get a column value as a string.
     * \param pos column position (0-based)
     * \return result with the string value
     */
    Result<std::string> AsString(int pos) override;

private:
    static Err AsError(const std::exception& error);
    static Err StatementError();

    std::reference_wrapper<SQLiteConnection> connection_;
    std::map<int, std::variant<int, double, const char*, bool>> binds_;
    std::optional<::SQLite::Statement> statement_;
    std::string query_;
};

} // namespace evget

#endif
