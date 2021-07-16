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
#include "EventDeviceLister.h"
#include "iostream"

using namespace std;

bool EventDevice::operator<(const EventDevice &eventDevice) const {
    return eventNumber < eventDevice.eventNumber;
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
    for (auto device : deviceLister.eventDevices) {
        os << "    " << device.eventNumber << "\n    <- ";
        if (device.byId.has_value()) {
            os << "by-id" << device.byId.value() << "\n    <- ";
        }
        if (device.byPath.has_value()) {
            os << "by-path" << device.byId.value() << "\n";
        }
    }
    return os;
}
