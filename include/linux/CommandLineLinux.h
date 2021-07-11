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

#include "../include/CommandLine.h"

class CommandLineLinux : public CommandLine {
private:
    bool listEventDevices{};
public:

    /**
     * Create the command line object.
     *
     * @param argc from main
     * @param argv from main
     */
    CommandLineLinux(int argc, char *argv[]);

    /**
     * get the list event devices flag.
     * @return list event devices flag
     */
    bool isListEventDevices() const;
};

#endif //INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
