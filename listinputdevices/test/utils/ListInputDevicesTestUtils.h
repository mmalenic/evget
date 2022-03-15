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

#ifndef EVGET_CHECKINPUT_TEST_UTILS_CHECKINPUTTESTUTILS_H
#define EVGET_CHECKINPUT_TEST_UTILS_CHECKINPUTTESTUTILS_H

#include "listinputdevices/InputDevice.h"
#include "listinputdevices/InputDeviceLister.h"
#include <gtest/gtest.h>

namespace ListInputDeviceTestUtils {

    namespace fs = std::filesystem;

    void checkDevices(auto&& getDevice) {
        std::vector<ListInputDevices::InputDevice> devices = ListInputDevices::InputDeviceLister{}.listInputDevices();

        std::vector<bool> results{};
        for (auto& device : devices) {
            auto name = getDevice(device);
            if (name.has_value()) {
                fs::path path{name.value()};
                results.push_back(fs::is_symlink(path) && fs::read_symlink(path).filename() == device.getDevice().filename());
            }
        }
        ASSERT_TRUE(all_of(results.begin(), results.end(), [](bool v) { return v; }));
    }

    std::vector<std::pair<int, std::string>> createCapabilities();
}

#endif //EVGET_CHECKINPUT_TEST_UTILS_CHECKINPUTTESTUTILS_H
