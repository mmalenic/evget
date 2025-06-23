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

#include "evgetcore/cli.h"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <spdlog/cfg/env.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <utility>

#include "evgetcore/database/sqlite/Connection.h"
#include "evgetcore/Storage/DatabaseStorage.h"
#include "evgetcore/Storage/JsonStorage.h"

const std::vector<std::string>& EvgetCore::Cli::output() const {
    return this->output_;
}

std::expected<bool, int> EvgetCore::Cli::parse(int argc, char** argv) {
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
        ->default_val(100);
    app.add_option(
           "-s,--store-after-seconds",
           store_after_,
           "Store events at least every interval specified with this option, even if fewer events than "
           "`--store-n-events` has been receieved."
    )
        ->default_val(60);

    app.add_option_function<std::string>(
           "-o,--output",
           [this](std::string value) {
               if (value == "-") {
                   spdlog::set_level(spdlog::level::off);
               }

               std::ranges::transform(value, value.begin(), [](unsigned char c) { return std::tolower(c); });
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

    return static_cast<int>(should_exit);
}

EvgetCore::StorageType EvgetCore::Cli::get_storage_type(std::string& output) {
    if (output.ends_with(".sqlite") || output.ends_with(".sqlite3") || output.ends_with(".db") ||
        output.ends_with(".db3") || output.ends_with(".s3db") || output.ends_with(".sl3")) {
        return StorageType::SQLite;
    }

    return StorageType::Json;
}

std::vector<std::unique_ptr<EvgetCore::Storage::Store>> EvgetCore::Cli::to_stores() {
    auto stores = std::vector<std::unique_ptr<Storage::Store>>{};
    stores.reserve(output_.size());

    for (auto& output : this->output_) {
        switch (this->get_storage_type(output)) {
            case EvgetCore::StorageType::SQLite: {
                auto connect = std::make_unique<EvgetCore::SQLiteConnection>();
                auto database = std::make_unique<EvgetCore::Storage::DatabaseStorage>(std::move(connect), output);
                database->init();

                stores.emplace_back(std::move(database));

                break;
            }
            case EvgetCore::StorageType::Json: {
                if (output == "-") {
                    // Do nothing to delete std::cout.
                    auto deleter = [](std::ostream* _std_cout) {};
                    std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> out = {&std::cout, deleter};

                    stores.emplace_back(std::make_unique<EvgetCore::Storage::JsonStorage>(std::move(out)));
                } else {
                    auto out = std::make_unique<std::ofstream>(output, std::ios_base::app);
                    stores.emplace_back(std::make_unique<EvgetCore::Storage::JsonStorage>(std::move(out)));
                }

                break;
            }
        }
    }

    return stores;
}

size_t EvgetCore::Cli::store_n_events() const {
    return store_n_events_;
}

std::chrono::seconds EvgetCore::Cli::store_after() const {
    return store_after_;
}
