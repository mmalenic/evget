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

#include <memory>
#include <utility>

#include "evgetcore/EventHandler.h"
#include "evgetcore/Storage/DatabaseManager.h"
#include "evgetcore/cli.h"
#include "evgetx11/EventLoopX11.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XEventSwitchTouch.h"
#include "evgetx11/XWrapperX11.h"

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
    auto transformer =
        EvgetX11::EventTransformerX11<EvgetX11::XEventSwitchPointerKey, EvgetX11::XEventSwitchTouch>::build(
            xWrapperX11
        );

    EvgetX11::EventLoopX11 eventLoop{EvgetX11::XInputHandler::build(xWrapperX11).value()};

    auto scheduler = std::make_shared<EvgetCore::Scheduler>();
    auto manager = EvgetCore::Storage::DatabaseManager{scheduler, {}, cli.store_n_events(), cli.store_after()};
    for (auto&& store : cli.to_stores().value()) {
        manager.add_store(std::move(store));
    }
    EvgetCore::EventHandler handler{manager, transformer, eventLoop};

    scheduler->spawn(handler.start());
    scheduler->join();

    return 0;
}
