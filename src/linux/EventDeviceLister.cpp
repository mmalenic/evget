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

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <EventDevice.h>
#include <EventDeviceLister.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/input.h>
#include <regex>
#include <spdlog/spdlog.h>

#define ULONG_BITS   (CHAR_BIT * sizeof(unsigned long))
#define STRINGIFY(x) #x

using namespace std;

EventDeviceLister::EventDeviceLister() :
    inputDirectory{"/dev/input"},
    byId{inputDirectory / "by-id"},
    byPath{inputDirectory / "by-path"},
    sysClass{"/sys/class/input"},
    namePath{"device/name"},
    eventCodeToName{getEventCodeToName()},
    maxNameSize{0},
    maxPathSize{0},
    eventDevices{listEventDevices()} {
}

vector<EventDevice> EventDeviceLister::listEventDevices() {
    vector<EventDevice> devices{};
    for (auto& entry : fs::directory_iterator(inputDirectory)) {
        if (entry.is_character_file() && entry.path().filename().string().find("event") != string::npos) {
            auto name = getName(entry.path());
            EventDevice device = {
                entry.path(),
                checkSymlink(entry, byId, "Could not read by-id directory: "),
                checkSymlink(entry, byPath, "Could not read by-path directory: "),
                getName(entry.path()),
                getCapabilities(entry.path())
            };

            if (name.length() > EventDevice::getMaxNameSize()) {
                EventDevice::setMaxNameSize(name.length());
            }
            if (entry.path().string().length() > EventDevice::getMaxPathSize()) {
                EventDevice::setMaxPathSize(entry.path().string().length());
            }

            devices.push_back(device);
        }
    }
    sort(devices.begin(), devices.end());

    return devices;
}

const vector<EventDevice>& EventDeviceLister::getEventDevices() const {
    return eventDevices;
}

optional<fs::path>
EventDeviceLister::checkSymlink(const fs::path& entry, const fs::path& path, const string& msg) noexcept {
    try {
        for (auto& symEntry : fs::directory_iterator(path)) {
            if (symEntry.is_symlink() && fs::read_symlink(symEntry.path()).filename() == entry.filename()) {
                return symEntry.path();
            }
        }
    } catch (fs::filesystem_error& err) {
        spdlog::warn(msg + err.what());
    }
    return {};
}

ostream& operator<<(ostream& os, const EventDeviceLister& deviceLister) {
    if (!deviceLister.eventDevices.empty()) {
        os << "Event devices with path or id symbolic links: \n";
    }
    auto i = deviceLister.eventDevices.begin();
    for (; i != deviceLister.eventDevices.end(); i++) {
        if (!(*i).getById().has_value() && !(*i).getById().has_value()) {
            break;
        }
        os << *i << "\n";
    }
    if (i != deviceLister.eventDevices.end()) {
        os << "Event devices without path or id symbolic links: \n";
    }
    for (; i != deviceLister.eventDevices.end(); i++) {
        os << *i;
    }
    return os;
}

vector<string> EventDeviceLister::getCapabilities(const fs::path& device) {
    unsigned long bit[EV_MAX];

    memset(bit, 0, sizeof(bit));
    int fd = open(device.string().c_str(), O_RDONLY);

    if (fd == -1) {
        string err = strerror(errno);
        spdlog::warn("Could not open device to read capabilities: " + err);
        return {};
    }

    ioctl(fd, EVIOCGBIT(0, EV_MAX), &bit);

    vector<string> vec{};
    for (const auto& type : eventCodeToName) {
        if (!!(bit[type.first / ULONG_BITS] & (1uL << (type.first % ULONG_BITS)))) {
            vec.push_back(type.second);
        }
    }

    close(fd);
    return vec;
}

string EventDeviceLister::getName(const fs::path& device) {
    fs::path fullPath = sysClass / device.filename() / namePath;
    ifstream file{fullPath};
    string name{(istreambuf_iterator<char>(file)), istreambuf_iterator<char>()};
    name.erase(remove(name.begin(), name.end(), '\n'), name.end());

    if (name.length() > maxNameSize) {
        maxNameSize = name.length();
    }

    return name;
}

map<int, string> EventDeviceLister::getEventCodeToName() {
    return {
        {EV_SYN,       STRINGIFY(EV_SYN)},
        {EV_KEY,       STRINGIFY(EV_KEY)},
        {EV_REL,       STRINGIFY(EV_REL)},
        {EV_ABS,       STRINGIFY(EV_ABS)},
        {EV_MSC,       STRINGIFY(EV_MSC)},
        {EV_SW,        STRINGIFY(EV_SW)},
        {EV_LED,       STRINGIFY(EV_LED)},
        {EV_SND,       STRINGIFY(EV_SND)},
        {EV_REP,       STRINGIFY(EV_REP)},
        {EV_FF,        STRINGIFY(EV_FF)},
        {EV_PWR,       STRINGIFY(EV_PWR)},
        {EV_FF_STATUS, STRINGIFY(EV_FF_STATUS)},
        {EV_MAX,       STRINGIFY(EV_MAX)},
    };
}
