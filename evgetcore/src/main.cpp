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

#include <X11/Xlib.h>

#include <cstring>
#include "evgetcore/cli.h"
#include <iostream>

#include "evgetcore/database/sqlite/Connection.h"
#include "evgetcore/Storage/DatabaseStorage.h"
#include "evgetcore/Storage/JsonStorage.h"
#include "evgetx11/EventLoopX11.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XSetMaskCore.h"
#include "evgetx11/XSetMaskRefresh.h"
#include "evgetx11/XWrapperX11.h"
#include "evgetcore/EventHandler.h"
#include "evgetcore/Storage/DatabaseManager.h"

int main(int argc, char* argv[]) {
    auto cli = EvgetCore::Cli{};
    auto exit = cli.parse(argc, argv);
    if (!exit.has_value()) {
        return exit.error();
    }
    if (*exit) {
        return 0;
    }

    Display* display = XOpenDisplay(nullptr);
    EvgetX11::XWrapperX11 xWrapperX11{*display};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperX11};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointer{xWrapperX11, xEventSwitch};
    EvgetX11::EventTransformerX11 transformer{xWrapperX11, xEventSwitchPointer};
    EvgetX11::XSetMaskCore setCore{};
    EvgetX11::XSetMaskRefresh setRefresh{};
    EvgetX11::XInputHandler xInputHandler = EvgetX11::XInputHandler::build(xWrapperX11, {setCore, setRefresh}).value();
    EvgetX11::EventLoopX11 eventLoop{xInputHandler};

    auto scheduler = EvgetCore::Scheduler{};
    auto manager = EvgetCore::Storage::DatabaseManager{scheduler};
    for (auto&& store : cli.to_stores()) {
        manager.add_store(std::move(store));
    }
    EvgetCore::EventHandler handler{manager, transformer, eventLoop};

    scheduler.spawn([&handler]() -> boost::asio::awaitable<void> {
        co_await handler.start();
        co_return;
    });

    scheduler.join();

    return 0;
}
