#include "evgetwindows/windows_query_api.h"

#include <spdlog/spdlog.h>
#include <windows.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr UINT kToUnicodeNoKeyStateChange = 0x4;
constexpr int kCharacterBufferSize = 8;

std::string Utf16ToUtf8(std::wstring_view wide) {
    if (wide.empty()) {
        return {};
    }

    const int size =
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        spdlog::warn("WideCharToMultiByte size check failed");
        return {};
    }

    std::string out(static_cast<size_t>(size), '\0');
    const int converted =
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), out.data(), size, nullptr, nullptr);
    if (converted <= 0) {
        spdlog::warn("WideCharToMultiByte conversion failed");
        return {};
    }

    out.resize(static_cast<size_t>(converted));
    return out;
}

std::optional<std::string> MonitorDevice(HMONITOR monitor) {
    if (monitor == nullptr) {
        return std::nullopt;
    }

    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (GetMonitorInfoW(monitor, &info) == 0) {
        spdlog::warn("monitor info query failed");
        return std::nullopt;
    }

    return Utf16ToUtf8(info.szDevice);
}

} // namespace

std::optional<std::string>
evgetwindows::WindowsQuery::CharacterFor(UINT key, UINT scan_code, const std::array<BYTE, kKeyStateSize>& key_state) {
    HWND foreground = GetForegroundWindow();
    HKL layout = foreground != nullptr
        ? GetKeyboardLayout(GetWindowThreadProcessId(foreground, nullptr))
        : GetKeyboardLayout(0);

    std::array<wchar_t, kCharacterBufferSize> buffer{};
    // The flag keeps ToUnicodeEx from overriding the user key state.
    const int written = ToUnicodeEx(
        key,
        scan_code,
        key_state.data(),
        buffer.data(),
        buffer.size(),
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
        spdlog::warn("device name sizing call failed");
        return std::nullopt;
    }

    std::wstring name(size, L'\0');
    const UINT written = GetRawInputDeviceInfoW(device, RIDI_DEVICENAME, name.data(), &size);
    if (std::cmp_equal(written, -1)) {
        spdlog::warn("device name data call failed");
        return std::nullopt;
    }

    // Remove any excess bytes.
    name.erase(std::ranges::find(name, L'\0'), name.end());

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

    return info;
}

std::optional<std::string> evgetwindows::WindowsQuery::Screen() {
    POINT cursor{};
    if (GetCursorPos(&cursor) == 0) {
        spdlog::warn("cursor position query failed");
        return std::nullopt;
    }

    return MonitorDevice(MonitorFromPoint(cursor, MONITOR_DEFAULTTONULL));
}

bool evgetwindows::WindowsQuery::ToggleState(int key) {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (GetKeyState(key) & kToggleBit) != 0;
}
