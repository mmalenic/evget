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
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

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
     * Handles command line options that have simple logic, like the help message,
     * or version information.
     */
    void executeSimple();

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
    static fs::path defaultFile();

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

    /**
     * Get the platform information from the system.
     * @return platform information
     */
    virtual std::string platformInformation() = 0;

    /**
     * Get the file.
     * @return the file
     */
    virtual const fs::path &getFile() const = 0;

protected:
    po::options_description desc;
    po::variables_map vm;

private:
    const std::string versionNumber;
    const std::string versionMessage;
    const std::string filename;
};

#endif //INPUT_EVENT_RECORDER_COMMANDLINE_H
