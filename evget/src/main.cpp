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

#include "evgetx11/event_transformer.h"
#include "evgetx11/input_handler.h"
#include "evgetx11/x11_api.h"
#endif

#ifdef FEATURE_EVGETLIBINPUT
#include "evgetlibinput/event_transformer.h"
#include "evgetlibinput/libinput_api.h"
#include "evgetlibinput/next_event.h"
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
        auto builder = evgetx11::EventTransformerBuilder{}.PointerKey(x11_api).Touch();
        auto x11_transformer = std::move(builder).Build(x11_api);
        auto x11_next_event = evgetx11::InputHandlerBuilder::Build(x11_api);
        if (!x11_next_event.has_value()) {
            spdlog::error("{}", stores.error());
            return 1;
        }

        auto x11_handler = evget::EventHandler{manager, *x11_transformer, *x11_next_event};
        scheduler->SpawnResult(x11_handler.Start(), x11_handler, exit_code);
#endif

#ifdef FEATURE_EVGETLIBINPUT
        auto lib_input = evgetlibinput::LibInputApiImpl{};
        auto li_transformer = evgetlibinput::EventTransformer{};
        auto li_next_event = evgetlibinput::NextEvent{lib_input};
        auto li_handler = evget::EventHandler{manager, li_transformer, li_next_event};
        scheduler->SpawnResult(li_handler.Start(), x11_handler, exit_code);
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
