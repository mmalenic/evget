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

#include "CommandLine/Parser.h"

#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <utility>
#include <fmt/core.h>
#include <filesystem>

namespace algorithm = boost::algorithm;
namespace po = boost::program_options;
namespace fs = std::filesystem;

static constexpr char SQLITE_STRING[] = "sqlite";
static constexpr char CSV_STRING[] = "csv";
static constexpr char PROJECT_NAME[] = "evget";
static constexpr char DESCRIPTION[] = "Usage: evget [OPTION]...\n"
                                      "Shows events from input devices.\n"
                                      "Written by Marko Malenic 2021.\n\n"
                                      "Options";
static constexpr char VERSION[] = "0.1";
static constexpr char LICENSE_INFO[] = "Copyright (C) 2021 Marko Malenic.\n"
                                       "This program comes with ABSOLUTELY NO WARRANTY.\n"
                                       "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
                                       "Written by Marko Malenic 2021.";
static constexpr char DEFAULT_FOLDER_NAME[] = ".evget";
static constexpr char DEFAULT_CONFIG_NAME[] = ".evget_config";
static constexpr char ENVIRONMENT_VARIABLE_PREFIX[] = "EVGET_";
static constexpr char CONFIG_FILE_COMMENT_LINE[] = "# The following values represent the defaults for evget.\n"
                                                   "# Commented out values are required to be present, either in the config file or on the command line.";

std::ostream& operator<<(std::ostream& os, const CommandLine::Filetype& filetype) {
    switch (filetype) {
    case CommandLine::Filetype::sqlite:os << SQLITE_STRING;
        break;
    case CommandLine::csv:os << CSV_STRING;
        break;
    }
    return os;
}

std::istream& operator>>(std::istream& in, CommandLine::Filetype& algorithm) {
    std::string token{};
    in >> token;
    algorithm::to_lower(token);

    if (token == SQLITE_STRING) {
        algorithm = CommandLine::Filetype::sqlite;
    } else if (token == CSV_STRING) {
        algorithm = CommandLine::Filetype::csv;
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value, "filetype", token);
    }
    return in;
}

CommandLine::Parser::Parser(std::string platform) :
        platformInformation{std::move(platform)},
        cmdlineDesc{
        DESCRIPTION
    },
        configDesc{},
        vm{},
        help{
        OptionBuilder<bool>{cmdlineDesc}
            .shortName('h')
            .longName("help")
            .description("Produce help message describing program options.")
            .buildFlag()
    },
        version{
        OptionBuilder<bool>{cmdlineDesc}
            .shortName('v')
            .longName("version")
            .description("Produce version message.")
            .buildFlag()
    },
        config{
            OptionBuilder<fs::path>{cmdlineDesc}
                    .shortName('c')
                    .longName("config")
                    .description("Location of config file.")
                    .defaultValue(fs::current_path() / DEFAULT_CONFIG_NAME)
                    .build()
    },
        storageFolder{
        OptionBuilder<fs::path>{configDesc}
            .shortName('o')
            .longName("folder")
            .description("Folder location where events are stored.")
            .defaultValue(fs::current_path() / DEFAULT_FOLDER_NAME)
            .build()
    },
        filetypes{
        OptionBuilder<std::vector<Filetype>>{configDesc}
            .shortName('t')
            .longName("filetypes")
            .description("Filetypes used to store events.")
            .defaultValue(std::vector{sqlite})
            .representation("sqlite")
            //.validator()
            .build()
    },
        print{
        OptionBuilder<bool>{configDesc}
            .shortName('p')
            .longName("print")
            .description("Print events.")
            .buildFlag()
    },
        systemEvents{
        OptionBuilder<bool>{configDesc}
            .shortName('s')
            .longName("use-system-events")
            .description("Capture raw system events as well as cross platform events.")
            .buildFlag()
    },
        logLevel{
        OptionBuilder<spdlog::level::level_enum>{configDesc}
            .shortName('u')
            .longName("log-level")
            .description("log level to show messages at, defaults to \"warn\".\nValid values are:\n" + logLevelsString())
            .defaultValue(spdlog::level::warn)
            .representation("warn")
            .build(validateLogLevel)
    }
{
}

bool CommandLine::Parser::parseCommandLine(int argc, const char* argv[]) {
    po::options_description cmdlineOptions{};
    cmdlineOptions.add(cmdlineDesc).add(configDesc);
    storeAndNotify(po::command_line_parser(argc, argv).options(cmdlineOptions).allow_unregistered().run(), vm);

    if (!parseCmdlineOnlyOptions()) {
        return false;
    }

    if (!fs::exists(config.getValue())) {
        std::ofstream out(config.getValue());
        out << formatConfigFile();
        out.flush();
        out.close();
    }

    std::ifstream stream{config.getValue()};
    if (!stream) {
        throw std::ios_base::failure{"failed to read config file."};
    } else {
        storeAndNotify(po::parse_config_file(stream, configDesc), vm);
    }

    storeAndNotify(po::parse_environment(configDesc, ENVIRONMENT_VARIABLE_PREFIX), vm);

    if (!parseFileAndCmdlineOptions()) {
        return false;
    }

    return true;
}

CommandLine::fs::path CommandLine::Parser::getFolder() const {
    return storageFolder.getValue();
}

po::options_description& CommandLine::Parser::getCombinedDesc() {
    return cmdlineDesc.add(configDesc);
}

po::options_description& CommandLine::Parser::getCmdlineDesc() {
    return cmdlineDesc;
}

po::options_description& CommandLine::Parser::getConfigDesc() {
    return configDesc;
}

const po::variables_map& CommandLine::Parser::getVm() const {
    return vm;
}

bool CommandLine::Parser::shouldPrint() const {
    return print.getValue();
}

spdlog::level::level_enum CommandLine::Parser::getLogLevel() const {
    return logLevel.getValue();
}

std::string CommandLine::Parser::logLevelsString() {
    return fmt::format(
            "[ {}, {}, {}, {}, {}, {}, {} ]",
            spdlog::level::to_string_view(spdlog::level::trace),
            spdlog::level::to_string_view(spdlog::level::debug),
            spdlog::level::to_string_view(spdlog::level::info),
            spdlog::level::to_string_view(spdlog::level::warn),
            spdlog::level::to_string_view(spdlog::level::err),
            spdlog::level::to_string_view(spdlog::level::critical),
            spdlog::level::to_string_view(spdlog::level::off)
            );
}

std::optional<spdlog::level::level_enum> CommandLine::Parser::validateLogLevel(std::string logLevel) {
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

void CommandLine::Parser::storeAndNotify(const boost::program_options::parsed_options &parsedOptions, po::variables_map &vm) {
    po::store(parsedOptions, vm);
    po::notify(vm);
}

std::vector<CommandLine::Filetype> CommandLine::Parser::getFiletype() const {
    return filetypes.getValue();
}

bool CommandLine::Parser::useSystemEvents() const {
    return systemEvents.getValue();
}

std::filesystem::path CommandLine::Parser::getConfigFile() const {
    return config.getValue();
}

template<typename T>
std::string CommandLine::Parser::formatConfigOption(const OptionBase<T>& option) {
    std::ostringstream value{};
    value << std::boolalpha;
    value << option.getDefaultValue().value();
    return formatConfigOption(option, value.str());
}


template<typename T>
std::string CommandLine::Parser::formatConfigOption(const OptionBase<T>& option, const std::string& value) {
    std::string out{};
    if (option.isRequired()) {
        out += "# ";
    }
    out += option.getLongName();
    out += " = ";
    out += value;
    out += "\n";
    return out;
}

std::string CommandLine::Parser::formatConfigFile() {
    std::string out{};
    out += CONFIG_FILE_COMMENT_LINE;
    out += "\n\n";

    out += formatConfigOption(storageFolder);
    out += formatConfigOption(filetypes, filetypes.getRepresentation());
    out += formatConfigOption(print);
    out += formatConfigOption(systemEvents);
    out += formatConfigOption(logLevel, logLevel.getRepresentation());

    return out;
}

bool CommandLine::Parser::parseCmdlineOnlyOptions() {
    help.run(vm);
    version.run(vm);
    config.run(vm);

    if (help.getValue()) {
        std::cout << getCombinedDesc() << "\n";
        return false;
    }
    if (version.getValue()) {
        std::cout << PROJECT_NAME << " (" << platformInformation << ") " << VERSION << ".\n\n" << LICENSE_INFO << "\n";
        return false;
    }
    return true;
}

bool CommandLine::Parser::parseFileAndCmdlineOptions() {
    storageFolder.run(vm);
    filetypes.run(vm);
    print.run(vm);
    systemEvents.run(vm);
    logLevel.run(vm);

    return true;
}
