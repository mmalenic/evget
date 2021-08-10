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

#ifndef EVGET_INCLUDE_LINUX_SYSTEMEVENTSLINUX_H
#define EVGET_INCLUDE_LINUX_SYSTEMEVENTSLINUX_H

#include <linux/input.h>
#include <filesystem>
#include "SystemEvents.h"

/**
 * Class represents processing linux system events.
 * @tparam T type of events to process
 */
class SystemEventsLinux : public SystemEvents<std::pair<SystemEventDevice, input_event>> {
public:
    /**
     * Create the event processor.
     * @param sendChannel send channel to send events to
     */
    explicit SystemEventsLinux(boost::fibers::buffered_channel<std::pair<SystemEventDevice, input_event>>& sendChannel, std::vector<std::pair<SystemEventDevice, std::filesystem::path>> devices);

    /**
     * Run the event loop for a single device.
     * @return true if successful
     */
    boost::asio::awaitable<void> eventLoopForDevice(SystemEventDevice device, std::filesystem::path path, boost::fibers::buffered_channel<bool>& result);

    boost::asio::awaitable<void> eventLoop() override;

private:
    std::vector<std::pair<SystemEventDevice, std::filesystem::path>> devices;
};

#endif // EVGET_INCLUDE_LINUX_SYSTEMEVENTSLINUX_H
