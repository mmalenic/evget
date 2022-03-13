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

#include "../include/checkinput/EventDeviceLister.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST(EventDeviceListerTest, ElevatedContainsAllDevices) { // NOLINT(cert-err58-cpp)
    std::vector<CheckInput::EventDevice> devices = CheckInput::EventDeviceLister{}.listEventDevices();

    std::vector<bool> results {};
    for (auto& entry : fs::directory_iterator("/dev/input")) {
        if (entry.is_character_file() && entry.path().filename().string().find("event") != std::string::npos) {
            auto i{devices.begin()};
            for (; i != devices.end(); ++i) {
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

TEST(EventDeviceListerTest, ElevatedContainsAllIdSymlinks) { // NOLINT(cert-err58-cpp)
    std::vector<CheckInput::EventDevice> devices = CheckInput::EventDeviceLister{}.listEventDevices();

    std::vector<bool> results {};
    for (auto& device : devices) {
        if (device.getById().has_value()) {
            fs::path path {device.getById().value()};
            results.push_back(fs::is_symlink(path) && fs::read_symlink(path).filename() == device.getDevice().filename());
        }
    }
    ASSERT_TRUE(all_of(results.begin(), results.end(), [](bool v) { return v; }));
}

TEST(EventDeviceListerTest, ElevatedContainsAllPathSymlinks) { // NOLINT(cert-err58-cpp)
    std::vector<CheckInput::EventDevice> devices = CheckInput::EventDeviceLister{}.listEventDevices();

    std::vector<bool> results {};
    for (auto& device : devices) {
        if (device.getByPath().has_value()) {
            fs::path path {device.getByPath().value()};
            results.push_back(fs::is_symlink(path) && fs::read_symlink(path).filename() == device.getDevice().filename());
        }
    }
    ASSERT_TRUE(all_of(results.begin(), results.end(), [](bool v) { return v; }));
}