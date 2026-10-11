#include "evgetlibinput/next_event.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/scope/unique_fd.hpp>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/eventfd.h>

#include <cerrno>
#include <format>
#include <string_view>
#include <system_error>
#include <utility>

#include "evget/error.h"
#include "evget/input_event.h"
#include "evgetlibinput/libinput.h"

namespace {

evget::Error<evget::ErrorType> HandlerError(std::string_view message, std::string_view cause) {
    return {.error_type = evget::ErrorType::kEventHandlerError, .message = std::format("{}: {}", message, cause)};
}

evget::Result<boost::asio::posix::stream_descriptor>
Watch(const boost::asio::any_io_executor& executor, int descriptor) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    boost::scope::unique_fd duplicate{fcntl(descriptor, F_DUPFD_CLOEXEC, 0)};
    if (!duplicate) {
        return evget::Err{
            HandlerError("unable to duplicate descriptor", std::generic_category().message(errno))
        };
    }

    boost::asio::posix::stream_descriptor watched{executor};
    // NOLINTNEXTLINE(misc-include-cleaner)
    boost::system::error_code error;
    // NOLINTNEXTLINE(bugprone-unused-return-value, cert-err33-c): the returned code is `error`
    watched.assign(duplicate.get(), error);
    if (error) {
        return evget::Err{HandlerError("unable to wait for descriptor", error.message())};
    }
    duplicate.release();

    return watched;
}
} // namespace

evgetlibinput::NextEvent::NextEvent(LibInputApi& libinput_api)
    : libinput_api_{libinput_api}, stop_event_{eventfd(0, EFD_CLOEXEC)} {}

boost::asio::awaitable<evget::Result<evget::InputEvent<evgetlibinput::LibInputEvent>>>
evgetlibinput::NextEvent::Next() const {
    while (true) {
        if (stop_requested_.load()) {
            if (libinput_api_.get().HasQueuedEvent()) {
                break;
            }
            co_return evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError, .message = "the libinput handler was stopped"}
            };
        }

        auto pending = libinput_api_.get().HasPendingEvent();
        if (!pending.has_value()) {
            co_return evget::Err{pending.error()};
        }
        if (*pending) {
            break;
        }

        auto waited = co_await WaitForInput();
        if (!waited.has_value()) {
            co_return evget::Err{waited.error()};
        }
    }

    co_return libinput_api_.get().GetEvent().transform([](auto event) { return evget::InputEvent{std::move(event)}; });
}

void evgetlibinput::NextEvent::RequestStop() {
    stop_requested_.store(true);
    if (stop_event_ && eventfd_write(stop_event_.get(), 1) != 0) {
        spdlog::error(
            "{}",
            HandlerError("unable to wake the libinput handler", std::generic_category().message(errno))
        );
    }
}

boost::asio::awaitable<evget::Result<void>> evgetlibinput::NextEvent::WaitForInput() const {
    if (!input_.has_value() || !stop_.has_value()) {
        if (!stop_event_) {
            co_return evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError,
                 .message = "the libinput handler has no stop event"}
            };
        }

        const auto executor = co_await boost::asio::this_coro::executor;
        auto input = Watch(executor, libinput_api_.get().GetFd());
        if (!input.has_value()) {
            co_return evget::Err{input.error()};
        }
        auto stop = Watch(executor, stop_event_.get());
        if (!stop.has_value()) {
            co_return evget::Err{stop.error()};
        }

        input_.emplace(std::move(*input));
        stop_.emplace(std::move(*stop));
    }

    // The stop event is never read, so it stays readable and a stop cannot be missed between waits.
    auto [order, input_error, stop_error] =
        co_await boost::asio::experimental::make_parallel_group(
            input_->async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::deferred),
            stop_->async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::deferred)
        )
            .async_wait(boost::asio::experimental::wait_for_one(), boost::asio::use_awaitable);

    const auto& error = order.front() == 0 ? input_error : stop_error;
    if (error) {
        co_return evget::Err{HandlerError("waiting for libinput input failed", error.message())};
    }
    co_return evget::Result<void>{};
}
