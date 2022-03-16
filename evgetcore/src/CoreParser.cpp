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

#include "evgetcore/CoreParser.h"

#include <clioption/OptionBuilder.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <utility>
#include <fmt/core.h>
#include <filesystem>

namespace algorithm = boost::algorithm;
namespace po = boost::program_options;

static constexpr char SQLITE_STRING[] = "sqlite";
static constexpr char CSV_STRING[] = "csv";
static constexpr char PROJECT_NAME[] = "evgetx11";
static constexpr char DESCRIPTION[] = "Usage: evgetx11 [OPTION]...\n"
                                      "Shows events from input devices.\n"
                                      "Written by Marko Malenic 2021.\n\n"
                                      "Options";
static constexpr char VERSION[] = "0.1";
static constexpr char LICENSE_INFO[] = "Copyright (C) 2021 Marko Malenic.\n"
                                       "This program comes with ABSOLUTELY NO WARRANTY.\n"
                                       "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
                                       "Written by Marko Malenic 2021.";
static constexpr char DEFAULT_FOLDER_NAME[] = ".evgetx11";
static constexpr char DEFAULT_CONFIG_NAME[] = ".evget_config";
static constexpr char ENVIRONMENT_VARIABLE_PREFIX[] = "EVGET_";
static constexpr char CONFIG_FILE_COMMENT_LINE[] = "# The following values represent the defaults for evgetx11.\n"
                                                   "# Commented out values are required to be present, either in the config file or on the command line.";

std::ostream& operator<<(std::ostream& os, const Evget::Filetype& filetype) {
    switch (filetype) {
    case Evget::Filetype::sqlite:os << SQLITE_STRING;
        break;
    case Evget::csv:os << CSV_STRING;
        break;
    }
    return os;
}

std::istream& operator>>(std::istream& in, Evget::Filetype& algorithm) {
    std::string token{};
    in >> token;
    algorithm::to_lower(token);

    if (token == SQLITE_STRING) {
        algorithm = Evget::Filetype::sqlite;
    } else if (token == CSV_STRING) {
        algorithm = Evget::Filetype::csv;
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value, "filetype", token);
    }
    return in;
}

Evget::CoreParser::CoreParser(std::string platform) :
        platformInformation{std::move(platform)},
        cmdlineDesc{
        DESCRIPTION
    },
        configDesc{},
        help{
        CliOption::OptionBuilder<bool>{cmdlineDesc}
            .shortName('h')
            .longName("help")
            .description("Produce help message describing program options.")
            .buildFlag()
    },
        version{
            CliOption::OptionBuilder<bool>{cmdlineDesc}
            .shortName('v')
            .longName("version")
            .description("Produce version message.")
            .buildFlag()
    },
        config{
            CliOption::OptionBuilder<fs::path>{cmdlineDesc}
                    .shortName('c')
                    .longName("config")
                    .description("Location of config file.")
                    .defaultValue(fs::current_path() / DEFAULT_CONFIG_NAME)
                    .build()
    },
        folder{
            CliOption::OptionBuilder<fs::path>{configDesc}
            .shortName('o')
            .longName("folder")
            .description("Folder location where events are stored.")
            .defaultValue(fs::current_path() / DEFAULT_FOLDER_NAME)
            .build()
    },
        filetypes{
            CliOption::OptionBuilder<std::vector<Filetype>>{configDesc}
            .shortName('f')
            .longName("filetypes")
            .description("Filetypes used to store events.")
            .defaultValue(std::vector{sqlite})
            .representation("sqlite")
            .build()
    },
        print{
            CliOption::OptionBuilder<bool>{configDesc}
            .shortName('p')
            .longName("print")
            .description("Print events.")
            .buildFlag()
    },
        systemEvents{
            CliOption::OptionBuilder<bool>{configDesc}
            .shortName('s')
            .longName("use-system-events")
            .description("Capture raw system events as well as cross platform events.")
            .buildFlag()
    },
        logLevel{
            CliOption::OptionBuilder<spdlog::level::level_enum>{configDesc}
            .shortName('u')
            .longName("log-level")
            .description("log level to show messages at, defaults to \"warn\".\nValid values are:\n" + logLevelsString())
            .defaultValue(spdlog::level::warn)
            .representation("warn")
            .build(validateLogLevel)
    }
{
}

bool Evget::CoreParser::parseCommandLine(int argc, const char* argv[], po::variables_map& vm) {
    po::options_description cmdlineOptions{};
    cmdlineOptions.add(cmdlineDesc).add(configDesc);
    storeAndNotify(po::command_line_parser(argc, argv).options(cmdlineOptions).allow_unregistered().run(), vm);

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

    if (!fs::exists(config.getValue())) {
        std::ofstream out(config.getValue());
        out << formatConfigFile();
        out.close();
    }

    std::ifstream stream{config.getValue()};
    if (!stream) {
        throw std::ios_base::failure{"failed to read config file."};
    } else {
        storeAndNotify(po::parse_config_file(stream, configDesc), vm);
    }

    storeAndNotify(po::parse_environment(configDesc, ENVIRONMENT_VARIABLE_PREFIX), vm);

    folder.run(vm);
    filetypes.run(vm);
    print.run(vm);
    systemEvents.run(vm);
    logLevel.run(vm);

    return true;
}

Evget::fs::path Evget::CoreParser::getFolder() const {
    return folder.getValue();
}

po::options_description& Evget::CoreParser::getCombinedDesc() {
    return cmdlineDesc.add(configDesc);
}

po::options_description& Evget::CoreParser::getCmdlineDesc() {
    return cmdlineDesc;
}

po::options_description& Evget::CoreParser::getConfigDesc() {
    return configDesc;
}

bool Evget::CoreParser::shouldPrint() const {
    return print.getValue();
}

spdlog::level::level_enum Evget::CoreParser::getLogLevel() const {
    return logLevel.getValue();
}

std::string Evget::CoreParser::logLevelsString() {
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

std::optional<spdlog::level::level_enum> Evget::CoreParser::validateLogLevel(std::string logLevel) {
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

std::vector<Evget::Filetype> Evget::CoreParser::getFiletype() const {
    return filetypes.getValue();
}

bool Evget::CoreParser::useSystemEvents() const {
    return systemEvents.getValue();
}

std::filesystem::path Evget::CoreParser::getConfigFile() const {
    return config.getValue();
}

template<typename T>
std::string Evget::CoreParser::formatConfigOption(const CliOption::AbstractOption<T>& option) {
    std::ostringstream value{};
    value << std::boolalpha;
    value << option.getDefaultValue().value();
    return formatConfigOption(option, value.str());
}


template<typename T>
std::string Evget::CoreParser::formatConfigOption(const CliOption::AbstractOption<T>& option, const std::string& value) {
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

std::string Evget::CoreParser::formatConfigFile() {
    std::string out{};
    out += CONFIG_FILE_COMMENT_LINE;
    out += "\n\n";

    out += formatConfigOption(folder);
    out += formatConfigOption(filetypes, filetypes.getRepresentation());
    out += formatConfigOption(print);
    out += formatConfigOption(systemEvents);
    out += formatConfigOption(logLevel, logLevel.getRepresentation());

    return out;
}
