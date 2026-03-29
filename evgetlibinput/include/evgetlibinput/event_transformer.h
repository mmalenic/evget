/**
 * \file event_transformer.h
 * \brief The libinput specific event transformer for converting input events to the evget format.
 */

#ifndef EVGETLIBINPUT_EVENT_TRANSFORMER_H
#define EVGETLIBINPUT_EVENT_TRANSFORMER_H

#include <libinput.h>

#include <chrono>
#include <unordered_map>

#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/modifier_value.h"
#include "evget/event/mouse_move.h"
#include "evget/event_transformer.h"
#include "evget/input_event.h"
#include "evget/interval_tracker.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"

namespace evgetlibinput {
class EventTransformer : public evget::EventTransformer<evget::InputEvent<LibInputEvent>> {
public:
    /**
     * Create an event transformer.
     * \param libinput_api the libinput API wrapper
     * \param dimensions the screen dimensions for absolute transformation
     */
    explicit EventTransformer(LibInputApi& libinput_api, ScreenDimensions dimensions);

    evget::Data TransformEvent(evget::InputEvent<LibInputEvent> event) override;

private:
    std::reference_wrapper<LibInputApi> libinput_api_;
    ScreenDimensions dimensions_;

    std::unordered_map<libinput_device*, int> device_ids_;
    int next_device_id_{};

    std::unordered_map<int, double> previous_absolute_x_;
    std::unordered_map<int, double> previous_absolute_y_;
    std::unordered_map<int, evget::IntervalTracker> device_intervals_;

    evget::DeviceType GetDeviceType(LibInputEvent& event) const;
    static evget::ButtonAction GetButtonAction(libinput_button_state state);
    int GetDeviceId(libinput_device& device);
    void SetRelativePosition(evget::MouseMove& builder, int device_id, libinput_event_pointer& pointer_event);

    template <evget::BuilderHasModifier T>
    T& SetModifierValues(T& builder) const;
};

template <evget::BuilderHasModifier T>
T& EventTransformer::SetModifierValues(T& builder) const {
    if (this->libinput_api_.get().IsModifierActive(XKB_MOD_NAME_SHIFT)) {
        builder.Modifier(evget::ModifierValue::kShift);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_MOD_NAME_CAPS)) {
        builder.Modifier(evget::ModifierValue::kCapsLock);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_MOD_NAME_CTRL)) {
        builder.Modifier(evget::ModifierValue::kControl);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_VMOD_NAME_ALT)) {
        builder.Modifier(evget::ModifierValue::kAlt);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_VMOD_NAME_NUM)) {
        builder.Modifier(evget::ModifierValue::kNumLock);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_MOD_NAME_MOD3)) {
        builder.Modifier(evget::ModifierValue::kMod3);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_VMOD_NAME_SUPER)) {
        builder.Modifier(evget::ModifierValue::kSuper);
    }
    if (this->libinput_api_.get().IsModifierActive(XKB_MOD_NAME_MOD5)) {
        builder.Modifier(evget::ModifierValue::kMod5);
    }

    return builder;
}

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_EVENT_TRANSFORMER_H
