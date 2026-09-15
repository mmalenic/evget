/**
 * \file message_window.h
 * \brief The windows message system that passes raw input through a channel thread.
 */

#ifndef EVGETWINDOWS_MESSAGE_WINDOW_H
#define EVGETWINDOWS_MESSAGE_WINDOW_H

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>
#include <windows.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "evget/error.h"
#include "evgetwindows/raw_event.h"

namespace evgetwindows {

/**
 * \brief The channel carrying owned `RawEvent` values.
 */
using RawEventChannel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, RawEvent)>;

/**
 * \brief the channel capacity.
 */
constexpr std::size_t kRawEventChannelCapacity = 8192;
/**
 * \brief the number of values considered when the channel is near capacity.
 */
constexpr std::size_t kChannelNearCapacity = kRawEventChannelCapacity * 4 / 5;

/**
 * \brief The result of queuing an input event into the channel.
 */
enum class EnqueueOutcome : std::uint8_t {
    kSent, ///< Sent and queued
    kDropped, ///< channel full, dropping event
    kIgnored, ///< no raw input event could be extracted from the packet
};

/**
 * \brief The message window that transports `RawEvent` values through a channel.
 */
class MessageWindow {
public:
    /**
     * \brief Construct a message window where the channel finishes the executor.
     * \param executor the executor for the channel
     */
    explicit MessageWindow(const boost::asio::any_io_executor& executor);

    MessageWindow(const MessageWindow&) = delete;
    MessageWindow(MessageWindow&&) noexcept = delete;
    MessageWindow& operator=(const MessageWindow&) = delete;
    MessageWindow& operator=(MessageWindow&&) noexcept = delete;

    ~MessageWindow();

    /**
     * \brief Start the channel and message system.
     * \return a void result, or an error
     */
    [[nodiscard]] evget::Result<void> Start();

    /**
     * \brief Close the channel and join tasks.
     */
    void Stop();

    /**
     * \brief Get the channel events are sent to.
     * \return channel reference
     */
    [[nodiscard]] RawEventChannel& Channel();

    /**
     * \brief Convert a `RAWINPUT` record into an owned `RawEvent`.
     * \param raw the raw input
     * \return the event, or `nullopt` if no raw input event could be extracted
     */
    [[nodiscard]] static std::optional<RawEvent> ToRawEvent(const RAWINPUT& raw);

    /**
     * \brief Convert a HID report at `index` in the `RAWINPUT` record into an owned `RawEvent`.
     * \param raw the raw input
     * \param index the index of the report
     * \return the event, or `nullopt` if not HID or fails validation
     */
    [[nodiscard]] static std::optional<RawEvent> ToRawEventAt(const RAWINPUT& raw, DWORD index);

    /**
     * \brief Convert a `RAWINPUT` record and try to queue it.
     * \param raw the raw input record
     * \return the outcome
     */
    EnqueueOutcome Enqueue(const RAWINPUT& raw);

private:
    using WindowHandle = std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&DestroyWindow)>;

    class WindowClass {
    public:
        WindowClass(const wchar_t* class_name, HINSTANCE instance);
        WindowClass(const WindowClass&) = delete;
        WindowClass(WindowClass&&) = delete;
        WindowClass& operator=(const WindowClass&) = delete;
        WindowClass& operator=(WindowClass&&) = delete;
        ~WindowClass();

    private:
        const wchar_t* class_name_;
        HINSTANCE instance_;
    };

    class RawInput {
    public:
        static evget::Result<std::unique_ptr<RawInput>> Create(HWND target);
        RawInput(const RawInput&) = delete;
        RawInput(RawInput&&) = delete;
        RawInput& operator=(const RawInput&) = delete;
        RawInput& operator=(RawInput&&) = delete;
        ~RawInput();

    private:
        RawInput() = default;
    };

    static LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    void RunPump(std::promise<evget::Result<void>> registration);
    void HandleRawInput(HRAWINPUT input);
    EnqueueOutcome Send(const RawEvent& event);

    std::wstring class_name_;
    RawEventChannel channel_;
    std::vector<std::byte> raw_buffer_;
    std::jthread thread_;
    std::atomic<DWORD> thread_id_{0};
    std::atomic<std::size_t> in_flight_{0};
    std::atomic<std::size_t> dropped_{0};
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_MESSAGE_WINDOW_H
