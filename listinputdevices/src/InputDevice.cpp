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

#include "listinputdevices/InputDevice.h"

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <regex>
#include <utility>

namespace algorithm = boost::algorithm;

static constexpr size_t MIN_SPACE_GAP = 4;
static constexpr size_t SPACE_FOR_SYMLINK = 2;
static constexpr char BY_ID[] = "by-id";
static constexpr char BY_PATH[] = "by-path";

size_t ListInputDevices::InputDevice::maxNameSize = 0;
size_t ListInputDevices::InputDevice::maxPathSize = 0;

const ListInputDevices::fs::path& ListInputDevices::InputDevice::getDevice() const {
    return device;
}

ListInputDevices::InputDevice::InputDevice(
    fs::path device,
    std::optional<std::string> byId,
    std::optional<std::string> byPath,
    std::optional<std::string> name,
    std::vector<std::pair<int, std::string>> capabilities
)
    : device{std::move(device)},
      byId{std::move(byId)},
      byPath{std::move(byPath)},
      name{std::move(name)},
      capabilities{std::move(capabilities)} {}

const std::optional<std::string>& ListInputDevices::InputDevice::getById() const {
    return byId;
}

const std::optional<std::string>& ListInputDevices::InputDevice::getByPath() const {
    return byPath;
}

const std::optional<std::string>& ListInputDevices::InputDevice::getName() const {
    return name;
}

const std::vector<std::pair<int, std::string>>& ListInputDevices::InputDevice::getCapabilities() const {
    return capabilities;
}

size_t ListInputDevices::InputDevice::getMaxNameSize() {
    return maxNameSize;
}

void ListInputDevices::InputDevice::setMaxNameSize(size_t newMaxNameSize) {
    InputDevice::maxNameSize = newMaxNameSize;
}

size_t ListInputDevices::InputDevice::getMaxPathSize() {
    return maxPathSize;
}

void ListInputDevices::InputDevice::setMaxPathSize(size_t newMaxPathSize) {
    InputDevice::maxPathSize = newMaxPathSize;
}

std::partial_ordering ListInputDevices::InputDevice::operator<=>(const InputDevice& eventDevice) const {
    if ((byId.has_value() && !eventDevice.byId.has_value()) ||
        (byPath.has_value() && !eventDevice.byPath.has_value())) {
        return std::partial_ordering::less;
    }
    if ((!byId.has_value() && eventDevice.byId.has_value()) ||
        (!byPath.has_value() && eventDevice.byPath.has_value())) {
        return std::partial_ordering::greater;
    }
    std::string s1 = algorithm::to_lower_copy(device.string());
    std::string s2 = algorithm::to_lower_copy(eventDevice.device.string());

    std::regex numOrAlpha{R"(\d+|\D+)"};
    std::string nums = "0123456789";
    auto beginS1 = std::sregex_iterator(s1.begin(), s1.end(), numOrAlpha);
    auto endS1 = std::sregex_iterator();

    auto beginS2 = std::sregex_iterator(s2.begin(), s2.end(), numOrAlpha);
    auto endS2 = std::sregex_iterator();

    for (std::pair i{beginS1, beginS2}; i.first != endS1 && i.second != endS2; ++i.first, ++i.second) {
        std::string matchS1 = i.first->str();
        std::string matchS2 = i.second->str();

        if ((matchS1.find_first_of(nums) != std::string::npos && matchS2.find_first_of(nums) != std::string::npos &&
             stol(matchS1) < stol(matchS2)) ||
            ((matchS1 < matchS2))) {
            return std::partial_ordering::less;
        }
    }

    return std::partial_ordering::unordered;
}

std::ostream& ListInputDevices::operator<<(std::ostream& os, const ListInputDevices::InputDevice& eventDevice) {
    size_t deviceNameLength = 0;
    size_t devicePathLength = eventDevice.device.string().length();
    if (eventDevice.name.has_value()) {
        os << eventDevice.name.value();
        deviceNameLength = eventDevice.name->length();
    }
    auto totalName = MIN_SPACE_GAP + ListInputDevices::InputDevice::maxNameSize;
    auto spacesName = totalName - deviceNameLength;
    auto spacesPath = (MIN_SPACE_GAP + ListInputDevices::InputDevice::maxPathSize) - devicePathLength;
    os << std::string(spacesName, ' ') << eventDevice.device.string() << std::string(spacesPath, ' ');

    if (!eventDevice.capabilities.empty()) {
        os << "capabilities = [";
        for (auto i{eventDevice.capabilities.begin()}; i != --eventDevice.capabilities.end(); ++i) {
            os << "(" << i->first << "; " << i->second << "), ";
        }
        os << "(" << eventDevice.capabilities.back().first << "; " << eventDevice.capabilities.back().second << ")]";
    }
    os << "\n";

    if (eventDevice.byId.has_value()) {
        auto spacesById = totalName - (sizeof(BY_ID) / sizeof(*BY_ID)) + SPACE_FOR_SYMLINK;
        os << "  " << BY_ID << std::string(spacesById, ' ') << "<- " << eventDevice.byId.value() << "\n";
    }
    if (eventDevice.byPath.has_value()) {
        auto spacesByPath = totalName - (sizeof(BY_PATH) / sizeof(*BY_PATH)) + SPACE_FOR_SYMLINK;
        os << "  " << BY_PATH << std::string(spacesByPath, ' ') << "<- " << eventDevice.byPath.value() << "\n";
    }
    return os;
}