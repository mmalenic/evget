/**
 * \file message_window.h
 * \brief The windows message system that passes raw input through a channel thread.
 */

#ifndef EVGETWINDOWS_MESSAGE_WINDOW_H
#define EVGETWINDOWS_MESSAGE_WINDOW_H

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>
#include <windows.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_set>
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
    kClosed, ///< channel closed by a stop
};

/**
 * \brief The message window that transports `RawEvent` values through a channel.
 */
class MessageWindow {
public:
    /**
     * \brief Construct a message window.
     * \param executor the executor for the channel
     */
    explicit MessageWindow(const boost::asio::any_io_executor& executor);

    /**
     * \brief Construct a message window where the channel finishes the executor.
     * \param executor the executor for the channel
     * \param on_stop called on the thread for a close or end request
     */
    MessageWindow(const boost::asio::any_io_executor& executor, std::function<void()> on_stop);

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
     * \brief Request a stop and join the pump thread.
     *
     * \note This and the destructor must not run on the message thread, which should call `RequestStop` instead.
     */
    void Stop();

    /**
     * \brief Close the channel and ask for it to exit without waiting.
     */
    void RequestStop();

    /**
     * \brief Handle a close or end message.
     * \param message the window message
     * \param wparam the message `WPARAM`
     * \return the message result, or `nullopt` if the message is not a stop message
     */
    [[nodiscard]] std::optional<LRESULT> HandleStopMessage(UINT message, WPARAM wparam);

    /**
     * \brief Get the channel events are sent to.
     * \return channel reference
     */
    [[nodiscard]] RawEventChannel& Channel();

    /**
     * \brief Receive the next event from the channel and count it as no longer buffered.
     * \return the error code and the event
     */
    [[nodiscard]] boost::asio::awaitable<std::tuple<boost::system::error_code, RawEvent>> Receive();

    /**
     * \brief The number of events sent to the channel and not yet received through `Receive`.
     * \return the buffered event count
     */
    [[nodiscard]] std::size_t Buffered() const;

    /**
     * \brief Convert a `RAWINPUT` record into an owned `RawEvent`.
     * \param raw the raw input
     * \param packet_size the number of bytes within `raw` that is the target
     * \return the event, or `nullopt` if no raw input event could be extracted
     */
    [[nodiscard]] static std::optional<RawEvent> ToRawEvent(const RAWINPUT& raw, std::size_t packet_size);

    /**
     * \brief Convert a HID report at `index` in the `RAWINPUT` record into an owned `RawEvent`.
     * \param raw the raw input
     * \param index the index of the report
     * \param packet_size the number of bytes within `raw` that is the target
     * \return the event, or `nullopt` if not HID or fails validation
     *
     * The packet states its own length, so the reports are bounded by the caller's extent as well as by that.
     */
    [[nodiscard]] static std::optional<RawEvent>
    ToRawEventAt(const RAWINPUT& raw, DWORD index, std::size_t packet_size);

    /**
     * \brief Build a `RawEvent` for a device arrival or removal.
     * \param change the device change code
     * \param device the device handle
     * \return the event
     */
    [[nodiscard]] static RawEvent ToDeviceChangeEvent(WPARAM change, HANDLE device);

    /**
     * \brief Convert a `RAWINPUT` record and try to queue it.
     * \param raw the raw input record
     * \param packet_size the number of bytes within `raw` that is the target
     * \return the outcome
     */
    EnqueueOutcome Enqueue(const RAWINPUT& raw, std::size_t packet_size);

    /**
     * \brief Queue a device for arrival or removal.
     * \param change the device change code
     * \param device the device handle
     *
     * If the removal is lost it won't be recovered and a device change is not dropped.
     */
    void EnqueueDeviceChange(WPARAM change, HANDLE device);

    /**
     * \brief The number of device changes available.
     * \return the device change count
     */
    [[nodiscard]] std::size_t PendingDeviceChanges() const;

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
    void DrainDeviceChanges();
    EnqueueOutcome EnqueueEvent(const RawEvent& event);
    bool TrySend(const RawEvent& event);

    std::wstring class_name_;
    std::function<void()> on_stop_;
    RawEventChannel channel_;
    std::vector<std::byte> raw_buffer_;
    std::deque<RawEvent> device_changes_;
    std::jthread thread_;
    std::atomic<DWORD> thread_id_{0};
    std::atomic<std::size_t> in_flight_{0};
    std::atomic<bool> near_capacity_warned_{false};
    std::atomic<std::size_t> dropped_{0};
    std::unordered_set<HANDLE> oversized_logged_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_MESSAGE_WINDOW_H
