#include "evgetwindows/message_window.h"

#include <hidusage.h>

#include <spdlog/spdlog.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <format>
#include <future>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include "evget/error.h"
#include "evgetwindows/raw_event.h"

namespace {
constexpr wchar_t kWindowClassName[] = L"evget_message_window";
constexpr std::chrono::seconds kJoinTimeout{2};
} // namespace

evgetwindows::MessageWindow::MessageWindow(boost::asio::any_io_executor executor)
    : channel_{std::move(executor), kRawEventChannelCapacity} {}

evgetwindows::MessageWindow::~MessageWindow() {
    Stop();
}

evgetwindows::RawEventChannel& evgetwindows::MessageWindow::Channel() {
    return channel_;
}

evget::Result<void> evgetwindows::MessageWindow::Start() {
    std::promise<evget::Result<void>> registration;
    std::future<evget::Result<void>> registration_result = registration.get_future();

    auto finished = std::make_shared<std::promise<void>>();
    finished_ = finished->get_future();

    thread_ = std::jthread{[this, registration = std::move(registration), finished]() mutable {
        RunPump(std::move(registration), finished);
    }};

    return registration_result.get();
}

void evgetwindows::MessageWindow::Stop() {
    channel_.close();

    const DWORD pump_tid = thread_id_.load(std::memory_order_acquire);
    if (pump_tid != 0) {
        PostThreadMessageW(pump_tid, WM_QUIT, 0, 0);
    }

    if (!thread_.joinable()) {
        return;
    }
    if (finished_.valid() && finished_.wait_for(kJoinTimeout) == std::future_status::ready) {
        thread_.join();
    } else {
        spdlog::error("message window did not exit within {}s", kJoinTimeout.count());
        thread_.detach();
    }
}

LRESULT CALLBACK evgetwindows::MessageWindow::WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_INPUT) {
        auto* self = reinterpret_cast<MessageWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (self != nullptr) {
            self->HandleRawInput(reinterpret_cast<HRAWINPUT>(lparam));
        }
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

std::optional<evgetwindows::RawEvent> evgetwindows::MessageWindow::ToRawEvent(const RAWINPUT& raw) {
    RawEvent event{};
    event.header = raw.header;
    if (raw.header.dwType == RIM_TYPEMOUSE) {
        event.data = raw.data.mouse;
    } else if (raw.header.dwType == RIM_TYPEKEYBOARD) {
        event.data = raw.data.keyboard;
    } else {
        return std::nullopt;
    }
    return event;
}

evgetwindows::EnqueueOutcome evgetwindows::MessageWindow::Enqueue(const RAWINPUT& raw) {
    const std::optional<RawEvent> event = ToRawEvent(raw);
    if (!event.has_value()) {
        return EnqueueOutcome::kIgnored;
    }

    if (channel_.try_send(boost::system::error_code{}, *event)) {
        if (in_flight_.fetch_add(1, std::memory_order_relaxed) + 1 == kHighWaterMark) {
            spdlog::warn("input channel reached {} buffered events", kHighWaterMark);
        }
        return EnqueueOutcome::kSent;
    }

    if (dropped_.fetch_add(1, std::memory_order_relaxed) % kHighWaterMark == 0) {
        spdlog::error("input channel full, dropping events");
    }
    return EnqueueOutcome::kDropped;
}

void evgetwindows::MessageWindow::HandleRawInput(HRAWINPUT input) {
    RAWINPUT raw{};
    UINT size = sizeof(raw);
    if (GetRawInputData(input, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
        return;
    }
    Enqueue(raw);
}

evgetwindows::MessageWindow::FinishGuard::~FinishGuard() {
    finished->set_value();
}

void evgetwindows::MessageWindow::RunPump(
    std::promise<evget::Result<void>> registration,
    std::shared_ptr<std::promise<void>> finished
) {
    const FinishGuard finish_guard{std::move(finished)};

    thread_id_.store(GetCurrentThreadId(), std::memory_order_release);

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = &MessageWindow::WndProc;
    window_class.hInstance = GetModuleHandleW(nullptr);
    window_class.lpszClassName = kWindowClassName;
    if (RegisterClassExW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        registration.set_value(evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("RegisterClassExW failed: {}", GetLastError())}
        });
        return;
    }

    const HWND raw_window = CreateWindowExW(
        0, kWindowClassName, L"evget", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, window_class.hInstance, nullptr
    );
    if (raw_window == nullptr) {
        registration.set_value(evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("CreateWindowExW failed: {}", GetLastError())}
        });
        return;
    }
    // DestroyWindow must run on the owning thread.
    const WindowHandle window{raw_window, &DestroyWindow};

    SetWindowLongPtrW(raw_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    std::array<RAWINPUTDEVICE, 2> devices{};
    devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    devices[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
    devices[0].hwndTarget = raw_window;
    devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    devices[1].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
    devices[1].hwndTarget = raw_window;

    SetLastError(ERROR_SUCCESS);
    if (RegisterRawInputDevices(devices.data(), devices.size(), sizeof(RAWINPUTDEVICE)) == FALSE
        || GetLastError() != ERROR_SUCCESS) {
        registration.set_value(evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("RegisterRawInputDevices failed: {}", GetLastError())}
        });
        return;
    }

    registration.set_value(evget::Result<void>{});

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
