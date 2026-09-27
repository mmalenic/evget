#ifndef EVGETX11_TEST_COMMON_INPUT_HANDLER_H
#define EVGETX11_TEST_COMMON_INPUT_HANDLER_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/scope/unique_fd.hpp>

#include <future>

#include "evget/error.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

namespace test {

/**
 * \brief A pipe which has reads representing the display.
 */
class Pipe {
public:
    /**
     * \brief Create the pipe.
     */
    Pipe();

    /**
     * \brief Get the read end.
     * \return the read end descriptor
     */
    [[nodiscard]] int ReadEnd() const;

    /**
     * \brief Write one byte to the pipe.
     * \return whether the byte was written
     */
    [[nodiscard]] bool Write() const;

private:
    boost::scope::unique_fd read_;
    boost::scope::unique_fd write_;
};

/**
 * \brief Get the next event and put it into the promise.
 * \param handler the handler
 * \param promise result of `Next`
 * \return awaitable that completes once the promise is set
 */
boost::asio::awaitable<void>
RunNext(const evgetx11::InputHandler& handler, std::promise<evget::Result<evgetx11::InputEvent>> promise);

/**
 * \brief Start `Next` on the pool.
 * \param handler the handler
 * \param pool the pool
 * \return the result of `Next`
 */
std::future<evget::Result<evgetx11::InputEvent>>
SpawnNext(const evgetx11::InputHandler& handler, boost::asio::thread_pool& pool);

} // namespace test

#endif
