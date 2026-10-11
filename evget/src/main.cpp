#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11) && !defined(FEATURE_EVGETWINDOWS)
#error "define at least one of `FEATURE_EVGETLIBINPUT`, `FEATURE_EVGETX11`, or `FEATURE_EVGETWINDOWS`"
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/scope/scope_exit.hpp>
#include <spdlog/spdlog.h>

#ifndef _WIN32
#include <signal.h> // NOLINT(hicpp-deprecated-headers,modernize-deprecated-headers)
#endif

#include <chrono>
#include <csignal>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/shutdown.h"
#include "evget/storage/database_manager.h"
#include "evget/storage/exclude_window_title_store.h"
#include "evget/storage/filter_store.h"
#include "evget/storage/store.h"

#ifdef FEATURE_EVGETLIBINPUT
#include "evgetlibinput/backend.h"
#endif

#ifdef FEATURE_EVGETX11
#include "evgetx11/backend.h"
#endif

#ifdef FEATURE_EVGETWINDOWS
#include "evgetwindows/backend.h"
#include "evgetwindows/console_stop.h"
#include "evgetwindows/elevation.h"
#endif

namespace {
constexpr std::chrono::seconds kChannelTimeout{2};

template <typename Handler, typename Backend>
void StopSource(Handler& handler, Backend& backend, const evget::Scheduler& scheduler) {
    // This must go before so the loop treats the stop gracefully.
    handler.Stop();
    backend.RequestStop();
    if (!scheduler.IsStopped() && !handler.WaitFinished(kChannelTimeout)) {
        spdlog::error("input events where not removed within {}, dropping events", kChannelTimeout);
        // A loop that never suspends would otherwise keep the pool from stopping.
        handler.Stop();
    }
}

#ifdef FEATURE_EVGETX11
void StopX11Source(evgetx11::Backend& backend, const evget::Scheduler& scheduler) {
    auto handler = backend.Handler();
    if (handler.has_value()) {
        StopSource(handler->get(), backend, scheduler);
    }
}
#endif

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

#ifdef FEATURE_EVGETWINDOWS
    if (cli.EventSource() == evget::EventSource::kWindows) {
        evgetwindows::LogElevation();
    }
#endif

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
    auto exclude_title = evget::ExcludeWindowTitleStore{filter};
    evget::Store& merged = cli.ExcludeWindowTitle() ? static_cast<evget::Store&>(exclude_title) : filter;
    auto shutdown = std::make_shared<evget::Shutdown>(scheduler, [&manager] { manager.Flush(); });

    boost::asio::io_context signal_context;
    boost::asio::signal_set signals{signal_context, SIGINT, SIGTERM};

#ifndef _WIN32
    // Adding SIGHUP would replace an inherited SIG_IGN, so only add it when not ignored.
    struct sigaction hangup{};
    if (sigaction(SIGHUP, nullptr, &hangup) == 0 && hangup.sa_handler != SIG_IGN) {
        signals.add(SIGHUP);
    }
#endif

    // NOLINTNEXTLINE(misc-include-cleaner)
    signals.async_wait([shutdown](const boost::system::error_code& error, int) {
        if (!error) {
            spdlog::debug("received stop signal, stopping scheduler");
            shutdown->Request();
        }
    });
    const std::jthread signal_thread{[&signal_context] {
        try {
            signal_context.run();
        } catch (const std::exception& e) {
            spdlog::error("signal handling failed: {}", e.what());
        }
    }};
    const boost::scope::scope_exit stop_signals{[&signal_context] {
        try {
            signal_context.stop();
        } catch (const std::exception& e) {
            spdlog::error("stopping signal handling failed: {}", e.what());
        }
    }};
#ifdef FEATURE_EVGETWINDOWS
    // Registered after the signal so it can run before the console handler.
    const evgetwindows::ConsoleStop console_stop{[shutdown] { shutdown->Request(); }};
#endif

    auto exit_code = 0;
    auto event_source = cli.EventSource();
    auto capturing = false;

#ifdef FEATURE_EVGETLIBINPUT
    std::unique_ptr<evgetlibinput::Backend> li_backend{};
#endif
#ifdef FEATURE_EVGETX11
    std::unique_ptr<evgetx11::Backend> x11_backend{};
#endif
#ifdef FEATURE_EVGETWINDOWS
    std::unique_ptr<evgetwindows::Backend> win_backend{};
#endif
    // Completes stop on every return path before the backends are destroyed.
    const boost::scope::scope_exit stop_before_teardown{[&shutdown] { shutdown->Request(); }};

#ifdef FEATURE_EVGETLIBINPUT
    if (event_source == evget::EventSource::kLibInput) {
        auto result = evgetlibinput::Backend::Create(cli.ScreenDimensions(), merged, cli.Seat());
        if (!result.has_value()) {
            spdlog::error("{}", result.error());
            return 1;
        }
        li_backend = std::move(*result);
        shutdown->SetSource([&li_backend, scheduler] { StopSource(li_backend->Handler(), *li_backend, *scheduler); });
        shutdown->SpawnCapture(li_backend->Handler().Start(), exit_code);
        capturing = true;
    }
#endif

#ifdef FEATURE_EVGETX11
    if (event_source == evget::EventSource::kX11) {
        auto result = evgetx11::Backend::Create(merged, cli.Display(), [shutdown] { shutdown->RequestFromPool(); });
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
        shutdown->SetSource([&x11_backend, scheduler] { StopX11Source(*x11_backend, *scheduler); });
        shutdown->SpawnCapture(handler->get().Start(), exit_code);
        capturing = true;
    }
#endif

#ifdef FEATURE_EVGETWINDOWS
    if (event_source == evget::EventSource::kWindows) {
        auto result = evgetwindows::Backend::Create(merged, scheduler->Executor(), [shutdown] { shutdown->Request(); });
        if (!result.has_value()) {
            spdlog::error("{}", result.error());
            return 1;
        }
        win_backend = std::move(*result);
        shutdown->SetSource([&win_backend, scheduler] {
            StopSource(win_backend->Handler(), *win_backend, *scheduler);
        });
        shutdown->SpawnCapture(win_backend->Handler().Start(), exit_code);
        capturing = true;
    }
#endif

    if (!capturing) {
        spdlog::error("invalid event source");
        return 1;
    }
    spdlog::info("capturing input events");

    scheduler->Join();
    shutdown->Request();
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
