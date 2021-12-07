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

#include "CommandLine/ParserLinux.h"

#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <iostream>

using namespace std;

CommandLine::ParserLinux::ParserLinux(int argc, char** argv) :
    Parser{platformInformation()} {
}

std::string CommandLine::ParserLinux::platformInformation() {
    struct utsname uts{};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

//void ParserLinux::readArgs() {
//    CommandLine::simpleArgs();
//    CommandLine::readArgs();
//    validateArgs();
//}
//
//const vector<fs::path>& ParserLinux::getMouseDevices() const {
//    return mouseDevice;
//}
//
//const vector<fs::path>& ParserLinux::getKeyDevices() const {
//    return keyDevice;
//}
//
//const vector<fs::path>& ParserLinux::getTouchDevices() const {
//    return touchDevice;
//}
//
//bool ParserLinux::isListEventDevices() const {
//    return listEventDevices;
//}
//
//void ParserLinux::validateArgs() {
//    po::variables_map vm = getVm();
//    if ((vm.count(get<0>(listEventDevicesOption))) && !vm[get<0>(listEventDevicesOption)].defaulted()
//        && ((vm.count(get<0>(getFileOption())) && !vm[get<0>(getFileOption())].defaulted())
//            || (vm.count(get<0>(mouseDeviceOption)) && !vm[get<0>(mouseDeviceOption)].defaulted())
//            || (vm.count(get<0>(keyDeviceOption)) && !vm[get<0>(keyDeviceOption)].defaulted())
//            || (vm.count(get<0>(touchDeviceOption)) && !vm[get<0>(touchDeviceOption)].defaulted())
//            || (vm.count(get<0>(getPrintOption())) && !vm[get<0>(getPrintOption())].defaulted()))) {
//        cout << "The list-event-devices option can only be specified with non-conflicting options.\n";
//        cout << getDesc() << "\n";
//        exit(EXIT_SUCCESS);
//    }
//    if (vm[get<0>(listEventDevicesOption)].defaulted() && !vm.count(get<0>(mouseDeviceOption))
//        && !vm.count(get<0>(keyDeviceOption)) && !vm.count(get<0>(touchDeviceOption))) {
//        cout << "At least one event device must be specified.\n";
//        cout << getDesc() << "\n";
//        exit(EXIT_SUCCESS);
//    }
//}
