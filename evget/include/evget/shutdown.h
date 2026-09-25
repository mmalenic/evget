/**
 * \file shutdown.h
 * \brief One stop routine shared by every stop trigger.
 */

#ifndef EVGET_SHUTDOWN_H
#define EVGET_SHUTDOWN_H

#include <functional>
#include <memory>
#include <mutex>

#include "evget/async/scheduler/scheduler.h"

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
     * \brief Run the stop once and returns after it has finished on any asking thread.
     *
     * \note This must not be called from a scheduler pool thread, because the flush blocks
     *       the pool, which could deadlock.
     */
    void Request() noexcept;

private:
    std::shared_ptr<Scheduler> scheduler_;
    std::function<void()> flush_;
    std::mutex mutex_;
    std::function<void()> stop_source_;
    std::once_flag once_;
};

} // namespace evget

#endif
