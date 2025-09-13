#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/event_handler.h"
#include "evget/storage/database_manager.h"

#ifdef FEATURE_EVGETX11
#include <X11/Xlib.h>

#include "evgetx11/event_handler.h"
#include "evgetx11/x11_api.h"
#endif

int main(int argc, char* argv[]) {
    auto cli = evget::Cli{};
    auto exit = cli.Parse(argc, argv);
    if (!exit.has_value()) {
        return exit.error();
    }
    if (*exit) {
        return 0;
    }

    std::shared_ptr<evget::Scheduler> scheduler;
    try {
        scheduler = std::make_shared<evget::Scheduler>();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return 1;
    }
    auto manager = evget::DatabaseManager{scheduler, {}, cli.StoreNEvents(), cli.StoreAfter()};

    auto stores = cli.ToStores();
    if (!stores.has_value()) {
        spdlog::error("{}", stores.error());
        return 1;
    }
    for (auto&& store : *stores) {
        manager.AddStore(std::move(store));
    }

    auto exit_code = 0;
    try {
#ifdef FEATURE_EVGETX11
        Display* display = XOpenDisplay(nullptr);
        evgetx11::X11ApiImpl x11_api{*display};
        auto handler = evgetx11::EventHandlerBuilder{}.Build(manager, x11_api);
        if (!handler.has_value()) {
            spdlog::error("{}", handler.error());
            return 1;
        }
        scheduler->SpawnResult(handler->Start(), *handler, exit_code);
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
