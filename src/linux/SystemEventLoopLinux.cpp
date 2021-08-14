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

#include "SystemEventLoopLinux.h"

#include <utility>

namespace asio = boost::asio;
namespace fibers = boost::fibers;

using namespace std;

template<boost::asio::execution::executor E>
SystemEventLoopLinux<E>::SystemEventLoopLinux(
    std::vector<std::filesystem::path>  mouseDevices,
    std::vector<std::filesystem::path>  keyDevices,
    std::vector<std::filesystem::path>  touchDevices,
    boost::asio::thread_pool& context
    ) : SystemEventLoop<input_event, E>{mouseDevices.size() + keyDevices.size() + touchDevices.size(), context}, mouseDevices{std::move(mouseDevices)}, keyDevices{std::move(keyDevices)}, touchDevices{std::move(touchDevices)} {
}

template<boost::asio::execution::executor E>
boost::asio::awaitable<bool> SystemEventLoopLinux<E>::eventLoopForDevice(SystemEvent<input_event>::Type type, std::filesystem::path path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        string err = strerror(errno);
        spdlog::warn("Could not open type to read events: " + err);
        co_return false;
    }

    asio::posix::stream_descriptor stream{this->getContext(), fd};
    while (!this->isCancelled()) {
        struct input_event event;
        memset(&event, 0, sizeof(event));
        size_t n = co_await stream.async_read_some(asio::buffer(&event, sizeof(event)), asio::use_awaitable);
        spdlog::trace("Event read in event loop.");

        if (n != sizeof(event)) {
            spdlog::warn("Incorrect bytes read for input event.");
            continue;
        }

        auto systemEvent = SystemEvent(type, event);
        notify(systemEvent);
    }

    stream.cancel();
    stream.close();
    co_return true;
}

template<boost::asio::execution::executor E>
boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoop() {
    eventLoopForEach(SystemEvent<input_event>::mouseDevice, mouseDevices);
    eventLoopForEach(SystemEvent<input_event>::keyDevice, keyDevices);
    eventLoopForEach(SystemEvent<input_event>::touchDevice, touchDevices);
}

template<boost::asio::execution::executor E>
boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoopForEach(
    SystemEvent<input_event>::Type type,
    std::vector<std::filesystem::path> paths
    ) {
    for (auto path : paths) {
        co_spawn(this->getContext(), [&]() { return eventLoopForDevice(type, path); }, [&](exception_ptr e, bool result) {
            this->submitOutcome(
                result
            ); });
    }
}
