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

namespace po = boost::program_options;

static const std::string VERSION = "1.0";
static const std::string HELP_MESSAGE =
    "Records events from input devices and stores them in a SQLite file.\n"
    "Written by Marko Malenic 2021.\n\n"
    "Options";

/**
 * The CommandLine class controls command line options.
 */
class CommandLine {
private:
    const std::string platform_information;
    const std::string version_message;

    std::string file;

    po::options_description desc;
    po::variables_map vm;

public:
    /**
     * Create a CommandLine object.
     *
     * @param argc from main
     * @param argv from main
     */
    CommandLine(int argc, char *argv[], const std::string& file);

    /**
     * Get the file storing events.
     * @return file storing events
     */
    const std::string &getFile() const;

    /**
     * Handles command line options that have simple logic, like the help message,
     * or version information.
     */
    void executeSimple();
};

#endif //INPUT_EVENT_RECORDER_COMMANDLINE_H
