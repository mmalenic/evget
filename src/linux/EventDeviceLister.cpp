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

using namespace std;

namespace algo = boost::algorithm;

bool EventDevice::operator<(const EventDevice &eventDevice) const {
    if (byId.has_value() && !eventDevice.byId.has_value()) {
        return true;
    }
    if (!byId.has_value() && eventDevice.byId.has_value()) {
        return false;
    }
    if (byPath.has_value() && !eventDevice.byPath.has_value()) {
        return true;
    }
    if (!byPath.has_value() && eventDevice.byPath.has_value()) {
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
    by_id { inputDirectory / "by-id" },
    by_path { inputDirectory / "by-path" },
    eventDevices { listEventDevices() } { }

vector<EventDevice> EventDeviceLister::listEventDevices() {
    vector<EventDevice> devices {};
    for (auto& entry : fs::directory_iterator(inputDirectory)) {
        if (entry.is_character_file() && entry.path().filename().string().find("event") != string::npos) {
            EventDevice device = {
                entry.path(),
                checkSymlink(entry, by_id, "Could not read by-id directory: "),
                checkSymlink(entry, by_path, "Could not read by-path directory: ")
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

std::ostream &operator<<(ostream &os, const EventDeviceLister &deviceLister) {
    if (!deviceLister.eventDevices.empty()) {
        os << "Event devices with symbolic links: \n";
    }
    auto i = deviceLister.eventDevices.begin();
    for (; i != deviceLister.eventDevices.end(); i++) {
        os << *i << "\n";
        if (!(*i).byId.has_value() && !(*i).byId.has_value()) {
            break;
        }
    }
    if (i != deviceLister.eventDevices.end()) {
        os << "Event devices without symbolic links: \n";
    }
    for (; i != deviceLister.eventDevices.end(); i++) {
        os << *i;
    }
    return os;
}
