/**
 * \file shutdown.h
 * \brief One stop routine shared by every stop trigger.
 */

#ifndef EVGET_SHUTDOWN_H
#define EVGET_SHUTDOWN_H

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <memory>
#include <mutex>

#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"

namespace evget {

/**
 * \brief Stops the event source and schedule, flushing buffered events inbetween.
 */
class Shutdown {
public:
    /**
     * \brief Construct a shutdown routine.
     * \param scheduler scheduler to stop
     * \param flush writes buffered events while the scheduler is running
     */
    Shutdown(std::shared_ptr<Scheduler> scheduler, std::function<void()> flush);

    ~Shutdown() = default;

    Shutdown(const Shutdown&) = delete;
    Shutdown(Shutdown&&) noexcept = delete;
    Shutdown& operator=(const Shutdown&) = delete;
    Shutdown& operator=(Shutdown&&) noexcept = delete;

    /**
     * \brief Set the source step that stops the event source.
     * \param stop_source stops the event source
     */
    void SetSource(std::function<void()> stop_source);

    /**
     * \brief Run the capture loop on the scheduler and request the stop once the loop ends.
     * \param capture the capture loop
     * \param exit_code set to 1 if the capture loop fails or throws
     *
     * \note The shutdown and `exit_code` must outlive the scheduler's threads.
     */
    void SpawnCapture(boost::asio::awaitable<Result<void>> capture, int& exit_code);

    /**
     * \brief Run the stop once and returns after it has finished on any asking thread.
     *
     * \note This must not be called from a scheduler pool thread, because the flush blocks
     *       the pool, which could deadlock. Use `RequestFromPool` there.
     */
    void Request() noexcept;

    /**
     * \brief Run the stop on a new thread and return once it has finished, so a pool thread can request it.
     */
    void RequestFromPool() noexcept;

private:
    std::shared_ptr<Scheduler> scheduler_;
    std::function<void()> flush_;
    std::mutex mutex_;
    std::function<void()> stop_source_;
    std::once_flag once_;
};

} // namespace evget

#endif
