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

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <EventDeviceLister.h>
#include <iostream>
#include <regex>
#include <fstream>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ULONG_BITS (CHAR_BIT * sizeof (unsigned long))
#define STRINGIFY(x) #x

using namespace std;

namespace algo = boost::algorithm;

bool EventDevice::operator<(const EventDevice &eventDevice) const {
    if ((byId.has_value() && !eventDevice.byId.has_value()) || (byPath.has_value() && !eventDevice.byPath.has_value())) {
        return true;
    }
    if ((!byId.has_value() && eventDevice.byId.has_value()) || (!byPath.has_value() && eventDevice.byPath.has_value())) {
        return false;
    }

    string s1 = algo::to_lower_copy(eventNumber.string());
    string s2 = algo::to_lower_copy(eventDevice.eventNumber.string());

    regex numOrAlpha { R"(\d+|\D+)" };
    string nums = "0123456789";
    auto beginS1 = std::sregex_iterator(s1.begin(), s1.end(), numOrAlpha);
    auto endS1 = std::sregex_iterator();

    auto beginS2 = std::sregex_iterator(s2.begin(), s2.end(), numOrAlpha);
    auto endS2 = std::sregex_iterator();

    for (pair i { beginS1, beginS2 }; i.first != endS1 && i.second != endS2; ++i.first, ++i.second) {
        string matchS1 = ((smatch) *i.first).str();
        string matchS2 = ((smatch) *i.second).str();

        if (matchS1.find_first_of(nums) != string::npos && matchS2.find_first_of(nums) != string::npos) {
            if (stol(matchS1) < stol(matchS2)) {
                return true;
            }
        } else {
            if (matchS1 < matchS2) {
                return true;
            }
        }
    }

    return false;
}

std::ostream &operator<<(ostream &os, const EventDevice &eventDevice) {
    os << "       " << eventDevice.eventNumber.string() << "\n";
    if (eventDevice.byId.has_value()) {
        os << "by-id      <- " << eventDevice.byId.value() << "\n";
    }
    if (eventDevice.byPath.has_value()) {
        os << "by-path    <- " << eventDevice.byPath.value() << "\n";
    }
    return os;
}

EventDeviceLister::EventDeviceLister() :
    inputDirectory { "/dev/input" },
    byId {inputDirectory / "by-id" },
    byPath {inputDirectory / "by-path" },
    sysClass { "/sys/class/input" },
    namePath { "device/name" },
    eventCodeToName { getEventCodeToName() },
    eventDevices { listEventDevices() } { }

vector<EventDevice> EventDeviceLister::listEventDevices() {
    vector<EventDevice> devices {};
    for (auto& entry : fs::directory_iterator(inputDirectory)) {
        if (entry.is_character_file() && entry.path().filename().string().find("event") != string::npos) {
            EventDevice device = {
                entry.path(),
                checkSymlink(entry, byId, "Could not read by-id directory: "),
                checkSymlink(entry, byPath, "Could not read by-path directory: "),
                getName(entry.path()),
                getCapabilities(entry.path())
            };
            devices.push_back(device);
        }
    }
    sort(devices.begin(), devices.end());
    return devices;
}

const vector<EventDevice> &EventDeviceLister::getEventDevices() const {
    return eventDevices;
}

optional<fs::path> EventDeviceLister::checkSymlink(const fs::path &entry, const fs::path &path, const string &msg) noexcept {
    try {
        for (auto& symEntry : fs::directory_iterator(path)) {
            if (symEntry.is_symlink() && fs::read_symlink(symEntry.path()).filename() == entry.filename()) {
                return symEntry.path();
            }
        }
    } catch (fs::filesystem_error& err) {
        cout << msg << err.what() << "\n";
    }
    return {};
}

ostream &operator<<(ostream &os, const EventDeviceLister &deviceLister) {
    if (!deviceLister.eventDevices.empty()) {
        os << "Event devices with symbolic links: \n";
    }
    auto i = deviceLister.eventDevices.begin();
    for (; i != deviceLister.eventDevices.end(); i++) {
        if (!(*i).byId.has_value() && !(*i).byId.has_value()) {
            break;
        }
        os << *i << "\n";
    }
    if (i != deviceLister.eventDevices.end()) {
        os << "Event devices without symbolic links: \n";
    }
    for (; i != deviceLister.eventDevices.end(); i++) {
        os << *i;
    }
    return os;
}

vector<string> EventDeviceLister::getCapabilities(const fs::path &device) {
    unsigned long bit[EV_MAX];

    memset(bit, 0, sizeof(bit));
    int fd = open(device.string().c_str(), O_RDONLY);
    ioctl(fd, EVIOCGBIT(0, EV_MAX), bit);

    vector<string> vec {};
    for (const auto& type : eventCodeToName) {
        if (!!(bit[type.first / ULONG_BITS] & (1uL << (type.first % ULONG_BITS)))) {
            vec.push_back(type.second);
        }
    }

    return vec;
}

string EventDeviceLister::getName(const fs::path &device) {
    fs::path fullPath = sysClass / device.filename() / namePath;
    ifstream file { fullPath };
    string name { (istreambuf_iterator<char>(file)), istreambuf_iterator<char>() };
    name.erase(remove(name.begin(), name.end(), '\n'), name.end());
    return name;
}

map<int, string> EventDeviceLister::getEventCodeToName() {
    return {
        {EV_SYN, STRINGIFY(EV_SYN)},
        {EV_KEY, STRINGIFY(EV_KEY)},
        {EV_REL, STRINGIFY(EV_REL)},
        {EV_ABS, STRINGIFY(EV_ABS)},
        {EV_MSC, STRINGIFY(EV_MSC)},
        {EV_SW, STRINGIFY(EV_SW)},
        {EV_LED, STRINGIFY(EV_LED)},
        {EV_SND, STRINGIFY(EV_SND)},
        {EV_REP, STRINGIFY(EV_REP)},
        {EV_FF, STRINGIFY(EV_FF)},
        {EV_PWR, STRINGIFY(EV_PWR)},
        {EV_FF_STATUS, STRINGIFY(EV_FF_STATUS)},
        {EV_MAX, STRINGIFY(EV_MAX)},
    };
}
