#include "evget/shutdown.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp> // NOLINT(misc-include-cleaner)
#include <spdlog/spdlog.h>

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"

evget::Shutdown::Shutdown(std::shared_ptr<Scheduler> scheduler, std::function<void()> flush)
    : scheduler_{std::move(scheduler)}, flush_{std::move(flush)} {}

void evget::Shutdown::SetSource(std::function<void()> stop_source) {
    const std::scoped_lock lock{mutex_};
    stop_source_ = std::move(stop_source);
}

void evget::Shutdown::SpawnCapture(boost::asio::awaitable<Result<void>> capture, int& exit_code) {
    // NOLINTNEXTLINE(misc-include-cleaner)
    boost::asio::co_spawn(
        scheduler_->Executor(),
        std::move(capture),
        [this, &exit_code](const std::exception_ptr& exception, const Result<void>& result) {
            if (exception) {
                exit_code = 1;
                try {
                    std::rethrow_exception(exception);
                } catch (const std::exception& e) {
                    spdlog::error("capturing input events failed: {}", e.what());
                } catch (...) {
                    spdlog::error("capturing input events failed with an unknown error");
                }
            } else if (!result.has_value()) {
                exit_code = 1;
                spdlog::error("{}", result.error());
            }

            RequestFromPool();
        }
    );
}

void evget::Shutdown::Request() noexcept {
    try {
        std::call_once(once_, [this] {
            std::function<void()> stop_source;
            {
                const std::scoped_lock lock{mutex_};
                stop_source = stop_source_;
            }

            try {
                if (stop_source) {
                    stop_source();
                }
            } catch (const std::exception& e) {
                spdlog::error("stopping event source failed: {}", e.what());
            } catch (...) {
                spdlog::error("stopping event source failed with an unknown error");
            }

            try {
                if (scheduler_->IsStopped()) {
                    spdlog::error("scheduler already stopped, any buffered events were not flushed");
                } else {
                    flush_();
                }
            } catch (const std::exception& e) {
                spdlog::error("flushing events failed: {}", e.what());
            } catch (...) {
                spdlog::error("flushing buffered events failed with an unknown error");
            }

            scheduler_->Stop();
        });
    } catch (const std::exception& e) {
        spdlog::error("shutdown failed: {}", e.what());
    } catch (...) {
        spdlog::error("shutdown failed");
    }
}

void evget::Shutdown::RequestFromPool() noexcept {
    try {
        const std::jthread stop{[this] { Request(); }};
    } catch (const std::exception& e) {
        spdlog::error("unable to start the shutdown: {}", e.what());
    }
}
