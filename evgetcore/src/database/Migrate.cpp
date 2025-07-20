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

#include "evgetcore/database/Migrate.h"

#include <cryptopp/config_int.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cryptopp/hex.h"
#include "cryptopp/sha3.h"
#include "evgetcore/Error.h"
#include "evgetcore/database/Connection.h"

EvgetCore::Migrate::Migrate(Connection& connection, std::vector<Migration> migrations)
    : connection{connection}, migrations{std::move(migrations)} {
    std::ranges::sort(
        this->migrations.begin(),
        this->migrations.end(),
        [](const Migration& a_migrate, const Migration& b_migrate) { return a_migrate.version < b_migrate.version; }
    );
}

EvgetCore::Result<void> EvgetCore::Migrate::createMigrationsTable() const {
    auto query = this->connection.get().buildQuery(
        "create table if not exists _migrations ("
        "version integer primary key,"
        "description text not null,"
        "installed_on timestamp not null default current_timestamp,"
        "checksum text not null"
        ");"
    );

    return query->nextWhile().and_then([&query] { return (*query).reset(); });
}

EvgetCore::Result<std::vector<EvgetCore::AppliedMigration>> EvgetCore::Migrate::getAppliedMigrations() const {
    auto query = this->connection.get().buildQuery("select version, checksum from _migrations order by version;");

    std::vector<AppliedMigration> migrations{};
    auto next = query->next();
    while (next.has_value() && *next) {
        auto version = query->asBool(0);
        auto checksum = query->asString(1);

        if (!version.has_value() || !checksum.has_value()) {
            return Err{Error{.errorType = ErrorType::DatabaseError, .message = "missing version or checksum"}};
        }

        migrations.emplace_back(*version, *checksum);

        next = query->next();
    }

    return next.and_then([&query, &migrations](bool) {
        return (*query).reset().and_then([&migrations] {
            return EvgetCore::Result<std::vector<AppliedMigration>>{migrations};
        });
    });
}

EvgetCore::Result<void> EvgetCore::Migrate::applyMigration(const Migration& migration, const std::string& checksum) {
    return applyMigrationSql(migration).and_then([this, &migration, &checksum] {
        auto query = this->connection.get().buildQuery(
            "insert into _migrations (version, description, checksum)"
            "values ($1, $2, $3);"
        );

        query->bindInt(0, migration.version);
        query->bindChars(1, migration.description.c_str());
        query->bindChars(2, checksum.c_str());

        auto next = query->nextWhile();

        return next.and_then([&query] { return (*query).reset(); });
    });
}

EvgetCore::Result<void> EvgetCore::Migrate::applyMigrationSql(const Migration& migration) const {
    auto query = this->connection.get().buildQuery(migration.sql);

    if (migration.exec) {
        return query->exec();
    }

    return query->nextWhile().and_then([&query] { return (*query).reset(); });
}

std::string EvgetCore::Migrate::checksum(const Migration& migration) {
    CryptoPP::SHA3_512 sha3{};
    std::array<CryptoPP::byte, CryptoPP::SHA3_512::DIGESTSIZE> digest{};

    std::vector<CryptoPP::byte> input{migration.sql.begin(), migration.sql.end()};
    sha3.CalculateDigest(digest.data(), input.data(), input.size());

    CryptoPP::HexEncoder encoder;

    encoder.Put(digest.data(), sizeof(digest));
    encoder.MessageEnd();

    const CryptoPP::word64 size = encoder.MaxRetrievable();
    std::vector<CryptoPP::byte> output;
    if (size != 0U) {
        output.resize(size);
        encoder.Get(output.data(), output.size());
    }

    return std::string{output.begin(), output.end()};
}

EvgetCore::Result<void> EvgetCore::Migrate::migrate() {
    return this->connection.get()
        .transaction()
        .and_then([this] { return this->createMigrationsTable(); })
        .and_then([this] { return this->getAppliedMigrations(); })
        .and_then([this](const std::vector<AppliedMigration>& applied) {
            for (auto i = 0; i < migrations.size(); i++) {
                auto migration = migrations[i];
                auto checksum = EvgetCore::Migrate::checksum(migration);

                if (i < applied.size()) {
                    const auto& appliedMigration = applied[i];
                    if (appliedMigration.version != migration.version || appliedMigration.checksum != checksum) {
                        return EvgetCore::Result<void>{Err{
                            {.errorType = ErrorType::DatabaseError,
                             .message = "applied migrations do not match current migrations"}
                        }};
                    }

                    continue;
                }

                auto migrationResult = applyMigration(migration, checksum);
                if (!migrationResult.has_value()) {
                    return migrationResult;
                }

                spdlog::info("applied migration with checksum: {}", migration.version, checksum);
            }

            return EvgetCore::Result<void>{};
        })
        .and_then([this] { return this->connection.get().commit(); })
        .or_else([this](Error<ErrorType> err) {
            return this->connection.get().rollback().and_then([&err] { return EvgetCore::Result<void>{Err{err}}; });
        });
}
