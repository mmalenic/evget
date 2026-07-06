#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11) && !defined(FEATURE_EVGETWINDOWS)
#error "define at least one of `FEATURE_EVGETLIBINPUT`, `FEATURE_EVGETX11`, or `FEATURE_EVGETWINDOWS`"
#endif

#include <spdlog/spdlog.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/scope/scope_exit.hpp>
#include <csignal>
#include <exception>
#include <functional>
#include <memory>
#include <thread>
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

namespace {
int Run(int argc, char** argv) {
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

    std::function<void()> stop_source = [] {};
    boost::asio::io_context signal_context;
    boost::asio::signal_set signals{signal_context, SIGINT, SIGTERM};
    // NOLINTNEXTLINE(misc-include-cleaner)
    signals.async_wait([scheduler, &stop_source, &manager](const boost::system::error_code& error, int) {
        if (!error) {
            spdlog::debug("received stop signal, stopping scheduler");
            stop_source();
            manager.Flush();
            scheduler->Stop();
        }
    });
    const std::jthread signal_thread{[&signal_context] { signal_context.run(); }};
    const boost::scope::scope_exit stop_signals{[&signal_context] { signal_context.stop(); }};

    auto exit_code = 0;
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
        stop_source = [&li_backend] { li_backend->Handler().Stop(); };
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
        stop_source = [&x11_backend] {
            auto handler = x11_backend->Handler();
            if (handler.has_value()) {
                handler->get().Stop();
            }
        };
        scheduler->SpawnResult(handler->get().Start(), handler->get(), exit_code);
    }
#endif

#ifdef FEATURE_EVGETWINDOWS
    std::unique_ptr<evgetwindows::Backend> win_backend{};
    if (event_source == evget::EventSource::kWindows) {
        auto result = evgetwindows::Backend::Create(filter, scheduler->Executor());
        if (!result.has_value()) {
            spdlog::error("{}", result.error());
            return 1;
        }
        win_backend = std::move(*result);
        stop_source = [&win_backend] {
            win_backend->Handler().Stop();
            win_backend->Stop();
        };
        scheduler->SpawnResult(win_backend->Handler().Start(), win_backend->Handler(), exit_code);
    }
#endif

    scheduler->Join();
    return exit_code;
}
} // namespace

int main(int argc, char* argv[]) {
    try {
        return Run(argc, argv);
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    } catch (...) {
        spdlog::error("unknown error");
        return 1;
    }
}
