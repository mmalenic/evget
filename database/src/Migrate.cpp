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

#include "database/Migrate.h"

#include <filesystem>
#include <ranges>

#include "cryptopp/hex.h"
#include "cryptopp/sha3.h"

Database::Migrate::Migrate(Connection& connection, std::vector<Migration> migrations) : connection{connection} {
    std::ranges::sort(migrations.begin(), migrations.end(), [](const Migration& a, const Migration& b) {
        return a.version < b.version;
    });

    this->migrations = migrations;
}

Database::Result<void> Database::Migrate::createMigrationsTable() {
    auto query = this->connection.get().buildQuery(
        "create table if not exists _migrations ("
            "version integer primary key,"
            "description text not null,"
            "installed_on timestamp not null default current_timestamp,"
            "checksum text not null"
        ");"
    );

    auto next = query->nextWhile();
    if (!next.has_value()) {
        return Err{next.error()};
    }

    auto reset = query->reset();
    if (!reset.has_value()) {
        return Err{reset.error()};
    }

    return {};
}

Database::Result<std::vector<Database::AppliedMigration>> Database::Migrate::getAppliedMigrations() {
    auto query = this->connection.get().buildQuery(
        "select version, checksum from _migrations order by version;"
    );

    std::vector<Migration> migrations{};
    auto next = query->next();
    while (next.has_value() && *next) {
        auto version = query->asBool(0);
        if (!version.has_value()) {
            return Err{{.errorType = ErrorType::MigrateError, .message = version.error().message}};
        }

        auto checksum = query->asString(1);
        if (!checksum.has_value()) {
            return Err{{.errorType = ErrorType::MigrateError, .message = checksum.error().message}};
        }

        migrations.emplace_back(
            *version,
            *checksum
        );

        next = query->next();
    }

    if (!next.has_value()) {
        return Err{{.errorType = ErrorType::MigrateError, .message = next.error().message}};
    }

    auto reset = query->reset();
    if (!reset.has_value()) {
        return Err{reset.error()};
    }

    return {};
}

Database::Result<void> Database::Migrate::applyMigration(const Migration& migration, const std::string& checksum) {
    auto applyMigration = applyMigrationSql(migration);
    if (!applyMigration.has_value()) {
        return Err{applyMigration.error()};
    }

    auto migrationQuery = this->connection.get().buildQuery(
        "insert into _migrations (version, description, checksum)"
        "values ($1, $2, $3);"
    );

    migrationQuery->bindInt(0, migration.version);
    migrationQuery->bindChars(1, migration.description.c_str());
    migrationQuery->bindChars(2, checksum.c_str());

    auto migrationNext = migrationQuery->nextWhile();
    if (!migrationNext.has_value()) {
        return Err{migrationNext.error()};
    }

    auto migrationReset = migrationQuery->reset();
    if (!migrationReset.has_value()) {
        return Err{migrationReset.error()};
    }

    return {};
}

Database::Result<void> Database::Migrate::applyMigrationSql(const Migration& migration) {
    auto query = this->connection.get().buildQuery(migration.sql.c_str());

    auto next = query->nextWhile();
    if (!next.has_value()) {
        return Err{next.error()};
    }

    auto reset = query->reset();
    if (!reset.has_value()) {
        return Err{reset.error()};
    }

    return {};
}


std::string Database::Migrate::checksum(const Migration& migration) {
    CryptoPP::SHA3_512 sha3{};
    CryptoPP::byte digest[CryptoPP::SHA3_512::DIGESTSIZE];

    sha3.CalculateDigest(
        digest,
        reinterpret_cast<const CryptoPP::byte*>(migration.sql.c_str()),
        migration.sql.length()
    );

    CryptoPP::HexEncoder encoder;
    std::string output;

    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();

    CryptoPP::word64 size = encoder.MaxRetrievable();
    if (size) {
        output.resize(size);
        encoder.Get(reinterpret_cast<CryptoPP::byte *>(&output[0]), output.size());
    }

    return output;
}

Database::Result<void> Database::Migrate::migrate() {
    auto transactionResult = this->connection.get().transaction();
    if (!rollbackOnError(transactionResult).has_value()) {
        return Err{transactionResult.error()};
    }

    auto createTableResult = this->createMigrationsTable();
    if (!rollbackOnError(createTableResult).has_value()) {
        return Err{createTableResult.error()};
    }

    auto applied = this->getAppliedMigrations();
    if (!rollbackOnError(applied).has_value()) {
        return Err{applied.error()};
    }

    for (auto i = 0; i < migrations.size(); i++) {
        auto migration = migrations[i];
        auto checksum = this->checksum(migration);

        if (i < applied->size()) {
            auto appliedMigration = (*applied)[i];
            if (appliedMigration.version != migration.version || appliedMigration.checksum != checksum) {
                return rollbackOnError<void>(Err{{.errorType = ErrorType::MigrateError, .message = "applied migrations do not match current migrations"}});
            }

            continue;
        }

        auto migrationResult = applyMigration(migration, checksum);
        if (!rollbackOnError(migrationResult).has_value()) {
            return Err{migrationResult.error()};
        }
    }

    auto commitResult = this->connection.get().commit();
    if (!rollbackOnError(commitResult).has_value()) {
        return Err{commitResult.error()};
    }

    return {};
}





