#include "evget/shutdown.h"

#include <spdlog/spdlog.h>

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "evget/async/scheduler/scheduler.h"

evget::Shutdown::Shutdown(std::shared_ptr<Scheduler> scheduler, std::function<void()> flush)
    : scheduler_{std::move(scheduler)}, flush_{std::move(flush)} {}

void evget::Shutdown::SetSource(std::function<void()> stop_source) {
    const std::scoped_lock lock{mutex_};
    stop_source_ = std::move(stop_source);
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
            }

            try {
                if (!scheduler_->IsStopped()) {
                    flush_();
                }
            } catch (const std::exception& e) {
                spdlog::error("flushing events failed: {}", e.what());
            }

            scheduler_->Stop();
        });
    } catch (const std::exception& e) {
        spdlog::error("shutdown failed: {}", e.what());
    } catch (...) {
        spdlog::error("shutdown failed");
    }
}
