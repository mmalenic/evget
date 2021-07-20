// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "linux/CommandLineLinux.h"

#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <iostream>

using namespace std;

CommandLineLinux::CommandLineLinux(int argc, char** argv) :
    CommandLine{CommandLineLinux::platformInformation()},
    mouseDeviceOption{"mouse-device", "m", "set mouse device."},
    keyDeviceOption{"keyboard-device", "k", "set keyboard device."},
    touchDeviceOption{"touch-device", "t", "set touch device."},
    listEventDevicesOption{"list-event-devices", "l", "lists available event devices."},
    listEventDevices{false} {

    getDesc().add_options()
                 (
                     (get<0>(mouseDeviceOption) + "," + get<1>(mouseDeviceOption)).c_str(),
                     po::value<fs::path>(&this->mouseDevice)->default_value(fs::path{}),
                     get<2>(mouseDeviceOption).c_str()
                 )
                 (
                     (get<0>(keyDeviceOption) + "," + get<1>(keyDeviceOption)).c_str(),
                     po::value<fs::path>(&this->keyDevice)->default_value(fs::path{}),
                     get<2>(keyDeviceOption).c_str()
                 )
                 (
                     (get<0>(touchDeviceOption) + "," + get<1>(touchDeviceOption)).c_str(),
                     po::value<fs::path>(&this->touchDevice)->default_value(fs::path{}),
                     get<2>(touchDeviceOption).c_str()
                 )
                 (
                     (get<0>(listEventDevicesOption) + "," + get<1>(listEventDevicesOption)).c_str(),
                     po::bool_switch(&this->listEventDevices),
                     get<2>(listEventDevicesOption).c_str()
                 );

    parseCommandLine(argc, argv);
}

std::string CommandLineLinux::platformInformation() {
    struct utsname uts{};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

void CommandLineLinux::read_args() {
    CommandLine::read_args();
    checkExclusiveOptions();
}

const fs::path& CommandLineLinux::getMouseDevice() const {
    return mouseDevice;
}

const fs::path& CommandLineLinux::getKeyDevice() const {
    return keyDevice;
}

const fs::path& CommandLineLinux::getTouchDevice() const {
    return touchDevice;
}

bool CommandLineLinux::isListEventDevices() const {
    return listEventDevices;
}

void CommandLineLinux::checkExclusiveOptions() {
    po::variables_map vm = getVm();
    if ((vm.count(get<0>(listEventDevicesOption))) && !vm[get<0>(listEventDevicesOption)].defaulted()
        && ((vm.count(get<0>(getFileOption())) && !vm[get<0>(getFileOption())].defaulted())
            || (vm.count(get<0>(mouseDeviceOption)) && !vm[get<0>(mouseDeviceOption)].defaulted())
            || (vm.count(get<0>(keyDeviceOption)) && !vm[get<0>(keyDeviceOption)].defaulted())
            || (vm.count(get<0>(touchDeviceOption)) && !vm[get<0>(touchDeviceOption)].defaulted())
            || (vm.count(get<0>(getPrintOption())) && !vm[get<0>(getPrintOption())].defaulted()))) {
        cout << "The list-event-devices option must be specified alone.\n";
        cout << getDesc() << "\n";
        exit(EXIT_SUCCESS);
    }
    if (vm[get<0>(listEventDevicesOption)].defaulted() && vm[get<0>(mouseDeviceOption)].defaulted()
        && vm[get<0>(keyDeviceOption)].defaulted() && vm[get<0>(touchDeviceOption)].defaulted()) {
        cout << "At least one event device must be specified.\n";
        cout << getDesc() << "\n";
        exit(EXIT_SUCCESS);
    }
}
