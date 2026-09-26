#include "evgetwindows/elevation.h"

#include <spdlog/spdlog.h>
#include <windows.h>

#include <format>
#include <string>

#include "evget/error.h"

evget::Result<bool> evgetwindows::IsElevated() {
    TOKEN_ELEVATION elevation{};
    DWORD size = 0;
    if (GetTokenInformation(GetCurrentProcessToken(), TokenElevation, &elevation, sizeof(elevation), &size) == FALSE) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("GetTokenInformation failed: {}", GetLastError())}
        };
    }
    return elevation.TokenIsElevated != 0;
}

std::string evgetwindows::ElevationMessage(bool elevated) {
    if (elevated) {
        return "running elevated, input to elevated windows is captured";
    }
    return "running without elevation, input sent to elevated windows is not captured, run evget as administrator";
}

void evgetwindows::LogElevation() {
    auto elevated = IsElevated();
    if (!elevated.has_value()) {
        spdlog::error("{}", elevated.error());
        return;
    }
    spdlog::info("{}", ElevationMessage(*elevated));
}
