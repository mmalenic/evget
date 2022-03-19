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

#include "listinputdevices/InputDeviceLister.h"
#include "ListInputDevicesTestUtils.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST(InputDeviceListerTest, ElevatedContainsAllDevices) { // NOLINT(cert-err58-cpp)
    std::vector<ListInputDevices::InputDevice> devices = ListInputDevices::InputDeviceLister{}.listInputDevices();

    std::vector<bool> results {};
    for (auto& entry : fs::directory_iterator("/dev/input")) {
        if (entry.is_character_file() && entry.path().filename().string().find("event") != std::string::npos) {
            auto i{devices.begin()};
            for (; i != devices.end(); i++) {
                if ((*i).getDevice().string() == entry.path().string()) {
                    results.push_back(true);
                    break;
                }
            }
            if (i == devices.end()) {
                results.push_back(false);
            }
        }
    }
    ASSERT_TRUE(all_of(results.begin(), results.end(), [](bool v) { return v; }));
}

TEST(InputDeviceListerTest, ElevatedContainsAllIdSymlinks) { // NOLINT(cert-err58-cpp)
    ListInputDeviceTestUtils::checkDevices([](auto& device) {
        return device.getById();
    });
}

TEST(InputDeviceListerTest, ElevatedContainsAllPathSymlinks) { // NOLINT(cert-err58-cpp)
    ListInputDeviceTestUtils::checkDevices([](auto& device) {
        return device.getByPath();
    });
}