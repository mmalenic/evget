#include "evgetx11/input_handler.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/scope/unique_fd.hpp>
#include <evget/error.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/eventfd.h>

#include <array>
#include <cerrno>
#include <format>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>

#include "evgetx11/input_event.h"
#include "evgetx11/x11.h"

evgetx11::InputHandler::InputHandler(X11Api& x_wrapper) : x_wrapper_{x_wrapper}, stop_event_{eventfd(0, EFD_CLOEXEC)} {}

namespace {

constexpr int kVersionMajor = 2;
constexpr int kVersionMinor = 2;

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
        return evget::Err{HandlerError("unable to wait on descriptor", error.message())};
    }
    duplicate.release();

    return watched;
}

evget::Result<void> AnnounceVersion(evgetx11::X11Api& x_wrapper) {
    int major = kVersionMajor;
    int minor = kVersionMinor;

    Status status = x_wrapper.QueryVersion(major, minor);
    if (status == Success) {
        spdlog::info("XI2 is supported with version {}.{}", major, minor);
        return {};
    }

    return evget::Err{
        {.error_type = evget::ErrorType::kEventHandlerError,
         .message = std::format("XI2 is not supported, only version {}.{} is available.", major, minor)}
    };
}

void SetMask(evgetx11::X11Api& x_wrapper) {
    XIEventMask mask{};
    mask.deviceid = XIAllMasterDevices;

    std::array<unsigned char, XI_LASTEVENT> event_mask{};
    evgetx11::X11::SetMask(
        event_mask.data(),
        {
            XI_RawButtonPress,
            XI_RawButtonRelease,
            XI_RawKeyPress,
            XI_RawKeyRelease,
            XI_RawMotion,
            XI_DeviceChanged,
            XI_RawTouchBegin,
            XI_RawTouchEnd,
            XI_RawTouchUpdate,
        }
    );

    mask.mask_len = sizeof(event_mask);
    mask.mask = event_mask.data();

    x_wrapper.SelectEvents(mask);

    // Hierarchy events must go on XIAllDevices.
    std::array<unsigned char, XI_LASTEVENT> hierarchy_mask{};
    evgetx11::X11::SetMask(hierarchy_mask.data(), {XI_HierarchyChanged});
    XIEventMask hierarchy{.deviceid = XIAllDevices, .mask_len = sizeof(hierarchy_mask), .mask = hierarchy_mask.data()};
    x_wrapper.SelectEvents(hierarchy);
}
} // namespace

boost::asio::awaitable<evget::Result<evgetx11::InputEvent>> evgetx11::InputHandler::Next() const {
    while (true) {
        if (stop_requested_.load()) {
            // Replies read later also queue events, so the queue length at the stop is the bound.
            if (!remaining_after_stop_.has_value()) {
                remaining_after_stop_ = x_wrapper_.get().Queued();
                spdlog::debug("removing {} queued X11 events", *remaining_after_stop_);
            }
            if (*remaining_after_stop_ > 0) {
                --*remaining_after_stop_;
                break;
            }
            co_return evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError, .message = "the X11 input handler was stopped"}
            };
        }

        if (x_wrapper_.get().Pending() > 0) {
            break;
        }

        auto waited = co_await WaitForInput();
        if (!waited.has_value()) {
            co_return evget::Err{waited.error()};
        }
    }

    co_return InputEvent::NextEvent(x_wrapper_.get());
}

void evgetx11::InputHandler::RequestStop() {
    stop_requested_.store(true);
    if (stop_event_ && eventfd_write(stop_event_.get(), 1) != 0) {
        spdlog::error(
            "{}",
            HandlerError("unable to wake the X11 input handler", std::generic_category().message(errno))
        );
    }
}

boost::asio::awaitable<evget::Result<void>> evgetx11::InputHandler::WaitForInput() const {
    if (!connection_.has_value() || !stop_.has_value()) {
        if (!stop_event_) {
            co_return evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError,
                 .message = "the X11 input handler has no stop event to wait on"}
            };
        }

        const auto executor = co_await boost::asio::this_coro::executor;
        auto connection = Watch(executor, x_wrapper_.get().GetConnectionNumber());
        if (!connection.has_value()) {
            co_return evget::Err{connection.error()};
        }
        auto stop = Watch(executor, stop_event_.get());
        if (!stop.has_value()) {
            co_return evget::Err{stop.error()};
        }

        connection_.emplace(std::move(*connection));
        stop_.emplace(std::move(*stop));
    }

    // The stop event is never read, so it must be readable, meaning that a stop cannot be missed between waits.
    auto [order, connection_error, stop_error] =
        co_await boost::asio::experimental::make_parallel_group(
            connection_->async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::deferred),
            stop_->async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::deferred)
        )
            .async_wait(boost::asio::experimental::wait_for_one(), boost::asio::use_awaitable);

    const auto& error = order.front() == 0 ? connection_error : stop_error;
    if (error) {
        co_return evget::Err{HandlerError("waiting for X11 input failed", error.message())};
    }
    co_return evget::Result<void>{};
}

evget::Result<std::unique_ptr<evgetx11::InputHandler>> evgetx11::InputHandlerBuilder::Build(X11Api& x_wrapper) {
    return AnnounceVersion(x_wrapper).transform([&x_wrapper] {
        SetMask(x_wrapper);
        return std::make_unique<InputHandler>(x_wrapper);
    });
}
