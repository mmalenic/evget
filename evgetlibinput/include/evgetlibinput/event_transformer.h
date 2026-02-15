/**
 * \file event_transformer.h
 * \brief The libinput specific event transformer for converting input events to the evget format.
 */

#ifndef EVGETLIBINPUT_EVENT_TRANSFORMER_H
#define EVGETLIBINPUT_EVENT_TRANSFORMER_H

#include "evget/error.h"
#include "evget/event/modifier_value.h"
#include "evget/event_transformer.h"
#include "evget/input_event.h"
#include "evgetlibinput/libinput_api.h"

namespace evgetlibinput {
class EventTransformer : public evget::EventTransformer<evget::InputEvent<LibInputEvent>> {
public:
    /**
     * Create an event transformer.
     * \param libinput_api the API wrapper
     */
    explicit EventTransformer(LibInputApi& libinput_api);

    evget::Data TransformEvent(evget::InputEvent<LibInputEvent> event) override;

private:
    std::reference_wrapper<LibInputApi> libinput_api_;

    evget::DeviceType GetDeviceType(LibInputEvent& event) const;

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
