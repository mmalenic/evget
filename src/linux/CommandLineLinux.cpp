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

#include "linux/CommandLineLinux.h"

#include "sys/utsname.h"

using namespace std;

CommandLineLinux::CommandLineLinux(int argc, char **argv) :
    CommandLine { CommandLineLinux::platformInformation() } {
    getDesc().add_options()
        ("file,f", po::value<string>(&this->file)->default_value(CommandLineLinux::defaultFile()),
        "file to store events, defaults to current directory.");

    parseCommandLine(argc, argv);
}

const std::string &CommandLineLinux::getFile() const {
    return file;
}

std::string CommandLineLinux::platformInformation() {
    struct utsname uts {};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

std::string CommandLineLinux::defaultFile() {
    return std::string();
}
