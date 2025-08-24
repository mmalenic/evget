#include <X11/Xlib.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/event_handler.h"
#include "evget/storage/database_manager.h"
#include "evgetx11/event_loop.h"
#include "evgetx11/event_switch_pointer_key.h"
#include "evgetx11/event_switch_touch.h"
#include "evgetx11/event_transformer.h"
#include "evgetx11/x11_api.h"

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

        Display* display = XOpenDisplay(nullptr);
        evgetx11::X11ApiImpl x_wrapper_x11{*display};
        auto transformer =
            evgetx11::EventTransformer<evgetx11::EventSwitchPointerKey, evgetx11::EventSwitchTouch>::Build(
                x_wrapper_x11
            );

        evgetx11::EventLoop event_loop{evgetx11::InputHandler::Build(x_wrapper_x11).value()};

        auto scheduler = std::make_shared<evget::Scheduler>();
        auto manager = evget::DatabaseManager{scheduler, {}, cli.StoreNEvents(), cli.StoreAfter()};
        for (auto&& store : cli.ToStores().value()) {
            manager.AddStore(std::move(store));
        }
        evget::EventHandler handler{manager, transformer, event_loop};

        auto exit_code = 0;
        scheduler->Spawn(handler.Start(), [&exit_code](auto err) {
            if (!err.has_value()) {
                exit_code = 1;
                spdlog::error(err.error().message);
            }
        });
        scheduler->Join();

        return exit_code;
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
