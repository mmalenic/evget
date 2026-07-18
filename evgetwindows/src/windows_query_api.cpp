#include "evgetwindows/windows_query_api.h"

#include <spdlog/spdlog.h>
#include <windows.h>

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace {

constexpr int kCharacterBufferSize = 8;
constexpr SHORT kToggleStateMask = 0x1;

std::string Utf16ToUtf8(std::wstring_view wide) {
    if (wide.empty()) {
        return {};
    }

    const int size =
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        spdlog::warn("WideCharToMultiByte sizing failed");
        return {};
    }

    std::string out(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), out.data(), size, nullptr, nullptr);
    return out;
}

struct MonitorSearch {
    HMONITOR target;
    int index;
    int found;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC /*hdc*/, LPRECT /*rect*/, LPARAM data) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* search = reinterpret_cast<MonitorSearch*>(data);
    if (monitor == search->target) {
        search->found = search->index;
        return FALSE;
    }

    ++search->index;
    return TRUE;
}

int MonitorIndex(HMONITOR monitor) {
    if (monitor == nullptr) {
        return 0;
    }

    MonitorSearch search{.target = monitor, .index = 0, .found = 0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    EnumDisplayMonitors(nullptr, nullptr, &MonitorEnumProc, reinterpret_cast<LPARAM>(&search));
    return search.found;
}

} // namespace

std::optional<std::string>
evgetwindows::WindowsQuery::CharacterFor(UINT vk, UINT scan_code, const std::array<BYTE, 256>& key_state) {
    HWND foreground = GetForegroundWindow();
    const HKL layout = (foreground != nullptr)
        ? GetKeyboardLayout(GetWindowThreadProcessId(foreground, nullptr))
        : GetKeyboardLayout(0);

    std::array<wchar_t, kCharacterBufferSize> buffer{};
    // The no-state-change flag keeps ToUnicodeEx from corrupting the user's live keys.
    const int written = ToUnicodeEx(
        vk,
        scan_code,
        key_state.data(),
        buffer.data(),
        static_cast<int>(buffer.size()),
        kToUnicodeNoKeyStateChange,
        layout
    );
    if (written <= 0) {
        return std::nullopt;
    }

    return Utf16ToUtf8(std::wstring_view{buffer.data(), static_cast<size_t>(written)});
}

std::optional<std::string> evgetwindows::WindowsQuery::DeviceName(HANDLE device) {
    if (device == nullptr) {
        return std::nullopt;
    }

    UINT size = 0;
    if (GetRawInputDeviceInfoW(device, RIDI_DEVICENAME, nullptr, &size) != 0) {
        spdlog::warn("RIDI_DEVICENAME sizing query failed");
        return std::nullopt;
    }

    std::wstring name(size, L'\0');
    const UINT written = GetRawInputDeviceInfoW(device, RIDI_DEVICENAME, name.data(), &size);
    if (written == static_cast<UINT>(-1)) {
        spdlog::warn("RIDI_DEVICENAME data query failed");
        return std::nullopt;
    }

    name.resize(written);
    if (!name.empty() && name.back() == L'\0') {
        name.pop_back();
    }

    return Utf16ToUtf8(name);
}

std::optional<evgetwindows::FocusWindowInfo> evgetwindows::WindowsQuery::FocusWindow() {
    HWND window = GetForegroundWindow();
    if (window == nullptr) {
        return std::nullopt;
    }

    FocusWindowInfo info{};

    const int length = GetWindowTextLengthW(window);
    if (length > 0) {
        std::wstring title(static_cast<size_t>(length) + 1, L'\0');
        const int copied = GetWindowTextW(window, title.data(), length + 1);
        title.resize(static_cast<size_t>(copied));
        info.name = Utf16ToUtf8(title);
    }

    RECT rect{};
    if (GetWindowRect(window, &rect) != 0) {
        info.position_x = static_cast<double>(rect.left);
        info.position_y = static_cast<double>(rect.top);
        info.width = static_cast<double>(rect.right - rect.left);
        info.height = static_cast<double>(rect.bottom - rect.top);
    }

    info.screen = MonitorIndex(MonitorFromWindow(window, MONITOR_DEFAULTTONULL));
    return info;
}

bool evgetwindows::WindowsQuery::ToggleState(int vk) {
    return (GetKeyState(vk) & kToggleStateMask) != 0;
}
