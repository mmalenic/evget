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
#include <clioption/OptionBuilder.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <iostream>
#include <utility>

#include "evgetcore/Parser.h"

namespace algorithm = boost::algorithm;
namespace po = boost::program_options;

static constexpr char SQLITE_STRING[] = "sqlite";
static constexpr char CSV_STRING[] = "csv";
static constexpr char PROJECT_NAME[] = "evget";
static constexpr char DESCRIPTION[] =
    "Usage: evget [OPTION]...\n"
    "Shows events from input devices.\n"
    "Written by Marko Malenic 2021.\n\n"
    "Options";
static constexpr char VERSION[] = "0.1";
static constexpr char LICENSE_INFO[] =
    "Copyright (C) 2021 Marko Malenic.\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
    "Written by Marko Malenic 2021.";
static constexpr char DEFAULT_FOLDER_NAME[] = ".evgetx11";
static constexpr char DEFAULT_CONFIG_NAME[] = ".evget_config";
static constexpr char ENVIRONMENT_VARIABLE_PREFIX[] = "EVGET_";
static constexpr char CONFIG_FILE_COMMENT_LINE[] =
    "# The following values represent the defaults for evgetx11.\n"
    "# Commented out values are required to be present, either in the config file or on the command line.";

EvgetCore::Parser::Parser(std::string platform)
    : parser{DESCRIPTION, "", ""},
      platformInformation{std::move(platform)},
      help{CliOption::OptionBuilder<bool>{parser.cmdlineDesc()}
               .shortName('h')
               .longName("help")
               .description("Produce help message describing program options.")
               .buildFlag()},
      version{CliOption::OptionBuilder<bool>{parser.cmdlineDesc()}
                  .shortName('v')
                  .longName("version")
                  .description("Produce version message.")
                  .buildFlag()},
      config{CliOption::OptionBuilder<fs::path>{parser.cmdlineDesc()}
                 .shortName('c')
                 .longName("config")
                 .description("Location of config file.")
                 .defaultValue(fs::current_path() / DEFAULT_CONFIG_NAME)
                 .build()},
      folder{CliOption::OptionBuilder<fs::path>{parser.configDesc()}
                 .shortName('o')
                 .longName("folder")
                 .description("Folder location where events are stored.")
                 .defaultValue(fs::current_path() / DEFAULT_FOLDER_NAME)
                 .build()},
      print{CliOption::OptionBuilder<bool>{parser.configDesc()}
                .shortName('p')
                .longName("print")
                .description("Print events.")
                .buildFlag()},
      systemEvents{CliOption::OptionBuilder<bool>{parser.configDesc()}
                       .shortName('s')
                       .longName("use-system-events")
                       .description("Capture raw system events as well as cross platform events.")
                       .buildFlag()},
      logLevel{CliOption::OptionBuilder<spdlog::level::level_enum>{parser.configDesc()}
                   .shortName('u')
                   .longName("log-level")
                   .description(
                       "log level to show messages at, defaults to \"warn\".\nValid values are:\n" + logLevelsString()
                   )
                   .defaultValue(spdlog::level::warn)
                   .representation("warn")
                   .build(validateLogLevel)} {}

CliOption::Result<bool> EvgetCore::Parser::parseCommandLine(int argc, const char* argv[]) {
    parser.addCmdline(parser.configDesc());
    parser.parseCommandLine(argc, argv);

    auto result = parser.parseOption(help).and_then([this]() {
        return parser.parseOption(version);
    }).and_then([this]() {
        return parser.parseOption(config);
    });

    if (!result.has_value()) {
        return CliOption::Err{result.error()};
    }

    if (help.getValue()) {
        std::cout << parser.cmdlineDesc() << "\n";
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
        return CliOption::Err{{.type = CliOption::ErrorType::OptionError, .message = "failed to read config file."}};
    }

    parser.parseConfig(stream);

    parser.parseEnv(ENVIRONMENT_VARIABLE_PREFIX);

    return parser.parseOption(folder).and_then([this]() {
        return parser.parseOption(print);
    }).and_then([this]() {
        return parser.parseOption(systemEvents);
    }).and_then([this]() {
        return parser.parseOption(logLevel);
    }).transform([](){ return true; });
}

EvgetCore::fs::path EvgetCore::Parser::getFolder() const {
    return folder.getValue();
}

bool EvgetCore::Parser::shouldPrint() const {
    return print.getValue();
}

spdlog::level::level_enum EvgetCore::Parser::getLogLevel() const {
    return logLevel.getValue();
}

std::string EvgetCore::Parser::logLevelsString() {
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

std::optional<spdlog::level::level_enum> EvgetCore::Parser::validateLogLevel(std::string logLevel) {
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

bool EvgetCore::Parser::useSystemEvents() const {
    return systemEvents.getValue();
}

std::filesystem::path EvgetCore::Parser::getConfigFile() const {
    return config.getValue();
}

template <typename T>
std::string EvgetCore::Parser::formatConfigOption(const CliOption::AbstractOption<T>& option) {
    std::ostringstream value{};
    value << std::boolalpha;
    value << option.getDefaultValue().value();
    return formatConfigOption(option, value.str());
}

template <typename T>
std::string
EvgetCore::Parser::formatConfigOption(const CliOption::AbstractOption<T>& option, const std::string& value) {
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

std::string EvgetCore::Parser::formatConfigFile() {
    std::string out{};
    out += CONFIG_FILE_COMMENT_LINE;
    out += "\n\n";

    out += formatConfigOption(folder);
    out += formatConfigOption(print);
    out += formatConfigOption(systemEvents);
    out += formatConfigOption(logLevel, logLevel.getRepresentation());

    return out;
}
