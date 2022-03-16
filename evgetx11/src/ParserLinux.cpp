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

#include "evgetx11/ParserLinux.h"

#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <iostream>

constexpr char MOUSE_DEVICES_NAME[] = "mouse-devices";
constexpr char KEY_DEVICES_NAME[] = "key-devices";
constexpr char TOUCH_DEVICES_NAME[] = "touch-devices";
constexpr char LIST_DEVICES_NAME[] = "list-devices";

CliOption::ParserLinux::ParserLinux() :
    Parser{platformInformation()},
    mouseDevices{
        OptionBuilder<std::vector<fs::path>>{this->getCmdlineDesc()}
            .shortName('m')
            .longName(MOUSE_DEVICES_NAME)
            .description("Set mouse devices.")
            .defaultValue({})
            .atLeast({KEY_DEVICES_NAME, TOUCH_DEVICES_NAME}, {LIST_DEVICES_NAME})
            .build()
    },
    keyDevices{
        OptionBuilder<std::vector<fs::path>>{this->getCmdlineDesc()}
            .shortName('k')
            .longName(KEY_DEVICES_NAME)
            .description("Set key devices.")
            .defaultValue({})
            .atLeast({MOUSE_DEVICES_NAME, TOUCH_DEVICES_NAME}, {LIST_DEVICES_NAME})
            .build()
    },
    touchDevices{
        OptionBuilder<std::vector<fs::path>>{this->getCmdlineDesc()}
        .shortName('t')
        .longName(TOUCH_DEVICES_NAME)
        .description("Set touch devices.")
        .defaultValue({})
        .atLeast({MOUSE_DEVICES_NAME, KEY_DEVICES_NAME}, {LIST_DEVICES_NAME})
        .build()
    },
    listEventDevices{
        OptionBuilder<bool>{this->getCmdlineDesc()}
        .shortName('l')
        .longName(LIST_DEVICES_NAME)
        .description("list possible devices.")
        .conflictsWith({MOUSE_DEVICES_NAME, KEY_DEVICES_NAME, TOUCH_DEVICES_NAME})
        .buildFlag()
    } {
}

std::string CliOption::ParserLinux::platformInformation() {
    struct utsname uts{};
    if ((uname(&uts)) == -1) {
        return "unknown";
    }
    return uts.sysname;
}

std::vector<CliOption::fs::path> CliOption::ParserLinux::getMouseDevices() const {
    return mouseDevices.getValue();
}

std::vector<CliOption::fs::path> CliOption::ParserLinux::getKeyDevices() const {
    return keyDevices.getValue();
}

std::vector<CliOption::fs::path> CliOption::ParserLinux::getTouchDevices() const {
    return touchDevices.getValue();
}

bool CliOption::ParserLinux::isListEventDevices() const {
    return listEventDevices.getValue();
}

bool CliOption::ParserLinux::parseFileAndCmdlineOptions() {
    if (!Parser::parseFileAndCmdlineOptions()) {
        return false;
    }

    mouseDevices.run(this->getVm());
    keyDevices.run(this->getVm());
    touchDevices.run(this->getVm());
    listEventDevices.run(this->getVm());

    return true;
}

