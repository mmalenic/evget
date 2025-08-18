#include "evget/database/migrate.h"

#include <openssl/sha.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <string>
#include <vector>

#include "evget/database/connection.h"
#include "evget/error.h"

evget::Migrate::Migrate(Connection& connection, const std::vector<Migration>& migrations)
    : connection_{connection}, migrations_{migrations} {
    std::ranges::sort(
        this->migrations_.begin(),
        this->migrations_.end(),
        [](const Migration& a_migrate, const Migration& b_migrate) { return a_migrate.version < b_migrate.version; }
    );
}

evget::Result<void> evget::Migrate::CreateMigrationsTable() const {
    auto query = this->connection_.get().BuildQuery(
        "create table if not exists _migrations ("
        "version integer primary key,"
        "description text not null,"
        "installed_on timestamp not null default current_timestamp,"
        "checksum text not null"
        ");"
    );

    return query->NextWhile().and_then([&query] { return (*query).Reset(); });
}

evget::Result<std::vector<evget::AppliedMigration>> evget::Migrate::GetAppliedMigrations() const {
    auto query = this->connection_.get().BuildQuery("select version, checksum from _migrations order by version;");

    std::vector<AppliedMigration> migrations{};
    auto next = query->Next();
    while (next.has_value() && *next) {
        auto version = query->AsBool(0);
        auto checksum = query->AsString(1);

        if (!version.has_value() || !checksum.has_value()) {
            return Err{Error{.error_type = ErrorType::kDatabaseError, .message = "missing version or checksum"}};
        }

        migrations.emplace_back(*version, *checksum);

        next = query->Next();
    }

    return next.and_then([&query, &migrations](bool) {
        return (*query).Reset().and_then([&migrations] {
            return evget::Result<std::vector<AppliedMigration>>{migrations};
        });
    });
}

evget::Result<void> evget::Migrate::ApplyMigration(const Migration& migration, const std::string& checksum) {
    return ApplyMigrationSql(migration).and_then([this, &migration, &checksum] {
        auto query = this->connection_.get().BuildQuery(
            "insert into _migrations (version, description, checksum)"
            "values ($1, $2, $3);"
        );

        query->BindInt(0, migration.version);
        query->BindChars(1, migration.description.c_str());
        query->BindChars(2, checksum.c_str());

        auto next = query->NextWhile();

        return next.and_then([&query] { return (*query).Reset(); });
    });
}

evget::Result<void> evget::Migrate::ApplyMigrationSql(const Migration& migration) const {
    auto query = this->connection_.get().BuildQuery(migration.sql);

    if (migration.exec) {
        return query->Exec();
    }

    return query->NextWhile().and_then([&query] { return (*query).Reset(); });
}

std::string evget::Migrate::Checksum(const Migration& migration) {
    // Reinterpret cast is unavoidable here to continue using the signed std::string variety.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* checksum =
        SHA512(reinterpret_cast<const unsigned char*>(migration.sql.c_str()), migration.sql.length(), nullptr);
    return std::string{reinterpret_cast<const char*>(checksum)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

evget::Result<void> evget::Migrate::ApplyMigrations() {
    return this->connection_.get()
        .Transaction()
        .and_then([this] { return this->CreateMigrationsTable(); })
        .and_then([this] { return this->GetAppliedMigrations(); })
        .and_then([this](const std::vector<evget::AppliedMigration>& applied) {
            for (auto i = 0; i < migrations_.size(); i++) {
                auto migration = migrations_[i];
                auto checksum = Checksum(migration);

                if (i < applied.size()) {
                    const auto& applied_migration = applied[i];
                    if (applied_migration.version != migration.version || applied_migration.checksum != checksum) {
                        return evget::Result<void>{Err{
                            {.error_type = ErrorType::kDatabaseError,
                             .message = "applied migrations do not match current migrations"}
                        }};
                    }

                    continue;
                }

                auto migration_result = ApplyMigration(migration, checksum);
                if (!migration_result.has_value()) {
                    return migration_result;
                }

                spdlog::info("applied migration with checksum: {}", migration.version, checksum);
            }

            return evget::Result<void>{};
        })
        .and_then([this] { return this->connection_.get().Commit(); })
        .or_else([this](const Error<ErrorType>& err) {
            return this->connection_.get().Rollback().and_then([&err] { return evget::Result<void>{Err{err}}; });
        });
}
