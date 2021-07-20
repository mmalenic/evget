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

#include "EventDevice.h"

#include <boost/algorithm/string.hpp>
#include <utility>
#include <regex>

using namespace std;

namespace algorithm = boost::algorithm;

static constexpr size_t MIN_SPACE_GAP = 4;
static constexpr size_t SPACE_FOR_SYMLINK = 4;
static constexpr char BY_ID[] = "by-id";
static constexpr char BY_PATH[] = "by-path";

size_t EventDevice::maxNameSize = 0;
size_t EventDevice::maxPathSize = 0;

const fs::path& EventDevice::getDevice() const {
    return device;
}

EventDevice::EventDevice(
    fs::path device,
    const optional<string>& byId,
    const optional<string>& byPath,
    const optional<string>& name,
    const vector<string>& capabilities
) : device{move(device)}, byId{byId}, byPath{byPath}, name{name}, capabilities{capabilities} {
}

const optional<string>& EventDevice::getById() const {
    return byId;
}

const std::optional<string>& EventDevice::getByPath() const {
    return byPath;
}

const std::optional<string>& EventDevice::getName() const {
    return name;
}

const std::vector<string>& EventDevice::getCapabilities() const {
    return capabilities;
}

size_t EventDevice::getMaxNameSize() {
    return maxNameSize;
}

void EventDevice::setMaxNameSize(size_t newMaxNameSize) {
    EventDevice::maxNameSize = newMaxNameSize;
}

size_t EventDevice::getMaxPathSize() {
    return maxPathSize;
}

void EventDevice::setMaxPathSize(size_t newMaxPathSize) {
    EventDevice::maxPathSize = newMaxPathSize;
}

partial_ordering EventDevice::operator<=>(const EventDevice& eventDevice) const {
    if ((byId.has_value() && !eventDevice.byId.has_value())
        || (byPath.has_value() && !eventDevice.byPath.has_value())) {
        return partial_ordering::less;
    }
    if ((!byId.has_value() && eventDevice.byId.has_value()) ||
        (!byPath.has_value() && eventDevice.byPath.has_value())) {
        return partial_ordering::greater;
    }
    string s1 = algorithm::to_lower_copy(device.string());
    string s2 = algorithm::to_lower_copy(eventDevice.device.string());

    regex numOrAlpha{R"(\d+|\D+)"};
    string nums = "0123456789";
    auto beginS1 = sregex_iterator(s1.begin(), s1.end(), numOrAlpha);
    auto endS1 = sregex_iterator();

    auto beginS2 = std::sregex_iterator(s2.begin(), s2.end(), numOrAlpha);
    auto endS2 = std::sregex_iterator();

    for (pair i{beginS1, beginS2}; i.first != endS1 && i.second != endS2; ++i.first, ++i.second) {
        string matchS1 = ((smatch) *i.first).str();
        string matchS2 = ((smatch) *i.second).str();

        if (matchS1.find_first_of(nums) != string::npos && matchS2.find_first_of(nums) != string::npos) {
            if (stol(matchS1) < stol(matchS2)) {
                return partial_ordering::less;
            }
        } else {
            if (matchS1 < matchS2) {
                return partial_ordering::less;
            }
        }
    }

    return partial_ordering::unordered;
}

ostream& operator<<(ostream& os, const EventDevice& eventDevice) {
    size_t deviceNameLength = 0;
    size_t devicePathLength = eventDevice.device.string().length();
    if (eventDevice.name.has_value()) {
        os << eventDevice.name.value();
        deviceNameLength = eventDevice.name->length();
    }
    auto totalName = MIN_SPACE_GAP + EventDevice::maxNameSize;
    auto spacesName = totalName - deviceNameLength;
    auto spacesPath = (MIN_SPACE_GAP + EventDevice::maxPathSize) - devicePathLength;
    os << string(spacesName, ' ') << eventDevice.device.string() << string(spacesPath, ' ');

    if (!eventDevice.capabilities.empty()) {
        os << "capabilities = [";
        for (auto i{eventDevice.capabilities.begin()}; i != --eventDevice.capabilities.end(); ++i) {
            os << *i << ", ";
        }
        os << eventDevice.capabilities.back() << "]";
    }
    os << "\n";

    if (eventDevice.byId.has_value()) {
        auto spacesById = totalName - (sizeof(BY_ID) / sizeof(*BY_ID)) + SPACE_FOR_SYMLINK;
        os << BY_ID << string(spacesById, ' ') << "<- " << eventDevice.byId.value() << "\n";
    }
    if (eventDevice.byPath.has_value()) {
        auto spacesByPath = totalName - (sizeof(BY_PATH) / sizeof(*BY_PATH)) + SPACE_FOR_SYMLINK;
        os << BY_PATH << string(spacesByPath, ' ') << "<- " << eventDevice.byPath.value() << "\n";
    }
    return os;
}