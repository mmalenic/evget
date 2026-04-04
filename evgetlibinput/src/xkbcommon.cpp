#include "evgetlibinput/xkbcommon.h"

#include <xkbcommon/xkbcommon.h>

#include <array>
#include <optional>
#include <string>

#include "evget/error.h"

evget::Result<std::unique_ptr<evgetlibinput::XkbCommon>> evgetlibinput::XkbCommon::New() {
    auto xkb = std::unique_ptr<XkbCommon>(new XkbCommon{});

    xkb->xkb_context_ = {xkb_context_new(XKB_CONTEXT_NO_FLAGS), xkb_context_unref};
    if (xkb->xkb_context_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb context"}
        };
    }

    xkb->xkb_key_map_ = {
        // Recommended to use XKB_KEYMAP_FORMAT_TEXT_V1 for now instead of V2:
        // https://xkbcommon.org/doc/current/group__keymap.html#gab0f75d6cc5773e5dd404e2c3f61366a3
        xkb_keymap_new_from_names2(
            xkb->xkb_context_.get(),
            nullptr,
            XKB_KEYMAP_FORMAT_TEXT_V1,
            XKB_KEYMAP_COMPILE_NO_FLAGS
        ),
        xkb_keymap_unref
    };
    if (xkb->xkb_key_map_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb keymap"}
        };
    }

    xkb->xkb_state_ = {xkb_state_new(xkb->xkb_key_map_.get()), xkb_state_unref};
    if (xkb->xkb_state_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb state"}
        };
    }

    return xkb;
}

std::optional<std::string> evgetlibinput::XkbCommon::GetKeyName(xkb_keycode_t key) {
    auto sym = xkb_state_key_get_one_sym(xkb_state_.get(), key);
    if (sym == XKB_KEY_NoSymbol) {
        return std::nullopt;
    }

    // xkb_keysym_get_name recommends a buffer of 64 bytes.
    constexpr auto kKeySymBufSize = 64;
    std::array<char, kKeySymBufSize> buf{};
    auto needed = xkb_keysym_get_name(sym, buf.data(), buf.size());
    if (needed <= 0) {
        return std::nullopt;
    }

    if (needed < buf.size()) {
        // String output is null-terminated.
        return std::string{buf.data()};
    }

    // If this has failed for some reason, try again with the correct amount of bytes.
    std::string name(needed, '\0');
    xkb_keysym_get_name(sym, name.data(), name.size() + 1);
    return name;
}

std::optional<std::string> evgetlibinput::XkbCommon::GetKeyCharacter(xkb_keycode_t key) {
    // xkb_state_key_get_utf8 with null buffer returns the required size.
    auto size = xkb_state_key_get_utf8(xkb_state_.get(), key, nullptr, 0);
    if (size <= 0) {
        return std::nullopt;
    }

    std::string character(size, '\0');
    xkb_state_key_get_utf8(xkb_state_.get(), key, character.data(), character.size() + 1);
    return character;
}

bool evgetlibinput::XkbCommon::IsModifierActive(const char* modifier_name) {
    return xkb_state_mod_name_is_active(this->xkb_state_.get(), modifier_name, XKB_STATE_MODS_EFFECTIVE) != 0;
}

void evgetlibinput::XkbCommon::UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) {
    // From xkb's perspective, evget is considered a server as it handles libinput events directly, so
    // use the server version of the updating state rather than xkb_state_update_mask.
    xkb_state_update_key(this->xkb_state_.get(), key, direction);
}
