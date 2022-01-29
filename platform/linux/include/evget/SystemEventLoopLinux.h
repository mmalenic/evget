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
#include "evget/SystemEventLoop.h"
#include "evget/SystemEvent.h"
#include <utility>

namespace evget {
    namespace asio = boost::asio;

    /**
     * Class represents processing linux system events.
     */
    template<boost::asio::execution::executor E>
    class SystemEventLoopLinux : public SystemEventLoop<E, input_event> {
    public:
        /**
         * Create the system events.
         *
         * @param mouseDevices mouse devices
         * @param keyDevices key devices
         * @param touchDevices touch devices
         */
        SystemEventLoopLinux(
            E& context,
            std::vector<std::filesystem::path> mouseDevices,
            std::vector<std::filesystem::path> keyDevices,
            std::vector<std::filesystem::path> touchDevices
        );

        /**
         * Run the event loop for a single type.
         */
        boost::asio::awaitable<void> eventLoopForDevice(
            SystemEvent<input_event>::Type type,
            std::filesystem::path path
        );

        /**
         * Run the event loop for each set of devices.
         */
        boost::asio::awaitable<void> eventLoopForEach(
            SystemEvent<input_event>::Type type,
            std::vector<std::filesystem::path> paths
        );

        boost::asio::awaitable<void> eventLoop() override;

    private:
        std::vector<std::filesystem::path> mouseDevices;
        std::vector<std::filesystem::path> keyDevices;
        std::vector<std::filesystem::path> touchDevices;
    };

    template<boost::asio::execution::executor E>
    SystemEventLoopLinux<E>::SystemEventLoopLinux(
        E& context,
        std::vector<std::filesystem::path> mouseDevices,
        std::vector<std::filesystem::path> keyDevices,
        std::vector<std::filesystem::path> touchDevices
    ) : SystemEventLoop<E, input_event>{context},
        mouseDevices{std::move(mouseDevices)},
        keyDevices{std::move(keyDevices)},
        touchDevices{std::move(touchDevices)} {
    }

    template<boost::asio::execution::executor E>
    boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoopForDevice(
        SystemEvent<input_event>::Type type,
        std::filesystem::path path
    ) {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            std::string err = fmt::format("Could not open path to read events: {}", strerror(errno));
            spdlog::error(err);
            throw UnsupportedOperationException(err);
        }

        asio::posix::stream_descriptor stream{this->getContext(), fd};
        while (!this->isCancelled()) {
            struct input_event event{};
            size_t n = co_await asio::async_read(stream, asio::buffer(&event, sizeof(event)), asio::use_awaitable);
            printf("%d\n", event.time);
            spdlog::trace(fmt::format("{} bytes event read in event loop.", n));

            auto systemEvent = SystemEvent(type, event);
            this->notify(systemEvent);
        }

        stream.cancel();
        stream.close();
        co_return;
    }

    template<boost::asio::execution::executor E>
    boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoop() {
        this->spawn(
            [this]() {
                spdlog::trace("Spawned for each mouse device");
                return eventLoopForEach(SystemEvent<input_event>::Type::mouseDevice, mouseDevices);
            }
        );
        this->spawn(
            [this]() {
                spdlog::trace("Spawned for each key device");
                return eventLoopForEach(SystemEvent<input_event>::Type::keyDevice, keyDevices);
            }
        );
        this->spawn(
            [this]() {
                spdlog::trace("Spawned for each touch device");
                return eventLoopForEach(SystemEvent<input_event>::Type::touchDevice, touchDevices);
            }
        );
        co_return;
    }

    template<boost::asio::execution::executor E>
    boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoopForEach(
        SystemEvent<input_event>::Type type,
        std::vector<std::filesystem::path> paths
    ) {
        for (const auto& path: paths) {
            this->spawn(
                [this, type, path]() {
                    return eventLoopForDevice(type, path);
                }
            );
        }
        co_return;
    }
}

#endif // EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
