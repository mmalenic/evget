#include "evget/database/migrate.h"

#include <mbedtls/sha512.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <format>
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
        auto version = query->AsInt(0);
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

evget::Result<void> evget::Migrate::ApplyMigration(const Migration& migration, const std::string& checksum) const {
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
    constexpr std::size_t kSha512DigestBytes = 64;
    std::array<unsigned char, kSha512DigestBytes> digest{};
    const auto* input = std::bit_cast<const unsigned char*>(migration.sql.c_str());
    mbedtls_sha512(input, migration.sql.length(), digest.data(), 0);

    std::string hex{};
    hex.reserve(kSha512DigestBytes * 2);
    for (const unsigned char byte : digest) {
        hex += std::format("{:02x}", byte);
    }
    return hex;
}

evget::Result<void> evget::Migrate::ApplyMigrations() {
    return this->connection_.get()
        .Transaction()
        .and_then([this] { return this->CreateMigrationsTable(); })
        .and_then([this] { return this->GetAppliedMigrations(); })
        .and_then([this](const std::vector<AppliedMigration>& applied) {
            for (std::size_t i = 0; i < migrations_.size(); i++) {
                auto migration = migrations_.at(i);
                auto checksum = Checksum(migration);

                if (i < applied.size()) {
                    const auto& applied_migration = applied.at(i);
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

                spdlog::info("applied migration version {} with checksum: {}", migration.version, checksum);
            }

            return evget::Result<void>{};
        })
        .and_then([this] { return this->connection_.get().Commit(); })
        .or_else([this](const Error<ErrorType>& err) {
            return this->connection_.get().Rollback().and_then([&err] { return evget::Result<void>{Err{err}}; });
        });
}
