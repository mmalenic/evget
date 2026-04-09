/**
 * \file event_transformer.h
 * \brief The libinput specific event transformer for converting input events to the evget format.
 */

#ifndef EVGETLIBINPUT_EVENT_TRANSFORMER_H
#define EVGETLIBINPUT_EVENT_TRANSFORMER_H

#include <libevdev/libevdev.h>
#include <libinput.h>
#include <linux/input-event-codes.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <unordered_map>

#include "evget/device_id.h"
#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/concepts.h"
#include "evget/event/key.h"
#include "evget/event/modifier_value.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/mouse_scroll.h"
#include "evget/event_transformer.h"
#include "evget/input_event.h"
#include "evget/interval_tracker.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/xkbcommon.h"

namespace evgetlibinput {

/// \brief Event source for the libinput backend.
constexpr std::string_view kEventSourceName{"libinput"};

class EventTransformer : public evget::EventTransformer<evget::InputEvent<LibInputEvent>> {
public:
    /**
     * Create an event transformer.
     * \param libinput_api the libinput API wrapper
     * \param xkb the xkbcommon wrapper used for keysym translation and modifier state
     * \param dimensions the screen dimensions for absolute transformation
     */
    EventTransformer(LibInputApi& libinput_api, XkbCommon& xkb, ScreenDimensions dimensions);

    evget::Data TransformEvent(evget::InputEvent<LibInputEvent> event) override;

private:
    struct EventContext {
        evget::TimestampType timestamp;
        evget::DeviceType device_type;
        const char* device_name;
        std::reference_wrapper<const std::string> device_uuid;
        std::string system_event;
    };

    std::reference_wrapper<LibInputApi> libinput_api_;
    std::reference_wrapper<XkbCommon> xkb_;
    ScreenDimensions dimensions_;

    evget::DeviceId<libinput_device*> device_ids_;

    std::unordered_map<std::string, double> previous_absolute_x_;
    std::unordered_map<std::string, double> previous_absolute_y_;
    std::map<std::pair<std::string, std::int32_t>, double> previous_touch_x_;
    std::map<std::pair<std::string, std::int32_t>, double> previous_touch_y_;
    std::unordered_map<std::string, evget::IntervalTracker> device_intervals_;

    evget::DeviceType GetDeviceType(LibInputEvent& event, libinput_event_type event_type) const;
    static evget::ButtonAction GetButtonAction(libinput_button_state state);
    static evget::ButtonAction GetTipAction(libinput_tablet_tool_tip_state state);
    static evget::ButtonAction GetKeyAction(libinput_key_state state);
    static xkb_key_direction GetXkbDirection(libinput_key_state state);
    void SetRelativePosition(
        evget::MouseMove& builder,
        const std::string& device_uuid,
        libinput_event_pointer& pointer_event
    );
    void SetTouchRelativePosition(
        evget::MouseMove& builder,
        const std::string& device_uuid,
        std::int32_t seat_slot,
        libinput_event_touch& touch_event
    );
    void ClearTouchPosition(const std::string& device_uuid, std::int32_t seat_slot);
    void BuildTabletToolMove(
        evget::Data& data,
        const EventContext& ctx,
        std::uint64_t event_time,
        libinput_event_tablet_tool& tool_event
    );
    void BuildScrollEvent(evget::Data& data, const EventContext& ctx, LibInputEvent& event);
    void
    BuildTouchRelease(evget::Data& data, const EventContext& ctx, const std::string& device_uuid, LibInputEvent& event);
    template <evget::BuilderHasButtonName T>
    void SetButtonName(T& builder, std::uint32_t code);

    template <evget::BuilderHasBaseFields T>
    T& SetBaseFields(T& builder, const EventContext& ctx, std::uint64_t event_time);

    template <evget::BuilderHasModifier T>
    T& SetModifierValues(T& builder) const;
};

template <evget::BuilderHasBaseFields T>
T& EventTransformer::SetBaseFields(T& builder, const EventContext& ctx, std::uint64_t event_time) {
    builder.Timestamp(ctx.timestamp)
        .Interval(device_intervals_[ctx.device_uuid].Interval(event_time))
        .Device(ctx.device_type)
        .DeviceName(ctx.device_name)
        .DeviceId(ctx.device_uuid)
        .SystemEvent(ctx.system_event)
        .EventSource(std::string{kEventSourceName});
    return SetModifierValues(builder);
}

template <evget::BuilderHasButtonName T>
void EventTransformer::SetButtonName(T& builder, std::uint32_t code) {
    const auto* name = libevdev_event_code_get_name(EV_KEY, code);
    if (name != nullptr) {
        builder.ButtonName(name);
    }
}

template <evget::BuilderHasModifier T>
T& EventTransformer::SetModifierValues(T& builder) const {
    if (this->xkb_.get().IsModifierActive(XKB_MOD_NAME_SHIFT)) {
        builder.Modifier(evget::ModifierValue::kShift);
    }
    if (this->xkb_.get().IsModifierActive(XKB_MOD_NAME_CAPS)) {
        builder.Modifier(evget::ModifierValue::kCapsLock);
    }
    if (this->xkb_.get().IsModifierActive(XKB_MOD_NAME_CTRL)) {
        builder.Modifier(evget::ModifierValue::kControl);
    }
    if (this->xkb_.get().IsModifierActive(XKB_VMOD_NAME_ALT)) {
        builder.Modifier(evget::ModifierValue::kAlt);
    }
    if (this->xkb_.get().IsModifierActive(XKB_VMOD_NAME_NUM)) {
        builder.Modifier(evget::ModifierValue::kNumLock);
    }
    if (this->xkb_.get().IsModifierActive(XKB_MOD_NAME_MOD3)) {
        builder.Modifier(evget::ModifierValue::kMod3);
    }
    if (this->xkb_.get().IsModifierActive(XKB_VMOD_NAME_SUPER)) {
        builder.Modifier(evget::ModifierValue::kSuper);
    }
    if (this->xkb_.get().IsModifierActive(XKB_MOD_NAME_MOD5)) {
        builder.Modifier(evget::ModifierValue::kMod5);
    }

    return builder;
}

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_EVENT_TRANSFORMER_H
