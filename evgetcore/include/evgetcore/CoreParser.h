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

#ifndef INPUT_EVENT_RECORDER_COMMANDLINE_H
#define INPUT_EVENT_RECORDER_COMMANDLINE_H

#include <boost/program_options.hpp>
#include <clioption/Option.h>
#include <clioption/OptionFlag.h>
#include <clioption/OptionValidated.h>
#include <clioption/Parser.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace EvgetCore {

namespace po = boost::program_options;
namespace fs = std::filesystem;

/**
 * The CoreParser class controls command line options.
 */
class CoreParser : CliOption::Parser {
public:
    /**
     * Create a CoreParser object.
     *
     * @param platform platform information string
     */
    explicit CoreParser(std::string platform);

    /**
     * Get config file location.
     */
    [[nodiscard]] fs::path getConfigFile() const;

    /**
     * Get the folder.
     */
    [[nodiscard]] fs::path getFolder() const;

    /**
     * Getter for print flag.
     * @return print flag
     */
    [[nodiscard]] bool shouldPrint() const;

    /**
     * Getter for system events flag.
     * @return system events flag
     */
    [[nodiscard]] bool useSystemEvents() const;

    /**
     * Get the log level.
     * @return
     */
    [[nodiscard]] spdlog::level::level_enum getLogLevel() const;

    bool parseCommandLine(int argc, const char** argv, po::variables_map& vm) override;

protected:
    /**
     * Get description.
     * @return description
     */
    po::options_description& getCombinedDesc();

    /**
     * Get commandline description.
     */
    po::options_description& getCmdlineDesc();

    /**
     * Get config description.
     */
    po::options_description& getConfigDesc();

private:
    std::string platformInformation;
    po::options_description cmdlineDesc;
    po::options_description configDesc;

    CliOption::OptionFlag help;
    CliOption::OptionFlag version;
    CliOption::Option<fs::path> config;
    CliOption::Option<fs::path> folder;
    CliOption::OptionFlag print;
    CliOption::OptionFlag systemEvents;
    CliOption::OptionValidated<spdlog::level::level_enum> logLevel;

    /**
     * Get the valid log levels.
     * @return valid log levels
     */
    static std::string logLevelsString();

    /**
     * Converts the log level string into a log level if possible.
     */
    static std::optional<spdlog::level::level_enum> validateLogLevel(std::string logLevel);

    /**
     * Format an option.
     */
    template <typename T>
    static std::string formatConfigOption(const CliOption::AbstractOption<T>& option);

    /**
     * Format an option.
     */
    template <typename T>
    static std::string formatConfigOption(const CliOption::AbstractOption<T>& option, const std::string& value);

    /**
     * Format the config file.
     */
    virtual std::string formatConfigFile();
};

}  // namespace EvgetCore
#endif  // INPUT_EVENT_RECORDER_COMMANDLINE_H
