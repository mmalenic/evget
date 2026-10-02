#include "evgetwindows/message_window.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>
#include <hidusage.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <utility>

#include "evget/error.h"
#include "evgetwindows/hid_usages.h"
#include "evgetwindows/raw_event.h"

namespace {

// A device change is not frequent so if it ever reaches this the consuming channel has stopped.
constexpr std::size_t kMaxPendingDeviceChanges = 1024;

std::wstring MakeClassName() {
    static std::atomic<std::uint64_t> counter{0};
    return std::format(L"evget_message_window_{}", counter.fetch_add(1));
}

std::array<RAWINPUTDEVICE, 4> MakeRawInputDevices(DWORD flags, HWND target) {
    return {
        {{.usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags,
          .hwndTarget = target},
         {.usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags,
          .hwndTarget = target},
         {.usUsagePage = HID_USAGE_PAGE_DIGITIZER,
          .usUsage = HID_USAGE_DIGITIZER_TOUCH_SCREEN,
          .dwFlags = flags,
          .hwndTarget = target},
         {.usUsagePage = HID_USAGE_PAGE_DIGITIZER,
          .usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD,
          .dwFlags = flags,
          .hwndTarget = target}}
    };
}

} // namespace

evgetwindows::MessageWindow::MessageWindow(const boost::asio::any_io_executor& executor)
    : MessageWindow{executor, {}} {}

evgetwindows::MessageWindow::MessageWindow(const boost::asio::any_io_executor& executor, std::function<void()> on_stop)
    : class_name_{MakeClassName()}, on_stop_{std::move(on_stop)}, channel_{executor, kRawEventChannelCapacity} {}

evgetwindows::MessageWindow::~MessageWindow() {
    try {
        Stop();
    } catch (const std::exception& e) {
        spdlog::error("failed to stop message window: {}", e.what());
    }
}

evgetwindows::RawEventChannel& evgetwindows::MessageWindow::Channel() {
    return channel_;
}

evget::Result<void> evgetwindows::MessageWindow::Start() {
    if (thread_.joinable()) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "message window already started"}
        };
    }

    std::promise<evget::Result<void>> registration;
    std::future<evget::Result<void>> registration_result = registration.get_future();

    thread_ = std::jthread{[this, registration = std::move(registration)]() mutable {
        try {
            RunPump(std::move(registration));
        } catch (const std::exception& e) {
            spdlog::error("message pump failed: {}", e.what());
        }
    }};

    return registration_result.get();
}

void evgetwindows::MessageWindow::Stop() {
    RequestStop();

    if (thread_.joinable() && thread_.get_id() != std::this_thread::get_id()) {
        thread_.join();
    }
}

void evgetwindows::MessageWindow::RequestStop() {
    channel_.close();

    const DWORD pump_tid = thread_id_.load();
    if (pump_tid != 0) {
        PostThreadMessageW(pump_tid, WM_QUIT, 0, 0);
    }
}

std::optional<LRESULT> evgetwindows::MessageWindow::HandleStopMessage(UINT message, WPARAM wparam) {
    switch (message) {
        case WM_CLOSE:
            // Needed here because DefWindowProcW would later destroy the input while the process keeps running.
            if (on_stop_) {
                on_stop_();
            }
            return 0;
        case WM_QUERYENDSESSION:
            return TRUE;
        case WM_ENDSESSION:
            // The lParam is an ENDSESSION_* bitmask meaning that any of these should stop.
            if (wparam != FALSE && on_stop_) {
                on_stop_();
            }
            return 0;
        default:
            return std::nullopt;
    }
}

LRESULT CALLBACK evgetwindows::MessageWindow::WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_INPUT) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
        auto* self = reinterpret_cast<MessageWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (self != nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
            self->HandleRawInput(reinterpret_cast<HRAWINPUT>(lparam));
        }
        return 0;
    }
    if (message == WM_INPUT_DEVICE_CHANGE) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
        auto* self = reinterpret_cast<MessageWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (self != nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
            self->EnqueueDeviceChange(GET_DEVICE_CHANGE_WPARAM(wparam), reinterpret_cast<HANDLE>(lparam));
        }
        return 0;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    auto* self = reinterpret_cast<MessageWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (self != nullptr) {
        const std::optional<LRESULT> handled = self->HandleStopMessage(message, wparam);
        if (handled.has_value()) {
            return *handled;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

std::optional<evgetwindows::RawEvent>
evgetwindows::MessageWindow::ToRawEvent(const RAWINPUT& raw, std::size_t packet_size) {
    RawEvent event{};
    event.header = raw.header;
    if (raw.header.dwType == RIM_TYPEMOUSE) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        event.data = raw.data.mouse;
    } else if (raw.header.dwType == RIM_TYPEKEYBOARD) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        event.data = raw.data.keyboard;
    } else {
        return ToRawEventAt(raw, 0, packet_size);
    }
    return event;
}

std::optional<evgetwindows::RawEvent>
evgetwindows::MessageWindow::ToRawEventAt(const RAWINPUT& raw, DWORD index, std::size_t packet_size) {
    if (raw.header.dwType != RIM_TYPEHID) {
        return std::nullopt;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    const RAWHID& hid = raw.data.hid;
    if (hid.dwSizeHid == 0 || hid.dwSizeHid > kHidReportCapacity || index >= hid.dwCount) {
        return std::nullopt;
    }

    // Packet must appear to contain the report as counts come from the device.
    const auto reports_end = static_cast<std::uint64_t>(offsetof(RAWINPUT, data)) + offsetof(RAWHID, bRawData) +
        (static_cast<std::uint64_t>(hid.dwSizeHid) * hid.dwCount);
    if (reports_end > std::min<std::uint64_t>(raw.header.dwSize, packet_size)) {
        return std::nullopt;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    const auto* reports = reinterpret_cast<const std::byte*>(hid.bRawData);

    HidPayload payload{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy_n(reports + (static_cast<std::size_t>(index) * hid.dwSizeHid), hid.dwSizeHid, payload.report.begin());
    payload.size = static_cast<std::uint16_t>(hid.dwSizeHid);

    RawEvent event{};
    event.header = raw.header;
    event.data = payload;

    return event;
}

evgetwindows::RawEvent evgetwindows::MessageWindow::ToDeviceChangeEvent(WPARAM change, HANDLE device) {
    RawEvent event{};
    event.header.hDevice = device;
    event.data = DeviceChange{.device = device, .arrival = change == GIDC_ARRIVAL};

    return event;
}

bool evgetwindows::MessageWindow::TrySend(const RawEvent& event) {
    // Count before sending so the receive can't decrement ahead.
    const std::size_t buffered = in_flight_.fetch_add(1, std::memory_order_relaxed) + 1;
    if (!channel_.try_send(boost::system::error_code{}, event)) {
        in_flight_.fetch_sub(1, std::memory_order_relaxed);
        return false;
    }

    if (buffered >= kChannelNearCapacity && !near_capacity_warned_.exchange(true, std::memory_order_relaxed)) {
        spdlog::warn("input channel reached {} buffered events", kChannelNearCapacity);
    }
    return true;
}

boost::asio::awaitable<std::tuple<boost::system::error_code, evgetwindows::RawEvent>>
evgetwindows::MessageWindow::Receive() {
    auto received = co_await channel_.async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
    if (!std::get<0>(received) && in_flight_.fetch_sub(1, std::memory_order_relaxed) - 1 < kChannelNearCapacity / 2) {
        near_capacity_warned_.store(false, std::memory_order_relaxed);
    }

    co_return received;
}

std::size_t evgetwindows::MessageWindow::Buffered() const {
    return in_flight_.load(std::memory_order_relaxed);
}

evgetwindows::EnqueueOutcome evgetwindows::MessageWindow::EnqueueEvent(const RawEvent& event) {
    if (TrySend(event)) {
        return EnqueueOutcome::kSent;
    }

    if (!channel_.is_open()) {
        return EnqueueOutcome::kClosed;
    }

    if (dropped_.fetch_add(1, std::memory_order_relaxed) % kChannelNearCapacity == 0) {
        spdlog::error("input channel full, dropping events");
    }
    return EnqueueOutcome::kDropped;
}

evgetwindows::EnqueueOutcome evgetwindows::MessageWindow::Enqueue(const RAWINPUT& raw, std::size_t packet_size) {
    DrainDeviceChanges();

    if (raw.header.dwType == RIM_TYPEHID) {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        const DWORD count = raw.data.hid.dwCount;
        const DWORD report_size = raw.data.hid.dwSizeHid;
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)

        // A device which does not fit reports has no touch device.
        if (report_size > kHidReportCapacity && oversized_logged_.insert(raw.header.hDevice).second) {
            spdlog::warn(
                "hid report of {} bytes exceeds the {} byte capacity, dropping reports",
                report_size,
                kHidReportCapacity
            );
        }

        auto outcome = EnqueueOutcome::kIgnored;
        for (DWORD index = 0; index < count; ++index) {
            const std::optional<RawEvent> report = ToRawEventAt(raw, index, packet_size);
            if (!report.has_value()) {
                continue;
            }

            const EnqueueOutcome sent = EnqueueEvent(*report);
            if (sent == EnqueueOutcome::kDropped || outcome != EnqueueOutcome::kDropped) {
                outcome = sent;
            }
        }

        return outcome;
    }

    const std::optional<RawEvent> event = ToRawEvent(raw, packet_size);
    if (!event.has_value()) {
        return EnqueueOutcome::kIgnored;
    }

    return EnqueueEvent(*event);
}

void evgetwindows::MessageWindow::EnqueueDeviceChange(WPARAM change, HANDLE device) {
    if (change != GIDC_ARRIVAL && change != GIDC_REMOVAL) {
        return;
    }

    device_changes_.push_back(ToDeviceChangeEvent(change, device));
    if (device_changes_.size() > kMaxPendingDeviceChanges) {
        spdlog::error("device change reached {} events, dropping events", kMaxPendingDeviceChanges);
        device_changes_.pop_front();
    }

    DrainDeviceChanges();
}

std::size_t evgetwindows::MessageWindow::PendingDeviceChanges() const {
    return device_changes_.size();
}

void evgetwindows::MessageWindow::DrainDeviceChanges() {
    while (!device_changes_.empty() && TrySend(device_changes_.front())) {
        device_changes_.pop_front();
    }
}

void evgetwindows::MessageWindow::HandleRawInput(HRAWINPUT input) {
    UINT size = 0;
    // The call is a success when it returns 0.
    if (GetRawInputData(input, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0) {
        return;
    }

    if (raw_buffer_.size() < size) {
        raw_buffer_.resize(size);
    }

    if (GetRawInputData(input, RID_INPUT, raw_buffer_.data(), &size, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
        return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    Enqueue(*reinterpret_cast<const RAWINPUT*>(raw_buffer_.data()), size);
}

evgetwindows::MessageWindow::WindowClass::WindowClass(const wchar_t* class_name, HINSTANCE instance)
    : class_name_{class_name}, instance_{instance} {}

evgetwindows::MessageWindow::WindowClass::~WindowClass() {
    if (UnregisterClassW(class_name_, instance_) == FALSE) {
        spdlog::error("UnregisterClassW failed: {}", GetLastError());
    }
}

evget::Result<std::unique_ptr<evgetwindows::MessageWindow::RawInput>> evgetwindows::MessageWindow::RawInput::Create(
    HWND target
) {
    const std::array<RAWINPUTDEVICE, 4> devices = MakeRawInputDevices(RIDEV_INPUTSINK | RIDEV_DEVNOTIFY, target);
    SetLastError(ERROR_SUCCESS);
    if (RegisterRawInputDevices(devices.data(), static_cast<UINT>(devices.size()), sizeof(RAWINPUTDEVICE)) == FALSE ||
        GetLastError() != ERROR_SUCCESS) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("RegisterRawInputDevices failed: {}", GetLastError())}
        };
    }
    return std::unique_ptr<RawInput>(new RawInput{});
}

evgetwindows::MessageWindow::RawInput::~RawInput() {
    // RIDEV_REMOVE requires hwndTarget == nullptr.
    const std::array<RAWINPUTDEVICE, 4> devices = MakeRawInputDevices(RIDEV_REMOVE, nullptr);
    if (RegisterRawInputDevices(devices.data(), static_cast<UINT>(devices.size()), sizeof(RAWINPUTDEVICE)) == FALSE) {
        spdlog::error("RegisterRawInputDevices RIDEV_REMOVE failed: {}", GetLastError());
    }
}

void evgetwindows::MessageWindow::RunPump(std::promise<evget::Result<void>> registration) {
    thread_id_.store(GetCurrentThreadId(), std::memory_order_release);

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = &MessageWindow::WndProc;
    window_class.hInstance = GetModuleHandleW(nullptr);
    window_class.lpszClassName = class_name_.c_str();
    if (RegisterClassExW(&window_class) == 0) {
        registration.set_value(
            evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError,
                 .message = std::format("RegisterClassExW failed: {}", GetLastError())}
            }
        );
        return;
    }
    // Constructed before the window so it unregisters after DestroyWindow.
    const WindowClass class_guard{class_name_.c_str(), window_class.hInstance};

    // This creates hidden top-level window in order to support task manager end process.
    // NOLINTNEXTLINE(misc-misplaced-const)
    const HWND raw_window = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        class_name_.c_str(),
        L"evget",
        0,
        0,
        0,
        0,
        0,
        nullptr,
        nullptr,
        window_class.hInstance,
        nullptr
    );
    if (raw_window == nullptr) {
        registration.set_value(
            evget::Err{
                {.error_type = evget::ErrorType::kEventHandlerError,
                 .message = std::format("CreateWindowExW failed: {}", GetLastError())}
            }
        );
        return;
    }
    // DestroyWindow must run on the owning thread.
    const WindowHandle window{raw_window, &DestroyWindow};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    SetWindowLongPtrW(raw_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    auto raw_input = RawInput::Create(raw_window);
    if (!raw_input.has_value()) {
        registration.set_value(evget::Err{raw_input.error()});
        return;
    }
    const std::unique_ptr<RawInput> raw_input_guard = std::move(*raw_input);

    registration.set_value(evget::Result<void>{});

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
