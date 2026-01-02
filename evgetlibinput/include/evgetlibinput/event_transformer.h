/**
 * \file event_transformer.h
 * \brief The libinput specific event transformer for converting input events to the evget format.
 */

#ifndef EVGETLIBINPUT_EVENT_TRANSFORMER_H
#define EVGETLIBINPUT_EVENT_TRANSFORMER_H

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
};
} // namespace evgetlibinput

#endif // EVGETLIBINPUT_EVENT_TRANSFORMER_H
