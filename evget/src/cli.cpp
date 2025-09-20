#include "evget/cli.h"

#include <CLI/CLI.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evget/database/sqlite/connection.h"
#include "evget/error.h"
#include "evget/storage/database_storage.h"
#include "evget/storage/json_storage.h"
#include "evget/storage/store.h"

const std::vector<std::string>& evget::Cli::Output() const {
    return this->output_;
}

std::expected<bool, int> evget::Cli::Parse(int argc, char** argv) {
    spdlog::cfg::load_env_levels();

    CLI::App app{"Show and store events from input devices.", "evget"};
    argv = app.ensure_utf8(argv);

    auto should_exit = false;
    app.add_flag_callback(
        "-v,--version",
        [&should_exit] {
            should_exit = true;
            std::cout << std::format("evlist {}\n\n{}\n{}\n", EVGET_VERSION, EVGET_LICENSE, EVGET_COPYRIGHT);
        },
        "Print version"
    );

    app.add_option(
           "-n,--store-n-events",
           store_n_events_,
           "Controls how many events to receive before outputting them to the store."
    )
        ->default_val(kDefaultNEvents);
    app.add_option(
           "-s,--store-after-seconds",
           store_after_,
           "Store events at least every interval specified with this option, even if fewer events than "
           "`--store-n-events` has been receieved."
    )
        ->default_val(kDefaultStoreAfter);

    app.add_option_function<std::string>(
           "-o,--output",
           [this](std::string value) {
               if (value == "-") {
                   spdlog::set_level(spdlog::level::off);
               }

               std::ranges::transform(value, value.begin(), [](unsigned char character) {
                   return std::tolower(character);
               });
               output_.emplace_back(value);
           },
           "The output location of the storage. "
           "'-' is supported to output to stdout when using json storage and this "
           "disables any logging."
    )
        ->default_val("-");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return std::unexpected{app.exit(e)};
    }

    if (output_.empty()) {
        output_.emplace_back("-");
        spdlog::set_level(spdlog::level::off);
    }

    return should_exit;
}

evget::StorageType evget::Cli::GetStorageType(std::string& output) {
    if (output.ends_with(".sqlite") || output.ends_with(".sqlite3") || output.ends_with(".db") ||
        output.ends_with(".db3") || output.ends_with(".s3db") || output.ends_with(".sl3")) {
        return StorageType::kSqLite;
    }

    return StorageType::kJson;
}

evget::Result<std::vector<std::unique_ptr<evget::Store>>> evget::Cli::ToStores() {
    auto stores = std::vector<std::unique_ptr<Store>>{};
    stores.reserve(output_.size());

    for (auto& output : this->output_) {
        switch (GetStorageType(output)) {
            case StorageType::kSqLite: {
                auto connect = std::make_unique<SQLiteConnection>();
                auto database = std::make_unique<DatabaseStorage>(std::move(connect), output);
                auto result = database->Init();
                if (!result.has_value()) {
                    return Err{result.error()};
                }

                stores.emplace_back(std::move(database));

                break;
            }
            case StorageType::kJson: {
                if (output == "-") {
                    // Do nothing to delete std::cout.
                    auto deleter = [](std::ostream*) {};
                    std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> out = {&std::cout, deleter};

                    stores.emplace_back(std::make_unique<JsonStorage>(std::move(out)));
                } else {
                    auto out = std::make_unique<std::ofstream>(output, std::ios_base::app);
                    stores.emplace_back(std::make_unique<JsonStorage>(std::move(out)));
                }

                break;
            }
        }
    }

    return stores;
}

size_t evget::Cli::StoreNEvents() const {
    return store_n_events_;
}

std::chrono::seconds evget::Cli::StoreAfter() const {
    return store_after_;
}
