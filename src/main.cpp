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

#include <CommandLineLinux.h>
#include <EventDeviceLister.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <atomic>
#include <EventHandler.h>
#include <SystemEventLoopLinux.h>

using namespace std;

boost::asio::awaitable<int> test(int arg) {
    int a = arg;
    co_return 1;
}

int main(int argc, char* argv[]) {
    CommandLineLinux cmd{argc, argv};
    cmd.readArgs();

    spdlog::set_level(cmd.getLogLevel());

    boost::asio::thread_pool pool{};
    auto context = pool.get_executor();
    Storage<boost::asio::thread_pool::executor_type> storage{context};
    EventTransformer<int> transformer{};
    SystemEventLoopLinux eventLoop{context, {}, {}, {}};

//    EventHandler hanlder{Storage<boost::asio::thread_pool::executor_type>{context.get_executor()}, EventTransformer<void>{}, SystemEventLoopLinux{context, vector{}, vector{}, vector{}}};

    boost::asio::co_spawn(context,
        []() {
            return test(1);
        }, [cmd](exception_ptr e, int a) {
            cmd.getLogLevel();
        });

    if (cmd.isListEventDevices()) {
        EventDeviceLister lister{};
        cout << lister;
    }
    return 1;
}