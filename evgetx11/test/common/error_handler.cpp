#include "common/error_handler.h"

#include <X11/Xlib.h>

#include <cstddef>
#include <span>
#include <string_view>

#include "evgetx11/error_router.h"

XErrorHandler& test::InstalledErrorHandler() {
    static XErrorHandler installed = nullptr;
    return installed;
}

XErrorHandler test::SetErrorHandler(XErrorHandler handler) {
    auto previous = InstalledErrorHandler();
    InstalledErrorHandler() = handler;
    return previous;
}

XIOErrorHandler& test::InstalledIoErrorHandler() {
    static XIOErrorHandler installed = nullptr;
    return installed;
}

XIOErrorHandler test::SetIoErrorHandler(XIOErrorHandler handler) {
    auto previous = InstalledIoErrorHandler();
    InstalledIoErrorHandler() = handler;
    return previous;
}

int test::HostIoErrorHandler(Display* /* display */) {
    return kHostHandled;
}

int test::RoutedIoErrorHandler(Display* /* display */) {
    return 0;
}

int test::HostErrorHandler(Display* /* display */, XErrorEvent* /* event */) {
    return kHostHandled;
}

int test::RoutedErrorHandler(Display* /* display */, XErrorEvent* /* event */) {
    return 0;
}

int test::LaterHostErrorHandler(Display* /* display */, XErrorEvent* /* event */) {
    return 0;
}

evgetx11::ErrorRouter*& test::ChainedRouter() {
    // NOLINTNEXTLINE(misc-const-correctness)
    static evgetx11::ErrorRouter* router = nullptr;
    return router;
}

int& test::ChainingCalls() {
    static int calls = 0;
    return calls;
}

int test::ChainingErrorHandler(Display* display, XErrorEvent* event) {
    ChainingCalls()++;
    return ChainedRouter()->Dispatch(display, event);
}

int test::FakeErrorText(Display* /* display */, int /* code */, char* buffer, int length) {
    constexpr std::string_view kText{"fake error"};
    const std::span output{buffer, static_cast<std::size_t>(length)};
    const auto copied = kText.copy(output.data(), output.size() - 1);
    output[copied] = '\0';
    return 0;
}

Display* test::FakeDisplays::At(std::size_t index) {
    // Only the address identifies a display; nothing reads through it.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Display*>(&storage_.at(index));
}

XErrorEvent test::CreateXErrorEvent(unsigned char error_code, unsigned char request_code) {
    return {
        .type = 0,
        .display = nullptr,
        .resourceid = 0,
        .serial = 0,
        .error_code = error_code,
        .request_code = request_code,
        .minor_code = 0
    };
}
