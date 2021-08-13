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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
#define EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H

#include <linux/input.h>
#include <filesystem>
#include "SystemEventLoop.h"
#include "SystemEvent.h"

/**
 * Class represents processing linux system events.
 * @tparam T type of events to process
 */
class SystemEventLoopLinux : public SystemEventLoop<input_event> {
public:
    /**
     * Create the system events.
     *
     * @param mouseDevices mouse devices
     * @param keyDevices key devices
     * @param touchDevices touch devices
     */
    SystemEventLoopLinux(std::vector<std::filesystem::path> mouseDevices, std::vector<std::filesystem::path> keyDevices, std::vector<std::filesystem::path> touchDevices, EventHandler<input_event>& eventHandler);

    /**
     * Run the event loop for a single type.
     */
    boost::asio::awaitable<bool> eventLoopForDevice(SystemEvent<input_event>::Type type, std::filesystem::path path);

    /**
     * Run the event loop for each set of devices.
     */
    boost::asio::awaitable<void> eventLoopForEach(SystemEvent<input_event>::Type type, std::vector<std::filesystem::path> paths);

    boost::asio::awaitable<void> eventLoop() override;

private:
    std::vector<std::filesystem::path> mouseDevices;
    std::vector<std::filesystem::path> keyDevices;
    std::vector<std::filesystem::path> touchDevices;
};

#endif // EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
