/**
 * \file message_window.h
 * \brief The windows message system that passes raw input through a channel thread.
 */

#ifndef EVGETWINDOWS_MESSAGE_WINDOW_H
#define EVGETWINDOWS_MESSAGE_WINDOW_H

#include <windows.h>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <cstddef>
#include <future>
#include <memory>
#include <optional>
#include <thread>
#include <type_traits>

#include "evget/error.h"
#include "evgetwindows/raw_event.h"

namespace evgetwindows {

/**
 * \brief The channel carrying owned `RawEvent` values.
 */
using RawEventChannel =
    boost::asio::experimental::concurrent_channel<void(boost::system::error_code, RawEvent)>;

constexpr std::size_t kRawEventChannelCapacity = 8192;
constexpr std::size_t kHighWaterMark = 6144;

/**
 * \brief The outcome of queuing an input event into the channel.
 */
enum class EnqueueOutcome {
    kSent,    ///< Sent and queued
    kDropped, ///< channel full, counted as dropped
    kIgnored, ///< event type is neither mouse nor keyboard
};

/**
 * \brief The message window system that transports `RawEvent` values through a channel.
 */
class MessageWindow {
public:
    /**
     * \brief Construct a message window where the channel finishes the executor.
     * \param executor the executor for the channel
     */
    explicit MessageWindow(boost::asio::any_io_executor executor);

    MessageWindow(const MessageWindow&) = delete;
    MessageWindow(MessageWindow&&) noexcept = delete;
    MessageWindow& operator=(const MessageWindow&) = delete;
    MessageWindow& operator=(MessageWindow&&) noexcept = delete;

    ~MessageWindow();

    /**
     * \brief Launch the channel and message system by creating an invisible window.
     * \return a void result, or an error if window creation or device registration fails
     */
    [[nodiscard]] evget::Result<void> Start();

    /**
     * \brief Close the channel and join any leftover tasks.
     */
    void Stop();

    /**
     * \brief Get the channel events are sent to.
     * \return channel reference
     */
    [[nodiscard]] RawEventChannel& Channel();

    /**
     * \brief Classify a `RAWINPUT` record into an owned `RawEvent`.
     * \param raw the raw input record
     * \return the event, or `nullopt` if it is neither a mouse nor keyboard event
     */
    [[nodiscard]] static std::optional<RawEvent> ToRawEvent(const RAWINPUT& raw);

    /**
     * \brief Classify a `RAWINPUT` record and try to queue it.
     * \param raw the raw input record
     * \return the queuing outcome
     */
    EnqueueOutcome Enqueue(const RAWINPUT& raw);

private:
    using WindowHandle = std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)>;

    struct FinishGuard {
        std::shared_ptr<std::promise<void>> finished;
        ~FinishGuard();
    };

    static LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    void RunPump(std::promise<evget::Result<void>> registration, std::shared_ptr<std::promise<void>> finished);
    void HandleRawInput(HRAWINPUT input);

    RawEventChannel channel_;
    std::jthread thread_;
    std::atomic<DWORD> thread_id_{0};
    std::atomic<std::size_t> in_flight_{0};
    std::atomic<std::size_t> dropped_{0};
    std::future<void> finished_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_MESSAGE_WINDOW_H
