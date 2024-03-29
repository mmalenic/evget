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

    return query->nextWhile().and_then([&query] { return query->reset(); });
}

Database::Result<std::vector<Database::AppliedMigration>> Database::Migrate::getAppliedMigrations() {
    auto query = this->connection.get().buildQuery(
        "select version, checksum from _migrations order by version;"
    );

    std::vector<AppliedMigration> migrations{};
    auto next = query->next();
    while (next.has_value() && *next) {
        auto version = query->asBool(0);
        auto checksum = query->asString(1);

        if (!version.has_value() || !checksum.has_value()) {
            return Err{{.errorType = ErrorType::MigrateError, .message = "missing version or checksum"}};
        }

        migrations.emplace_back(
            *version,
            *checksum
        );

        next = query->next();
    }

    return next.and_then([&query, &migrations](bool _) {
        return query->reset()
            .and_then([&migrations] {
                return Database::Result<std::vector<AppliedMigration>>{migrations};
            });
    });
}

Database::Result<void> Database::Migrate::applyMigration(const Migration& migration, const std::string& checksum) {
    return applyMigrationSql(migration).and_then([this, &migration, &checksum] {
        auto query = this->connection.get().buildQuery(
            "insert into _migrations (version, description, checksum)"
            "values ($1, $2, $3);"
        );

        query->bindInt(0, migration.version);
        query->bindChars(1, migration.description.c_str());
        query->bindChars(2, checksum.c_str());

        auto next = query->nextWhile();

        return next.and_then([&query] { return query->reset(); });
    });
}

Database::Result<void> Database::Migrate::applyMigrationSql(const Migration& migration) {
    auto query = this->connection.get().buildQuery(migration.sql.c_str());

    if (migration.exec) {
        return query->exec();
    }

    return query->nextWhile().and_then([&query] { return query->reset(); });
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
    return this->connection.get().transaction().and_then([this] {
        return this->createMigrationsTable();
    }).and_then([this] {
        return this->getAppliedMigrations();
    }).and_then([this](const std::vector<AppliedMigration>& applied) {
        for (auto i = 0; i < migrations.size(); i++) {
            auto migration = migrations[i];
            auto checksum = this->checksum(migration);

            if (i < applied.size()) {
                auto appliedMigration = applied[i];
                if (appliedMigration.version != migration.version || appliedMigration.checksum != checksum) {
                    return Database::Result<void>{Err{{.errorType = ErrorType::MigrateError, .message = "applied migrations do not match current migrations"}}};
                }

                continue;
            }

            auto migrationResult = applyMigration(migration, checksum);
            if (!migrationResult.has_value()) {
                return migrationResult;
            }
        }

        return Database::Result<void>{};
    }).and_then([this] {
        return this->connection.get().commit();
    }).or_else([this](Util::Error<ErrorType> err) {
        return this->connection.get().rollback().and_then([&err] {
            return Database::Result<void>{Err{err}};
        });
    });
}





