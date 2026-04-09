#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11)
#error "define at least one of `FEATURE_EVGETLIBINPUT`or `FEATURE_EVGETX11`"
#endif

#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/error.h"
#include "evget/storage/database_manager.h"

#ifdef FEATURE_EVGETLIBINPUT
#include "evgetlibinput/backend.h"
#include "evgetlibinput/drm.h"
#endif

#ifdef FEATURE_EVGETX11
#include "evgetx11/backend.h"
#endif

#ifdef FEATURE_EVGETLIBINPUT
namespace {
evget::Result<evgetlibinput::ScreenDimensions> ResolveScreenDimensions(evget::Cli& cli) {
    if (auto cli_dimensions = cli.ScreenDimensions()) {
        return evgetlibinput::ScreenDimensions{.width = cli_dimensions->first, .height = cli_dimensions->second};
    }

    auto drm = evgetlibinput::DrmOutput::New();
    if (!drm.has_value()) {
        return std::unexpected(drm.error());
    }
    return drm->GetDimensions();
}
} // namespace
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
        std::unique_ptr<evgetlibinput::Backend> li_backend{};
        if (event_source == evget::EventSource::kLibInput) {
            auto dimensions = ResolveScreenDimensions(cli);
            if (!dimensions.has_value()) {
                spdlog::error("{}", dimensions.error());
                return 1;
            }
            auto result = evgetlibinput::Backend::Create(*dimensions, manager);
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
            auto result = evgetx11::Backend::Create(manager);
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            x11_backend = std::move(*result);
            scheduler->SpawnResult(x11_backend->Handler().Start(), x11_backend->Handler(), exit_code);
        }
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
