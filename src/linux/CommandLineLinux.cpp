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

#include <linux/CommandLineLinux.h>
#include <string>




CommandLineLinux::CommandLineLinux(int argc1, char *argv1[], const std::string &file, int argc, char **argv)
        : CommandLine(argc1, argv1, file) {

}

bool CommandLineLinux::isListEventDevices() const {
    return false;
}
