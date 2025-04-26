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

#include <boost/algorithm/string.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <iostream>
#include <utility>
#include <CLI/CLI.hpp>

#include "evgetcore/cli.h"

// static constexpr char SQLITE_STRING[] = "sqlite";
// static constexpr char CSV_STRING[] = "csv";
// static constexpr char PROJECT_NAME[] = "evget";
// static constexpr char DESCRIPTION[] =
//     "Usage: evget [OPTION]...\n"
//     "Shows events from input devices.\n"
//     "Written by Marko Malenic 2021.\n\n"
//     "Options";
// static constexpr char VERSION[] = "0.1";
// static constexpr char LICENSE_INFO[] =
//     "Copyright (C) 2021 Marko Malenic.\n"
//     "This program comes with ABSOLUTELY NO WARRANTY.\n"
//     "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
//     "Written by Marko Malenic 2021.";
// static constexpr char DEFAULT_FOLDER_NAME[] = ".evgetx11";
// static constexpr char DEFAULT_CONFIG_NAME[] = ".evget_config";
// static constexpr char ENVIRONMENT_VARIABLE_PREFIX[] = "EVGET_";
// static constexpr char CONFIG_FILE_COMMENT_LINE[] =
//     "# The following values represent the defaults for evgetx11.\n"
//     "# Commented out values are required to be present, either in the config file or on the command line.";


EvgetCore::StorageType EvgetCore::Cli::storage_type() const {
    return this->storage_type_;
}

spdlog::level::level_enum EvgetCore::Cli::log_level() const {
    return this->log_level_;
}

const std::string & EvgetCore::Cli::output() const {
    return this->output_;
}

bool EvgetCore::Cli::output_to_stdout() const {
    return this->output_to_stdout_;
}

std::expected<bool, int> EvgetCore::Cli::parse(int argc, char** argv) {
    CLI::App app{"Show and store events from input devices.", "evget"};
    argv = app.ensure_utf8(argv);

    auto should_exit = false;
    app.add_flag_callback(
        "-v,--version",
        [&should_exit] {
            should_exit = true;
            std::cout << std::format(
                "evlist {}\n\n{}\n{}\n",
                EVGET_VERSION,
                EVGET_LICENSE,
                EVGET_COPYRIGHT
            );
        },
        "Print version"
    );

    app.add_option("-s,--storage-type", storage_type_)
->transform(CLI::Transformer{storage_type_mappings(), CLI::ignore_case})
->option_text(format_enum(
    "STORAGE_TYPE",
    "How to store or display events",
    STORAGE_TYPE_INDENT_BY,
    storage_type_descriptions()
));
    app.add_option("-l,--log-level", log_level_)
        ->transform(CLI::Transformer{log_level_mappings(), CLI::ignore_case})
        ->option_text(format_enum(
            "LOG_LEVEL",
            "Level of log messages to print to stdout",
            LOG_LEVEL_INDENT_BY,
            log_level_descriptions()
        ));

    auto code = 0;
    app.add_option_function<std::string>("-o,--output", [this, &code](const std::string& value) {
        if (value == "-") {
            log_level_ = spdlog::level::off;

            if (storage_type_ != StorageType::Json) {
                std::cout << "cannot output to stdout if not using json storage\n";
                code = 1;
            }

            output_to_stdout_ = true;
        }

        output_ = value;
    }, "The output location of the storage. "
    "'-' is supported to output to stdout when using json storage and this "
    "disables any logging.");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return std::unexpected{app.exit(e)};
    }

    if (code > 0) {
        return std::unexpected{code};
    }

    if (output_.empty()) {
        switch (storage_type_) {
            case StorageType::Json:
                output_ = "evget_store.json";
                break;
            case StorageType::SQLite:
                output_ = "evget_store.sqlite";
                break;
        }
    }

    return should_exit;
}

std::map<std::string, spdlog::level::level_enum> EvgetCore::Cli::log_level_mappings() {
    return {
    {"trace", spdlog::level::trace},
    {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},
    {"warn", spdlog::level::warn},
    {"err", spdlog::level::err},
    {"critical", spdlog::level::critical},
    {"off", spdlog::level::off}
        };
}

std::map<spdlog::level::level_enum, std::string> EvgetCore::Cli::log_level_descriptions() {
    return {
        {spdlog::level::trace, "- trace: enable trace logs"},
        {spdlog::level::debug, "- debug: enable debug logs"},
        {spdlog::level::info, "- info: enable info logs"},
        {spdlog::level::warn, "- warn: enable warning logs"},
        {spdlog::level::err, "- err: enable error logs"},
        {spdlog::level::critical, "- critical: enable critical logs"},
        {spdlog::level::off, "- off: disable logs"}
    };
}

std::map<std::string, EvgetCore::StorageType> EvgetCore::Cli::storage_type_mappings() {
    return {
        {"sqlite", StorageType::SQLite},
{"json", StorageType::Json},
    };
}

std::map<EvgetCore::StorageType, std::string> EvgetCore::Cli::storage_type_descriptions() {
    return {
            { StorageType::SQLite, "- sqlite: store events in an sqlite database"},
    {StorageType::Json, "- json: store events as JSON"},
        };
}
