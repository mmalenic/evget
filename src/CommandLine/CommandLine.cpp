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
    helpOption{CommandLineOptionBuilder<bool>{desc}.shortName("h").longName("help").description("Produce help message describing program options.").build()},
    versionOption{CommandLineOptionBuilder<bool>{desc}.shortName("v").longName("version").description("Produce version message.").build()},
    storageFolder{
    CommandLineOptionBuilder<fs::path>{desc}
        .shortName("o")
        .longName("folder")
        .description("Folder location where events are stored.")
        .defaultValue(fs::current_path() / DEFAULT_FOLDER_NAME)
        .build()
    },
    filetypes{
        CommandLineOptionBuilder<std::vector<Filetype>>{desc}
            .shortName("t")
            .longName("filetypes")
            .description("Filetypes used to store events.")
            .defaultValue(std::vector{sqlite})
            .build()
    },
    print{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("p")
            .longName("print")
            .description("Print events.")
            .defaultValue(false)
            .build()
    },
    useRawEvents{
        CommandLineOptionBuilder<bool>{desc}
            .shortName("r")
            .longName("use-raw-events")
            .description("Capture raw system events as well as universal cross platform events.")
            .defaultValue(false)
            .build()
    },
    logLevel{
        CommandLineOptionBuilder<spdlog::level::level_enum>{desc}
            .shortName("u")
            .longName("log-level")
            .description("log level to show messages at, defaults to \"warn\".\n Valid values are:\n" + validLogLevels())
            .defaultValue(spdlog::level::warn)
            .build()
    } {
}

void CommandLine::parseCommandLine(int argc, char** argv) {
    try {
        store(parse_command_line(argc, argv, desc), vm);
    } catch (po::error& error) {
        cout << "Error: " << error.what() << "\n";
        exit(EXIT_SUCCESS);
    }
    notify(vm);
}

void CommandLine::simpleArgs() {
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }
//    if (vm.count("version")) {
//        cout << versionMessage;
//        exit(EXIT_SUCCESS);
//    }
}

void CommandLine::readArgs() {
//    if (vm.count(get<0>(logLevelOption)) && !vm[get<0>(logLevelOption)].defaulted()) {
//        auto option = vm[get<0>(logLevelOption)].as<string>();
//
//        if (option == "off") {
//            logLevel = spdlog::level::from_str(vm[get<0>(logLevelOption)].as<string>());
//            return;
//        }
//        if (spdlog::level::from_str(option) == spdlog::level::off) {
//            cout << "Invalid log level setting.\n";
//            cout << desc << "\n";
//            exit(EXIT_SUCCESS);
//        }
//
//        logLevel = spdlog::level::from_str(vm[get<0>(logLevelOption)].as<string>());
//    }
//    if ((vm.count(get<0>(filetypeOption)) && !vm[get<0>(filetypeOption)].defaulted())
//        && !(vm.count(get<0>(fileOption)) && !vm[get<0>(fileOption)].defaulted())) {
//        cout << "The filetype option can only be specified if the file option is also specified.\n";
//        exit(EXIT_SUCCESS);
//    }
}

const fs::path& CommandLine::getFile() const {
//    return file;
}

po::options_description& CommandLine::getDesc() {
    return desc;
}

const po::variables_map& CommandLine::getVm() const {
    return vm;
}

bool CommandLine::isPrint() const {
//    return print;
}

const tuple<std::string, std::string, std::string>& CommandLine::getFileOption() const {
//    return fileOption;
}

const tuple<std::string, std::string, std::string>& CommandLine::getPrintOption() const {
//    return printOption;
}

spdlog::level::level_enum CommandLine::getLogLevel() const {
//    return logLevel;
}

std::string CommandLine::validLogLevels() {
    auto levels = {
        spdlog::level::to_string_view(spdlog::level::trace).data(),
        spdlog::level::to_string_view(spdlog::level::debug).data(),
        spdlog::level::to_string_view(spdlog::level::info).data(),
        spdlog::level::to_string_view(spdlog::level::warn).data(),
        spdlog::level::to_string_view(spdlog::level::err).data(),
        spdlog::level::to_string_view(spdlog::level::critical).data(),
        spdlog::level::to_string_view(spdlog::level::off).data()
    };

    string stringOut;
    stringOut += "[ ";
    for (auto i{levels.begin()}; i != prev(levels.end()); ++i) {
        stringOut += *i;
        stringOut += ", ";
    }
    stringOut += *prev(levels.end());
    stringOut += " ]";

    return stringOut;
}

const Filetype& CommandLine::getFiletype() const {
//    return filetype;
}

bool CommandLine::isRawEvents() const {
//    return useRawEvents;
}

Filetype CommandLine::extractFiletype(const string& str) {
    fs::path path{str};
    string ext{path.extension()};
    ext.erase(0, 1);
    istringstream stream{ext};
    Filetype filetype{};
    try {
        stream >> filetype;
    } catch (po::validation_error& err) {
        spdlog::warn(
            string{"Cannot extract filetype from filename alone: "} + err.what() + " Using default filetype sqlite."
        );
        return sqlite;
    }
    return filetype;
}