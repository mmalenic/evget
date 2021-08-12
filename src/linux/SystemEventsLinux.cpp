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
    std::vector<std::filesystem::path>  mouseDevices,
    std::vector<std::filesystem::path>  keyDevices,
    std::vector<std::filesystem::path>  touchDevices
    ) : SystemEvents{mouseDevices.size() + keyDevices.size() + touchDevices.size()}, mouseDevices{std::move(mouseDevices)}, keyDevices{std::move(keyDevices)}, touchDevices{std::move(touchDevices)} {
}

boost::asio::awaitable<bool> SystemEventsLinux::eventLoopForDevice(SystemEventDevice<input_event>::Value device, std::filesystem::path path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        string err = strerror(errno);
        spdlog::warn("Could not open device to read events: " + err);
        co_return false;
    }

//    asio::posix::stream_descriptor stream{getContext(), fd};
//    while (!isCancelled()) {
//        struct input_event event;
//        memset(&event, 0, sizeof(event));
//        co_await stream.async_read_some(&event);
//
//        auto eventValue = SystemEventDevice(device, event);
//        auto result = sendChannel.try_push(make_pair(device, event));
//        if (result == fibers::channel_op_status::full) {
//            spdlog::warn("Channel is full, losing event, consider increasing buffer size.");
//        }
//    }
//
//    stream.cancel();
//    stream.close();
    co_return true;
}

boost::asio::awaitable<void> SystemEventsLinux::eventLoop() {
    eventLoopForEach(SystemEventDevice<input_event>::mouseDevice, mouseDevices);
    eventLoopForEach(SystemEventDevice<input_event>::keyDevice, keyDevices);
    eventLoopForEach(SystemEventDevice<input_event>::touchDevice, touchDevices);
}
boost::asio::awaitable<void> SystemEventsLinux::eventLoopForEach(
    SystemEventDevice<input_event>::Value device,
    std::vector<std::filesystem::path> paths
    ) {
    for (auto path : paths) {
        co_spawn(getContext(), [&]() { return eventLoopForDevice(device, path); }, [&](exception_ptr e, bool result) { submitResult(result); });
    }
}
