/**
 * \file event_transformer.h
 * \brief Transformer for the Windows backend.
 */

#ifndef EVGETWINDOWS_EVENT_TRANSFORMER_H
#define EVGETWINDOWS_EVENT_TRANSFORMER_H

#include "evget/event/data.h"
#include "evget/event_transformer.h"
#include "evget/input_event.h"
#include "evgetwindows/raw_event.h"

namespace evgetwindows {

/**
 * \brief Event transformer for the Windows backend.
 */
class EventTransformer : public evget::EventTransformer<evget::InputEvent<RawEvent>> {
public:
    EventTransformer() = default;

    /**
     * \brief Transform a raw input event into storage data.
     * \param event the raw input event
     * \return the transformed data
     */
    evget::Data TransformEvent(evget::InputEvent<RawEvent> /*event*/) override {
        return evget::Data{};
    }
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_EVENT_TRANSFORMER_H
