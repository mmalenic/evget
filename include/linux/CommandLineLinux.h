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

#ifndef INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
#define INPUT_EVENT_RECORDER_COMMANDLINELINUX_H

#include <string>
#include <CommandLine.h>

class CommandLineLinux : public CommandLine {
public:
    /**
     * Create a CommandLine object.
     *
     * @param argc from main
     * @param argv from main
     */
    CommandLineLinux(int argc, char *argv[]);

    std::string platformInformation() override;

    const fs::path &getFile() const override;

private:
    fs::path file;
};


#endif //INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
