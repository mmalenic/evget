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

#include "../include/CommandLine.h"

#include <iostream>

using namespace std;

CommandLine::CommandLine(int argc, char **argv, const string& default_file) :
    platform_information { "unknown" },
    version_message {
        "input-event-recorder (" + platform_information + ") " + VERSION + ".\n\n"
        "Copyright (C) 2021 Marko Malenic.\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n"
        "This is free software, and you are welcome to redistribute it under certain conditions.\n\n"
        "Written by Marko Malenic 2021.\n"
    },
    desc { HELP_MESSAGE } {
    desc.add_options()
        ("help,h", "produce help message.")
        ("version,v", "version information.")
        ("file,f", po::value<string>(&this->file)->default_value(default_file),
                "file to store events, defaults to current directory.")
        ;

    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);
}

const std::string &CommandLine::getFile() const {
    return file;
}

void CommandLine::executeSimple() {
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }
    if (vm.count("version")) {
        cout << version_message;
        exit(EXIT_SUCCESS);
    }
}