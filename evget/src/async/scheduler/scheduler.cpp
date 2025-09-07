#include "evget/async/scheduler/scheduler.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <exception>
#include <utility>

evget::Scheduler::Scheduler(std::size_t n_threads) : pool_{n_threads} {}

void evget::Scheduler::Join() {
    pool_.join();
}

void evget::Scheduler::Stop() {
    stopped_.store(true);
    pool_.stop();
}

boost::asio::awaitable<bool> evget::Scheduler::IsStopped() const {
    co_return stopped_.load();
}

void evget::Scheduler::LogException(const std::exception_ptr& error) {
    try {
        if (error) {
            std::rethrow_exception(error);
        }
    } catch (const std::exception& e) {
        spdlog::error("exception in coroutine: {}", e.what());
        Stop();
    }
}

void evget::Scheduler::Spawn(asio::awaitable<void>&& task) {
    SpawnImpl(std::move(task), [this] { this->Stop(); }, pool_);
}
