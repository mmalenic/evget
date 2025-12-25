/**
 * \file event_transformer.h
 * \brief The libinput specific event transformer for converting input events to the evget format.
 */

#ifndef EVGETLIBINPUT_EVENT_TRANSFORMER_H
#define EVGETLIBINPUT_EVENT_TRANSFORMER_H

#include "evget/event_transformer.h"
#include "evgetlibinput/libinput_api.h"

namespace evgetlibinput {
class EventTransformer : public evget::EventTransformer<InputEvent> {
public:
    evget::Data TransformEvent(InputEvent event) override;
};
} // namespace evgetlibinput

#endif // EVGETLIBINPUT_EVENT_TRANSFORMER_H
