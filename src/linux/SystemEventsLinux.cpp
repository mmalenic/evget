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

#include "SystemEventsLinux.h"

#include <utility>

namespace asio = boost::asio;
namespace fibers = boost::fibers;

using namespace std;

SystemEventsLinux::SystemEventsLinux(
    boost::fibers::buffered_channel<std::pair<SystemEventDevice, input_event>>& sendChannel,
    std::vector<std::pair<SystemEventDevice, std::filesystem::path>> devices
) : SystemEvents(sendChannel), devices{std::move(devices)} {
}

boost::asio::awaitable<void> SystemEventsLinux::eventLoopForDevice(SystemEventDevice device, std::filesystem::path path, fibers::buffered_channel<bool>& result) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        string err = strerror(errno);
        spdlog::warn("Could not open device to read events: " + err);
        result.try_push(false);
        co_return;
    }

    asio::posix::stream_descriptor stream{getContext(), fd};
    while (!isCancelled()) {
        struct input_event event;
        memset(&event, 0, sizeof(event));
        co_await stream.async_read_some(&event);

        auto result = sendChannel.try_push(make_pair(device, event));
        if (result == fibers::channel_op_status::full) {
            spdlog::warn("Channel is full, losing event, consider increasing buffer size.");
        }
    }

    stream.cancel();
    stream.close();
    result.try_push(true);
    co_return;
}

boost::asio::awaitable<void> SystemEventsLinux::eventLoop() {
    fibers::buffered_channel<bool> results{devices.size + 1};
    for (auto device : devices) {
        co_spawn(getContext(), eventLoopForDevice(device.first, device.second), boost::asio::detached);
    }
    bool first = co_await results.pop();
    if (none_of(results.begin(), results.end(), [](bool v)
                { return v; }))
    {
        throw UnsupportedOperationException("No devices were set for reading input events.");
    }
}
