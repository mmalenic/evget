#include "evgetwindows/hid_query_api.h"

#include <spdlog/spdlog.h>
#include <windows.h>

#include <cstddef>
#include <optional>
#include <span>
#include <utility>

#include "evget/event/device_type.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_usages.h"

namespace {

evget::DeviceType TopLevelCollectionType(USHORT usage_page, USHORT usage) {
    if (usage_page != HID_USAGE_PAGE_DIGITIZER) {
        return evget::DeviceType::kUnknown;
    }
    if (usage == HID_USAGE_DIGITIZER_TOUCH_SCREEN) {
        return evget::DeviceType::kTouchscreen;
    }
    if (usage == HID_USAGE_DIGITIZER_TOUCH_PAD) {
        return evget::DeviceType::kTouchpad;
    }
    return evget::DeviceType::kUnknown;
}

} // namespace

evget::DeviceType evgetwindows::HidQuery::ClassifyDevice(HANDLE device) {
    // This is allowed to be null, e.g. for touchpad emulation.
    if (device == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    if (const auto cached = device_types_.find(device); cached != device_types_.end()) {
        return cached->second;
    }

    RID_DEVICE_INFO info{};
    info.cbSize = sizeof(info);
    UINT size = sizeof(info);
    if (std::cmp_equal(GetRawInputDeviceInfoW(device, RIDI_DEVICEINFO, &info, &size), -1)) {
        spdlog::warn("device info query failed");
        return evget::DeviceType::kUnknown;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    const evget::DeviceType type = info.dwType == RIM_TYPEHID
        ? TopLevelCollectionType(info.hid.usUsagePage, info.hid.usUsage)
        : evget::DeviceType::kUnknown;

    device_types_.emplace(device, type);

    return type;
}

std::optional<evgetwindows::HidAxisRange> evgetwindows::HidQuery::AxisRange(HANDLE /*device*/) {
    // Resolved once the preparsed data value caps are read.
    return std::nullopt;
}

std::optional<evgetwindows::HidReport>
evgetwindows::HidQuery::DecodeReport(HANDLE /*device*/, std::span<const std::byte> /*report*/) {
    // Resolved once the preparsed data report parse is in place.
    return std::nullopt;
}

void evgetwindows::HidQuery::EvictDevice(HANDLE device) {
    device_types_.erase(device);
}
