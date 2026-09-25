#ifndef EVGET_TEST_COMMON_EVENT_LOOP_H
#define EVGET_TEST_COMMON_EVENT_LOOP_H

#include <boost/asio/awaitable.hpp>

#include <deque>
#include <mutex>
#include <optional>
#include <vector>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evget/event_loop.h"
#include "evget/next_event.h"

namespace test {

/**
 * \brief A source that sends queued events.
 */
class QueuedNextEvent : public evget::NextEvent<int> {
public:
    /**
     * \brief Create a source with the events.
     * \param events events to return
     */
    explicit QueuedNextEvent(std::vector<int> events);

    /**
     * \brief Take the next queued event.
     *
     * \return the next event
     */
    [[nodiscard]] boost::asio::awaitable<evget::Result<int>> Next() const override;

private:
    mutable std::mutex mutex_;
    mutable std::deque<int> events_;
};

/**
 * \brief A listener that records every event it is notified of.
 */
class RecordingListener : public evget::EventListener<int> {
public:
    /**
     * \brief Record the event.
     * \param event event
     * \return success
     */
    boost::asio::awaitable<evget::Result<void>> Notify(int event) override;

    /**
     * \brief Start the listener.
     * \return success
     */
    boost::asio::awaitable<evget::Result<void>> Start() override;

    /**
     * \brief Get the events received.
     * \return received events
     */
    [[nodiscard]] std::vector<int> Received();

private:
    std::mutex mutex_;
    std::vector<int> received_;
};

/**
 * \brief Run an event loop.
 * \param loop event loop to run
 * \param result set to the loop result
 * \return awaitable that completes when the loop returns
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
boost::asio::awaitable<void> RunLoop(evget::EventLoop<int>& loop, std::optional<evget::Result<void>>& result);

} // namespace test

#endif
