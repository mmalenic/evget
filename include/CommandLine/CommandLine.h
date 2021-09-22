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
#include <filesystem>
#include <spdlog/spdlog.h>
#include "CommandLineOption.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

/**
 * Available filetypes.
 */
enum Filetype {
    csv,
    sqlite
};

/**
 * The CommandLine class controls command line options.
 */
class CommandLine {
public:
    /**
     * Create a CommandLine object.
     *
     * @param platformInformation platform information string
     */
    explicit CommandLine(const std::string& platformInformation);

    /**
    * Perform the actual command line parsing.
    *
    * @param argc from main
    * @param argv from main
    */
    void parseCommandLine(int argc, char** argv);

    /**
    * Get the file.
    * @return the file
    */
    [[nodiscard]] const fs::path& getFile() const;

    /**
     * Get file option.
     * @return file option
     */
    [[nodiscard]] const std::tuple<std::string, std::string, std::string>& getFileOption() const;

    /**
     * Get print option.
     * @return print option.
     */
    [[nodiscard]] const std::tuple<std::string, std::string, std::string>& getPrintOption() const;

    /**
     * Getter for print flag.
     * @return print flag
     */
    [[nodiscard]] bool isPrint() const;

    /**
     * Getter for raw events flag.
     * @return raw events flag
     */
    [[nodiscard]] bool isRawEvents() const;

    /**
     * Get the log level.
     * @return
     */
    [[nodiscard]] spdlog::level::level_enum getLogLevel() const;

    /**
     * Get the file type.
     * @return file type
     */
    [[nodiscard]] const Filetype& getFiletype() const;

    /**
     * Handles command line options that have simple logic, like the help message,
     * or version information.
     */
    void simpleArgs();

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
     * Extract the filetyoe from the string.
     * @param str string to use
     * @return filetype
     */
    static Filetype extractFiletype(const std::string& str);

    /**
     * Checks to see that the options are correctly specified, and finalises object.
     */
    virtual void readArgs();

    friend std::ostream& operator<<(std::ostream& os, const Filetype& filetype);
    friend std::istream& operator>>(std::istream& in, Filetype& algorithm);

    virtual ~CommandLine() = default;
    CommandLine(const CommandLine&) = default;
    CommandLine(CommandLine&&) = default;
    CommandLine& operator=(const CommandLine&) = delete;
    CommandLine& operator=(CommandLine&&) = delete;

protected:
    /**
     * Get description.
     * @return description
     */
    po::options_description& getDesc();

    /**
     * Get the variables map.
     * @return variables map
     */
    [[nodiscard]] const po::variables_map& getVm() const;

private:
    po::options_description desc;
    po::variables_map vm;

    const CommandLineOption<bool> helpOption;
    const CommandLineOption<bool> versionOption;
    const CommandLineOption<fs::path> storageFolder;
    const CommandLineOption<std::vector<Filetype>> filetypes;
    const CommandLineOption<bool> print;
    const CommandLineOption<bool> useRawEvents;
    const CommandLineOption<spdlog::level::level_enum> logLevel;
};

#endif //INPUT_EVENT_RECORDER_COMMANDLINE_H
