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
    mouseDeviceOption { "mouse-device", "m", "set mouse device." },
    keyDeviceOption { "keyboard-device", "k", "set keyboard device." },
    touchDeviceOption { "touch-device", "t", "set touch device." },
    listEventDevicesOption { "list-event-devices", "l", "lists available event devices." },
    listEventDevices { false } {

    getDesc().add_options()
        ((get<0>(mouseDeviceOption) + "," + get<1>(mouseDeviceOption)).c_str(),
            po::value<fs::path>(&this->mouseDevice)->default_value(fs::path {}),
            get<2>(mouseDeviceOption).c_str())
        ((get<0>(keyDeviceOption) + "," + get<1>(keyDeviceOption)).c_str(),
            po::value<fs::path>(&this->keyDevice)->default_value(fs::path {}),
            get<2>(keyDeviceOption).c_str())
        ((get<0>(touchDeviceOption) + "," + get<1>(touchDeviceOption)).c_str(),
            po::value<fs::path>(&this->touchDevice)->default_value(fs::path {}),
            get<2>(touchDeviceOption).c_str())
        ((get<0>(listEventDevicesOption) + "," + get<1>(listEventDevicesOption)).c_str(),
            po::bool_switch(&this->listEventDevices),
            get<2>(listEventDevicesOption).c_str());

    parseCommandLine(argc, argv);
}

std::string CommandLineLinux::platformInformation() {
    struct utsname uts {};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

void CommandLineLinux::read_args() {
    CommandLine::read_args();
    checkExclusiveOptions();
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

bool CommandLineLinux::isListEventDevices() const {
    return listEventDevices;
}

void CommandLineLinux::checkExclusiveOptions() {
    po::variables_map vm = getVm();
    if (
        (vm.count(get<0>(listEventDevicesOption))) && !vm[get<0>(listEventDevicesOption)].defaulted() &&
        (
            (vm.count(get<0>(getFileOption())) && !vm[get<0>(getFileOption())].defaulted()) ||
            (vm.count(get<0>(mouseDeviceOption)) && !vm[get<0>(mouseDeviceOption)].defaulted()) ||
            (vm.count(get<0>(keyDeviceOption)) && !vm[get<0>(keyDeviceOption)].defaulted()) ||
            (vm.count(get<0>(touchDeviceOption)) && !vm[get<0>(touchDeviceOption)].defaulted()) ||
            (vm.count(get<0>(getPrintOption())) && !vm[get<0>(getPrintOption())].defaulted())
        )
    ) {
        cout << "The list-event-devices option must be specified alone.\n";
        cout << getDesc() << "\n";
        exit(EXIT_SUCCESS);
    }
    if (!(vm.count(get<0>(listEventDevicesOption))) && !(vm[get<0>(listEventDevicesOption)].defaulted()) &&
        !(vm.count(get<0>(mouseDeviceOption)) && !vm[get<0>(mouseDeviceOption)].defaulted()) &&
        !(vm.count(get<0>(keyDeviceOption)) && !vm[get<0>(keyDeviceOption)].defaulted()) &&
        !(vm.count(get<0>(touchDeviceOption)) && !vm[get<0>(touchDeviceOption)].defaulted())
    ) {
        cout << "At least one event device must be specified.\n";
        cout << getDesc() << "\n";
        exit(EXIT_SUCCESS);
    }
}
