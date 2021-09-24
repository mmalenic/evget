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

#include "CommandLine/CommandLine.h"

#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace algorithm = boost::algorithm;

static constexpr char SQLITE_STRING[] = "sqlite";
static constexpr char CSV_STRING[] = "csv";
static constexpr char DESCRIPTION[] = "Usage: evget [OPTION]...\n"
                                      "Shows events from input devices.\n"
                                      "Written by Marko Malenic 2021.\n\n"
                                      "Options";
static constexpr char VERSION[] = "1.0";
static constexpr char LICENSE_INFO[] = "Copyright (C) 2021 Marko Malenic.\n"
                                       "This program comes with ABSOLUTELY NO WARRANTY.\n"
                                       "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
                                       "Written by Marko Malenic 2021.\n";
static constexpr char DEFAULT_FOLDER_NAME[] = ".evget";

std::ostream& operator<<(std::ostream& os, const Filetype& filetype) {
    switch (filetype) {
    case Filetype::sqlite:os << SQLITE_STRING;
        break;
    case csv:os << CSV_STRING;
        break;
    }
    return os;
}

std::istream& operator>>(std::istream& in, Filetype& algorithm) {
    std::string token{};
    in >> token;
    algorithm::to_lower(token);

    if (token == SQLITE_STRING) {
        algorithm = Filetype::sqlite;
    } else if (token == CSV_STRING) {
        algorithm = Filetype::csv;
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value, "filetype", token);
    }
    return in;
}

CommandLine::CommandLine(const std::string& platformInformation) :
    desc{
        DESCRIPTION
    },
    vm{},
    options{},
    help{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("h")
            .longName("help")
            .description("Produce help message describing program options.")
            .store(options)
            .build()
    },
    version{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("v")
            .longName("version")
            .description("Produce version message.")
            .store(options)
            .build()
    },
    storageFolder{
        CommandLineOptionBuilder<fs::path>{desc}
            .shortName("o")
            .longName("folder")
            .description("Folder location where events are stored.")
            .defaultValue(fs::current_path() / DEFAULT_FOLDER_NAME)
            .store(options)
            .build()
    },
    filetypes{
        CommandLineOptionBuilder<std::vector<Filetype>>{desc}
            .shortName("t")
            .longName("filetypes")
            .description("Filetypes used to store events.")
            .defaultValue(std::vector{sqlite})
            .store(options)
            //.validator()
            .build()
    },
    print{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("p")
            .longName("print")
            .description("Print events.")
            .defaultValue(false)
            .store(options)
            .build()
    },
    systemEvents{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("s")
            .longName("use-system-events")
            .description("Capture raw system events as well as cross platform events.")
            .defaultValue(false)
            .store(options)
            .build()
    },
    logLevel{
        CommandLineOptionBuilder<spdlog::level::level_enum>{desc}
            .shortName("u")
            .longName("log-level")
            .description("log level to show messages at, defaults to \"warn\".\n Valid values are:\n" + logLevelsString())
            .defaultValue(spdlog::level::warn)
            .validator(validateLogLevel)
            .store(options)
            .build()
    }
{
}

bool CommandLine::parseCommandLine(int argc, char** argv) {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    store(parsed, vm);
    notify(vm);

    for (auto const &option: options) {
        option->afterRead(vm);
    }

    if (help.getValue()) {
        std::cout << desc << "\n";
    }
    if (version.getValue()) {

    }


    return true;
}

fs::path CommandLine::getFolder() const {
    return storageFolder.getValue();
}

po::options_description& CommandLine::getDesc() {
    return desc;
}

const po::variables_map& CommandLine::getVm() const {
    return vm;
}

bool CommandLine::shouldPrint() const {
    return print.getValue();
}

spdlog::level::level_enum CommandLine::getLogLevel() const {
    return logLevel.getValue();
}

std::string CommandLine::logLevelsString() {
    std::string stringOut;
    stringOut += "[ ";
    for (auto i{std::begin(SPDLOG_LEVEL_NAMES)}; i != std::prev(std::begin(SPDLOG_LEVEL_NAMES)); ++i) {
        stringOut += i->data();
        stringOut += ", ";
    }
    stringOut += std::prev(std::begin(SPDLOG_LEVEL_NAMES))->data();
    stringOut += " ]";

    return stringOut;
}

std::optional<spdlog::level::level_enum> CommandLine::validateLogLevel(std::string logLevel) {
	algorithm::to_lower(logLevel);
    if (logLevel == "off" || logLevel == "o") {
		return spdlog::level::off;
    }
    auto level = spdlog::level::from_str(logLevel);
    if (level == spdlog::level::off) {
        return std::nullopt;
    }
    return level;
}

std::vector<Filetype> CommandLine::getFiletype() const {
    return filetypes.getValue();
}

bool CommandLine::useSystemEvents() const {
    return systemEvents.getValue();
}

std::vector<std::unique_ptr<CommandLineOptionBase>>& CommandLine::getOptions() {
    return options;
}
