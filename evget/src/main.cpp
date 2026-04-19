#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11) && !defined(FEATURE_EVGETWINDOWS)
#error "define at least one of `FEATURE_EVGETLIBINPUT`, `FEATURE_EVGETX11`, or `FEATURE_EVGETWINDOWS`"
#endif

#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/storage/database_manager.h"
#include "evget/storage/filter_store.h"

#ifdef FEATURE_EVGETLIBINPUT
#include "evgetlibinput/backend.h"
#endif

#ifdef FEATURE_EVGETX11
#include "evgetx11/backend.h"
#endif

#ifdef FEATURE_EVGETWINDOWS
#include "evgetwindows/backend.h"
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
#ifdef FEATURE_EVGETWINDOWS
    default_source = evget::EventSource::kWindows;
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

    auto filter = evget::FilterStore{manager, cli.Filter()};
    auto exit_code = 0;
    try {
        auto event_source = cli.EventSource();

#ifdef FEATURE_EVGETLIBINPUT
        std::unique_ptr<evgetlibinput::Backend> li_backend{};
        if (event_source == evget::EventSource::kLibInput) {
            auto result = evgetlibinput::Backend::Create(cli.ScreenDimensions(), filter, cli.Seat());
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            li_backend = std::move(*result);
            scheduler->SpawnResult(li_backend->Handler().Start(), li_backend->Handler(), exit_code);
        }
#endif

#ifdef FEATURE_EVGETX11
        std::unique_ptr<evgetx11::Backend> x11_backend{};
        if (event_source == evget::EventSource::kX11) {
            auto result = evgetx11::Backend::Create(filter, cli.Display());
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            x11_backend = std::move(*result);
            auto handler = x11_backend->Handler();
            if (!handler.has_value()) {
                spdlog::error("{}", handler.error());
                return 1;
            }
            scheduler->SpawnResult(handler->get().Start(), handler->get(), exit_code);
        }
#endif

#ifdef FEATURE_EVGETWINDOWS
        std::unique_ptr<evgetwindows::Backend> win_backend{};
        if (event_source == evget::EventSource::kWindows) {
            auto result = evgetwindows::Backend::Create(filter);
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            win_backend = std::move(*result);

            spdlog::error("Windows backend handler not yet implemented");
            return 1;
        }
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
