#include "evgetwindows/hid_query_api.h"

#include <hidsdi.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>
#include <windows.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <set>
#include <span>
#include <utility>
#include <vector>

#include "evget/event/device_type.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_usages.h"

namespace {

/**
 * \brief The parser status.
 */
enum class FieldOutcome : std::uint8_t {
    kRead, ///< the field was decoded
    kAbsent, ///< the field is not in this report
    kFailed, ///< the parse failed.
};

FieldOutcome Classify(NTSTATUS status) {
    if (status == HIDP_STATUS_SUCCESS) {
        return FieldOutcome::kRead;
    }
    // An optional usage and a report id has fields that are expected and not a failure.
    if (status == HIDP_STATUS_USAGE_NOT_FOUND || status == HIDP_STATUS_INCOMPATIBLE_REPORT_ID) {
        return FieldOutcome::kAbsent;
    }
    return FieldOutcome::kFailed;
}

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

PHIDP_PREPARSED_DATA AsPreparsed(std::span<const std::byte> preparsed) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<PHIDP_PREPARSED_DATA>(const_cast<std::byte*>(preparsed.data()));
}

PCHAR AsReport(std::span<const std::byte> report) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<PCHAR>(const_cast<std::byte*>(report.data()));
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
bool ValueCapsCover(const HIDP_VALUE_CAPS& caps, USAGE usage) {
    if (caps.IsRange != FALSE) {
        return usage >= caps.Range.UsageMin && usage <= caps.Range.UsageMax;
    }
    return caps.NotRange.Usage == usage;
}

bool ButtonCapsCover(const HIDP_BUTTON_CAPS& caps, USAGE usage) {
    if (caps.IsRange != FALSE) {
        return usage >= caps.Range.UsageMin && usage <= caps.Range.UsageMax;
    }
    return caps.NotRange.Usage == usage;
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access)

bool Contains(std::span<const USAGE> usages, USAGE usage) {
    return std::ranges::find(usages, usage) != usages.end();
}

/**
 * \brief The context for the parser when reading the HID report.
 */
struct ParseContext {
    PHIDP_PREPARSED_DATA parsed;
    PCHAR bytes;
    ULONG length;
};

FieldOutcome ReadValue(const ParseContext& context, USAGE page, USHORT collection, USAGE usage, ULONG& out) {
    return Classify(
        HidP_GetUsageValue(HidP_Input, page, collection, usage, &out, context.parsed, context.bytes, context.length)
    );
}

FieldOutcome
ReadUsages(const ParseContext& context, USAGE page, USHORT collection, std::span<USAGE> buffer, ULONG& out) {
    out = 0;
    if (buffer.empty()) {
        return FieldOutcome::kAbsent;
    }

    auto length = static_cast<ULONG>(buffer.size());
    const FieldOutcome outcome = Classify(HidP_GetUsages(
        HidP_Input,
        page,
        collection,
        buffer.data(),
        &length,
        context.parsed,
        context.bytes,
        context.length
    ));
    if (outcome == FieldOutcome::kRead) {
        out = length;
    }
    return outcome;
}

FieldOutcome ReadPosition(
    const ParseContext& context,
    USHORT collection,
    USAGE usage,
    std::int32_t min,
    std::int32_t max,
    std::optional<std::int32_t>& out
) {
    ULONG position = 0;
    const FieldOutcome outcome = ReadValue(context, HID_USAGE_PAGE_GENERIC, collection, usage, position);
    if (outcome == FieldOutcome::kRead) {
        out = std::clamp(static_cast<std::int32_t>(position), min, max);
    }
    return outcome;
}

FieldOutcome DecodeContact(
    const ParseContext& context,
    const evgetwindows::HidDeviceCaps& caps,
    USHORT collection,
    std::span<USAGE> usages,
    evgetwindows::HidContact& contact
) {
    ULONG contact_id = 0;
    const FieldOutcome identifier =
        ReadValue(context, HID_USAGE_PAGE_DIGITIZER, collection, HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER, contact_id);
    if (identifier != FieldOutcome::kRead) {
        return identifier;
    }
    contact.contact_id = static_cast<std::uint32_t>(contact_id);

    if (caps.axis_valid) {
        const FieldOutcome position_x = ReadPosition(
            context,
            collection,
            HID_USAGE_GENERIC_X,
            caps.axis.min_x,
            caps.axis.max_x,
            contact.position_x
        );
        if (position_x == FieldOutcome::kFailed) {
            return FieldOutcome::kFailed;
        }

        const FieldOutcome position_y = ReadPosition(
            context,
            collection,
            HID_USAGE_GENERIC_Y,
            caps.axis.min_y,
            caps.axis.max_y,
            contact.position_y
        );
        if (position_y == FieldOutcome::kFailed) {
            return FieldOutcome::kFailed;
        }
    }

    // Tip switch and touch valid are single bit buttons, so they occur in the list rather than as values.
    ULONG set_length = 0;
    const FieldOutcome set = ReadUsages(context, HID_USAGE_PAGE_DIGITIZER, collection, usages, set_length);
    if (set == FieldOutcome::kFailed) {
        return FieldOutcome::kFailed;
    }

    const std::span<const USAGE> set_usages{usages.data(), set_length};
    contact.tip_down = Contains(set_usages, HID_USAGE_DIGITIZER_TIP_SWITCH);
    contact.confident = !caps.reports_touch_valid || Contains(set_usages, HID_USAGE_DIGITIZER_TOUCH_VALID);

    // If the entry has a readable report and 0 id while held down, then this would collide so it should be marked
    // absent.
    if (!contact.tip_down && !contact.confident && contact.position_x.value_or(0) == 0 &&
        contact.position_y.value_or(0) == 0) {
        return FieldOutcome::kAbsent;
    }

    return FieldOutcome::kRead;
}

} // namespace

std::optional<evgetwindows::HidDeviceCaps> evgetwindows::HidQuery::CapsFrom(std::span<const std::byte> preparsed) {
    if (preparsed.empty()) {
        return std::nullopt;
    }

    PHIDP_PREPARSED_DATA parsed = AsPreparsed(preparsed);
    HIDP_CAPS caps{};
    if (HidP_GetCaps(parsed, &caps) != HIDP_STATUS_SUCCESS) {
        return std::nullopt;
    }

    std::vector<HIDP_VALUE_CAPS> value_caps(caps.NumberInputValueCaps);
    USHORT value_caps_length = caps.NumberInputValueCaps;
    if (value_caps_length == 0 ||
        HidP_GetValueCaps(HidP_Input, value_caps.data(), &value_caps_length, parsed) != HIDP_STATUS_SUCCESS) {
        return std::nullopt;
    }
    value_caps.resize(value_caps_length);

    // Must be a set to preserve order and deduplication.
    std::set<std::uint32_t> collections;
    for (const auto& value : value_caps) {
        if (value.LinkCollection != 0 && value.UsagePage == HID_USAGE_PAGE_DIGITIZER &&
            ValueCapsCover(value, HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER)) {
            collections.insert(value.LinkCollection);
        }
    }

    HidAxisRange axis{};
    bool has_x = false;
    bool has_y = false;
    for (const auto& value : value_caps) {
        if (value.UsagePage != HID_USAGE_PAGE_GENERIC || !collections.contains(value.LinkCollection)) {
            continue;
        }
        if (!has_x && ValueCapsCover(value, HID_USAGE_GENERIC_X)) {
            axis.min_x = value.LogicalMin;
            axis.max_x = value.LogicalMax;
            has_x = true;
        }
        if (!has_y && ValueCapsCover(value, HID_USAGE_GENERIC_Y)) {
            axis.min_y = value.LogicalMin;
            axis.max_y = value.LogicalMax;
            has_y = true;
        }
    }

    // A descriptor can have the sign extended.
    const bool axis_valid =
        has_x && has_y && axis.min_x >= 0 && axis.min_y >= 0 && axis.max_x > axis.min_x && axis.max_y > axis.min_y;

    std::vector<HIDP_BUTTON_CAPS> button_caps(caps.NumberInputButtonCaps);
    USHORT button_caps_length = caps.NumberInputButtonCaps;
    bool reports_touch_valid = false;
    if (button_caps_length != 0 &&
        HidP_GetButtonCaps(HidP_Input, button_caps.data(), &button_caps_length, parsed) == HIDP_STATUS_SUCCESS) {
        button_caps.resize(button_caps_length);
        reports_touch_valid = std::ranges::any_of(button_caps, [&collections](const HIDP_BUTTON_CAPS& button) {
            return button.UsagePage == HID_USAGE_PAGE_DIGITIZER && collections.contains(button.LinkCollection) &&
                ButtonCapsCover(button, HID_USAGE_DIGITIZER_TOUCH_VALID);
        });
    }

    return HidDeviceCaps{
        .contact_collections = {collections.begin(), collections.end()},
        .axis = axis,
        .axis_valid = axis_valid,
        .reports_touch_valid = reports_touch_valid,
        .digitizer_usage_list_length =
            static_cast<std::uint32_t>(HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, parsed)),
        .button_usage_list_length =
            static_cast<std::uint32_t>(HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_BUTTON, parsed)),
        .input_report_byte_length = caps.InputReportByteLength,
    };
}

std::optional<evgetwindows::HidReport> evgetwindows::HidQuery::DecodeWith(
    std::span<const std::byte> preparsed,
    const HidDeviceCaps& caps,
    std::span<const std::byte> report
) {
    if (preparsed.empty() || report.empty()) {
        return std::nullopt;
    }

    const ParseContext context{
        .parsed = AsPreparsed(preparsed),
        .bytes = AsReport(report),
        .length = static_cast<ULONG>(report.size())
    };

    HidReport decoded{};
    std::vector<USAGE> usages(caps.digitizer_usage_list_length);

    for (const auto collection : caps.contact_collections) {
        HidContact contact{};
        const FieldOutcome outcome = DecodeContact(context, caps, static_cast<USHORT>(collection), usages, contact);
        if (outcome == FieldOutcome::kFailed) {
            return std::nullopt;
        }
        // A contact with no readable identifier cannot be used, so it is removed.
        if (outcome == FieldOutcome::kRead) {
            decoded.contacts.push_back(contact);
        }
    }

    ULONG contact_count = 0;
    const FieldOutcome count =
        ReadValue(context, HID_USAGE_PAGE_DIGITIZER, 0, HID_USAGE_DIGITIZER_CONTACT_COUNT, contact_count);
    if (count == FieldOutcome::kFailed) {
        return std::nullopt;
    }
    if (count == FieldOutcome::kRead) {
        decoded.contact_count = static_cast<std::uint32_t>(contact_count);
    }

    std::vector<USAGE> buttons(caps.button_usage_list_length);
    ULONG pressed_length = 0;
    const FieldOutcome pressed = ReadUsages(context, HID_USAGE_PAGE_BUTTON, 0, buttons, pressed_length);
    if (pressed == FieldOutcome::kFailed) {
        return std::nullopt;
    }
    decoded.button_one_down = Contains(std::span<const USAGE>{buttons.data(), pressed_length}, USAGE{1});

    std::ranges::sort(decoded.contacts, {}, &HidContact::contact_id);

    return decoded;
}

evgetwindows::HidQuery::DeviceCache& evgetwindows::HidQuery::Entry(HANDLE device) {
    if (const auto cached = devices_.find(device); cached != devices_.end()) {
        return cached->second;
    }

    if (devices_.size() >= evgetwindows::kMaxCachedDevices) {
        spdlog::warn("hid device cache reached {} entries", evgetwindows::kMaxCachedDevices);
        devices_.clear();
    }

    return devices_.emplace(device, DeviceCache{}).first->second;
}

evgetwindows::HidQuery::DeviceCache* evgetwindows::HidQuery::Parsed(HANDLE device) {
    if (device == nullptr) {
        return nullptr;
    }

    DeviceCache& entry = Entry(device);
    if (entry.parse_resolved) {
        return entry.parse_valid ? &entry : nullptr;
    }
    entry.parse_resolved = true;

    UINT size = 0;
    // The call is a success if it returns 0.
    if (GetRawInputDeviceInfoW(device, RIDI_PREPARSEDDATA, nullptr, &size) != 0 || size == 0) {
        entry.warned = true;
        spdlog::warn("preparsed data sizing call failed");
        return nullptr;
    }

    // The buffer is allocated by the caller.
    auto preparsed = std::vector<std::byte>(size);
    if (std::cmp_equal(GetRawInputDeviceInfoW(device, RIDI_PREPARSEDDATA, preparsed.data(), &size), -1)) {
        entry.warned = true;
        spdlog::warn("preparsed data query failed");
        return nullptr;
    }

    auto caps = CapsFrom(preparsed);
    if (!caps.has_value()) {
        entry.warned = true;
        spdlog::warn("hid descriptor capabilities could not be derived");
        return nullptr;
    }

    entry.preparsed = std::move(preparsed);
    entry.caps = std::move(*caps);
    entry.parse_valid = true;

    spdlog::trace(
        "hid device report_byte_length={} collections={} axis_valid={} axis=[{},{}]x[{},{}] preparsed={:spn}",
        entry.caps.input_report_byte_length,
        entry.caps.contact_collections.size(),
        entry.caps.axis_valid,
        entry.caps.axis.min_x,
        entry.caps.axis.max_x,
        entry.caps.axis.min_y,
        entry.caps.axis.max_y,
        spdlog::to_hex(entry.preparsed)
    );
    for (const auto collection : entry.caps.contact_collections) {
        spdlog::trace("hid device contact collection {}", collection);
    }

    return &entry;
}

evget::DeviceType evgetwindows::HidQuery::ClassifyDevice(HANDLE device) {
    // This is allowed to be null, e.g. for touchpad emulation.
    if (device == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    DeviceCache& entry = Entry(device);
    if (entry.type_resolved) {
        return entry.device_type;
    }
    // Set before the query to avoid re-query on failure.
    entry.type_resolved = true;

    RID_DEVICE_INFO info{};
    info.cbSize = sizeof(info);
    UINT size = sizeof(info);
    if (std::cmp_equal(GetRawInputDeviceInfoW(device, RIDI_DEVICEINFO, &info, &size), -1)) {
        spdlog::warn("device info query failed");
        return evget::DeviceType::kUnknown;
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
    entry.device_type = info.dwType == RIM_TYPEHID
        ? TopLevelCollectionType(info.hid.usUsagePage, info.hid.usUsage)
        : evget::DeviceType::kUnknown;
    // NOLINTEND(cppcoreguidelines-pro-type-union-access)

    return entry.device_type;
}

std::optional<evgetwindows::HidAxisRange> evgetwindows::HidQuery::AxisRange(HANDLE device) {
    const DeviceCache* entry = Parsed(device);
    if (entry == nullptr || !entry->caps.axis_valid) {
        return std::nullopt;
    }

    return entry->caps.axis;
}

std::optional<evgetwindows::HidReport>
evgetwindows::HidQuery::DecodeReport(HANDLE device, std::span<const std::byte> report) {
    DeviceCache* entry = Parsed(device);
    if (entry == nullptr) {
        return std::nullopt;
    }

    spdlog::trace("hid report {:spn}", spdlog::to_hex(report));

    auto decoded = DecodeWith(entry->preparsed, entry->caps, report);
    if (!decoded.has_value() && !entry->warned) {
        entry->warned = true;
        spdlog::warn("hid report parse failed, suppressing further reports for this device");
    }

    return decoded;
}

void evgetwindows::HidQuery::RemoveDevice(HANDLE device) {
    devices_.erase(device);
}
