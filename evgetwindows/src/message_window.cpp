#include "evgetwindows/message_window.h"

#include <boost/asio/any_io_executor.hpp>
#include <hidusage.h>
#include <spdlog/spdlog.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <exception>
#include <format>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include "evget/error.h"
#include "evgetwindows/raw_event.h"

std::wstring evgetwindows::MessageWindow::MakeClassName() {
    static std::atomic<std::uint64_t> counter{0};
    return std::format(L"evget_message_window_{}", counter.fetch_add(1));
}

std::array<RAWINPUTDEVICE, 2> evgetwindows::MessageWindow::MakeRawInputDevices(DWORD flags, HWND target) {
    return {
        {{.usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags,
          .hwndTarget = target},
         {.usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags,
          .hwndTarget = target}}
    };
}

evgetwindows::MessageWindow::MessageWindow(const boost::asio::any_io_executor& executor)
    : class_name_{MakeClassName()}, channel_{executor, kRawEventChannelCapacity} {}

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

    thread_ =
        std::jthread{[this, registration = std::move(registration)]() mutable { RunPump(std::move(registration)); }};

    return registration_result.get();
}

void evgetwindows::MessageWindow::Stop() {
    channel_.close();

    const DWORD pump_tid = thread_id_.load();
    if (pump_tid != 0) {
        PostThreadMessageW(pump_tid, WM_QUIT, 0, 0);
    }

    if (thread_.joinable()) {
        thread_.join();
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
    return DefWindowProcW(window, message, wparam, lparam);
}

std::optional<evgetwindows::RawEvent> evgetwindows::MessageWindow::ToRawEvent(const RAWINPUT& raw) {
    RawEvent event{};
    event.header = raw.header;
    if (raw.header.dwType == RIM_TYPEMOUSE) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        event.data = raw.data.mouse;
    } else if (raw.header.dwType == RIM_TYPEKEYBOARD) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
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
    const std::array<RAWINPUTDEVICE, 2> devices = MakeRawInputDevices(RIDEV_INPUTSINK | RIDEV_DEVNOTIFY, target);
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
    const std::array<RAWINPUTDEVICE, 2> devices = MakeRawInputDevices(RIDEV_REMOVE, nullptr);
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

    // NOLINTNEXTLINE(misc-misplaced-const)
    const HWND raw_window = CreateWindowExW(
        0,
        class_name_.c_str(),
        L"evget",
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
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
