#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11)
#error "define at least one of `FEATURE_EVGETLIBINPUT`or `FEATURE_EVGETX11`"
#endif

#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/event_handler.h"
#include "evget/event_transformer.h"
#include "evget/storage/database_manager.h"

#ifdef FEATURE_EVGETLIBINPUT
#include "evgetlibinput/event_transformer.h"
#include "evgetlibinput/libinput_api.h"
#include "evgetlibinput/next_event.h"
#endif

#ifdef FEATURE_EVGETX11
#include <X11/Xlib.h>

#include "evgetx11/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"
#include "evgetx11/x11_api.h"
#endif

int main(int argc, char* argv[]) {
    evget::EventSource default_source{};
#ifdef FEATURE_EVGETX11
    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
    default_source = evget::EventSource::kX11;
#endif
#ifdef FEATURE_EVGETLIBINPUT
    default_source = evget::EventSource::kLibInput;
#endif

    auto cli = evget::Cli{default_source};
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
        auto event_source = cli.EventSource();

#ifdef FEATURE_EVGETLIBINPUT
        std::unique_ptr<evgetlibinput::LibInputApi> libinput{};
        std::optional<evgetlibinput::EventTransformer> li_transformer{};
        std::optional<evgetlibinput::NextEvent> li_next_event{};
        std::optional<evget::EventHandler<evgetlibinput::LibInputEvent>> li_handler{};

        if (event_source == evget::EventSource::kLibInput) {
            auto lib_input_result = evgetlibinput::LibInputApiImpl::New();
            if (!lib_input_result.has_value()) {
                spdlog::error("{}", stores.error());
                return 1;
            }
            libinput = std::move(*lib_input_result);

            li_transformer.emplace(*libinput);
            li_next_event.emplace(*libinput);
            li_handler.emplace(manager, *li_transformer, *li_next_event);

            scheduler->SpawnResult(li_handler->Start(), *li_handler, exit_code);
        }
#endif

#ifdef FEATURE_EVGETX11
        Display* display = nullptr;
        std::optional<evgetx11::X11ApiImpl> x11_api{};
        std::unique_ptr<evget::EventTransformer<evgetx11::InputEvent>> x11_transformer{};
        std::unique_ptr<evgetx11::InputHandler> x11_next_event{};
        std::optional<evget::EventHandler<evgetx11::InputEvent>> x11_handler{};

        if (event_source == evget::EventSource::kX11) {
            display = XOpenDisplay(nullptr);
            x11_api.emplace(*display);

            auto builder = evgetx11::EventTransformerBuilder{}.PointerKey(*x11_api).Touch();
            auto transformer = std::move(builder).Build(*x11_api);
            x11_transformer = std::move(transformer);

            auto x11_next_event_result = evgetx11::InputHandlerBuilder::Build(*x11_api);
            if (!x11_next_event_result.has_value()) {
                spdlog::error("{}", stores.error());
                return 1;
            }
            x11_next_event = std::move(*x11_next_event_result);

            x11_handler.emplace(manager, *x11_transformer, *x11_next_event);
            scheduler->SpawnResult(x11_handler->Start(), *x11_handler, exit_code);
        }
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
