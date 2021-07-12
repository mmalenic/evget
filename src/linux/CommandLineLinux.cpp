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

#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <iostream>

using namespace std;

CommandLineLinux::CommandLineLinux(int argc, char **argv) :
    CommandLine { CommandLineLinux::platformInformation() },
    fileOption {"file", "f"},
    mouseDeviceOption {"mouse-device", "m"},
    keyDeviceOption {"keyboard-device", "k"},
    touchDeviceOption {"touch-device", "t"},
    listEventDevicesOption {"list-event-devices", "l"},
    listEventDevices { false } {

    getDesc().add_options()
        ((fileOption.first + "," + fileOption.second).c_str(),
            po::value<fs::path>(&this->file)->default_value(defaultFile()),
            "file to store events, defaults to current directory.")
        ((mouseDeviceOption.first + "," + mouseDeviceOption.second).c_str(),
            po::value<fs::path>(&this->mouseDevice)->default_value(fs::path {}),
            "set mouse device.")
        ((keyDeviceOption.first + "," + keyDeviceOption.second).c_str(),
            po::value<fs::path>(&this->keyDevice)->default_value(fs::path {}),
            "set keyboard device.")
        ((touchDeviceOption.first + "," + touchDeviceOption.second).c_str(),
            po::value<fs::path>(&this->touchDevice)->default_value(fs::path {}),
            "set touch device.")
        ((listEventDevicesOption.first + "," + listEventDevicesOption.second).c_str(),
            po::value<bool>(&this->listEventDevices)->default_value(false),
            "lists available event devices.");

    checkListDevicesExclusive();
    parseCommandLine(argc, argv);
}

const fs::path &CommandLineLinux::getFile() const {
    return file;
}

std::string CommandLineLinux::platformInformation() {
    struct utsname uts {};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

const fs::path &CommandLineLinux::getMouseDevice() const {
    return mouseDevice;
}

const fs::path &CommandLineLinux::getKeyDevice() const {
    return keyDevice;
}

const fs::path &CommandLineLinux::getTouchDevice() const {
    return touchDevice;
}

void CommandLineLinux::checkListDevicesExclusive() {
    po::variables_map vm = getVm();
    if (
        (vm.count(listEventDevicesOption.first)) && !(vm[listEventDevicesOption.first].defaulted()) &&
        (
            (vm.count(fileOption.first) && !(vm[fileOption.first].defaulted())) ||
            (vm.count(mouseDeviceOption.first) && !(vm[mouseDeviceOption.first].defaulted())) ||
            (vm.count(keyDeviceOption.first) && !(vm[keyDeviceOption.first].defaulted())) ||
            (vm.count(touchDeviceOption.first) && !(vm[touchDeviceOption.first].defaulted()))
        )
    ) {
        cout << "The list-event-devices option must be specified alone\n";
        exit(EXIT_SUCCESS);
    }
}

bool CommandLineLinux::isListEventDevices() const {
    return listEventDevices;
}
