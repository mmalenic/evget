#include <spdlog/spdlog.h>

#include <exception>

#include "evget/cli.h"

// #include "evgetx11/EventLoopX11.h"
// #include "evgetx11/EventTransformerX11.h"
// #include "evgetx11/XEventSwitchPointerKey.h"
// #include "evgetx11/XEventSwitchTouch.h"
// #include "evgetx11/XWrapperX11.h"

int main(int argc, char* argv[]) {
    try {
        auto cli = evget::Cli{};
        auto exit = cli.Parse(argc, argv);
        if (!exit.has_value()) {
            return exit.error();
        }
        if (*exit) {
            return 0;
        }

        // Display* display = XOpenDisplay(nullptr);
        // EvgetX11::XWrapperX11 x_wrapper_x11{*display};
        // auto transformer =
        //     EvgetX11::EventTransformerX11<EvgetX11::XEventSwitchPointerKey, EvgetX11::XEventSwitchTouch>::build(
        //         xWrapperX11
        //     );
        //
        // EvgetX11::EventLoopX11 event_loop{EvgetX11::XInputHandler::build(xWrapperX11).value()};
        //
        // auto scheduler = std::make_shared<evget::Scheduler>();
        // auto manager = evget::DatabaseManager{scheduler, {}, cli.store_n_events(), cli.store_after()};
        // for (auto&& store : cli.to_stores().value()) {
        //     manager.add_store(std::move(store));
        // }
        // evget::EventHandler handler{manager, transformer, eventLoop};
        //
        // auto exit_code = 0;
        // scheduler->spawn(handler.start(), [&exit_code](auto err) {
        //     if (!err.has_value()) {
        //         exit_code = 1;
        //         spdlog::error(err.error().message);
        //     }
        // });
        // scheduler->join();
        //
        // return exit_code;
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
