// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef INPUT_EVENT_RECORDER_COMMANDLINE_H
#define INPUT_EVENT_RECORDER_COMMANDLINE_H

#include <boost/program_options.hpp>
#include <filesystem>

namespace po = boost::program_options;
namespace fs = std::filesystem;

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
    CommandLine(const std::string& platformInformation);

    /**
    * Perform the actual command line parsing.
    *
    * @param argc from main
    * @param argv from main
    */
    void parseCommandLine(int argc, char **argv);

    /**
    * Get the default file location.
    * @return default file location
    */
    fs::path defaultFile();

    /**
    * Get the file.
    * @return the file
    */
    const fs::path &getFile() const;

    /**
     * Get file option.
     * @return file option
     */
    const std::tuple<std::string, std::string, std::string> &getFileOption() const;

    /**
     * Get print option.
     * @return print option.
     */
    const std::tuple<std::string, std::string, std::string> &getPrintOption() const;

    /**
     * Getter for print flag.
     * @return print flag
     */
    bool isPrint() const;

    /**
     * Handles command line options that have simple logic, like the help message,
     * or version information.
     */
    virtual void read_args();

    /**
     * Get the platform information from the system.
     * @return platform information
     */
    virtual std::string platformInformation() = 0;

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
    po::options_description &getDesc();

    /**
     * Get the variables map.
     * @return variables map
     */
    const po::variables_map &getVm() const;

private:
    po::options_description desc;
    po::variables_map vm;

    const std::string versionNumber;
    const std::string versionMessage;
    const std::string filename;

    std::tuple<std::string, std::string, std::string> fileOption;
    std::tuple<std::string, std::string, std::string> printOption;

    fs::path file;
    bool print{};
};

#endif //INPUT_EVENT_RECORDER_COMMANDLINE_H
