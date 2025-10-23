/**
 * \file event_transformer.h
 * \brief Abstract interface for transforming events into the storage format.
 */

#ifndef EVGET_EVENT_TRANSFORMER_H
#define EVGET_EVENT_TRANSFORMER_H

#include "evget/event/data.h"

namespace evget {

/**
 * \brief Transform the data so it's usable by the storage.
 * \tparam T type of data
 */
template <typename T>
class EventTransformer {
public:
    /**
     * \brief Transform the event.
     * \param event event to transform
     * \return transformed event data
     */
    virtual Data TransformEvent(T event) = 0;

    EventTransformer() = default;

    virtual ~EventTransformer() = default;

    EventTransformer(EventTransformer&&) noexcept = delete;
    EventTransformer& operator=(EventTransformer&&) noexcept = delete;

    EventTransformer(const EventTransformer&) = delete;
    EventTransformer& operator=(const EventTransformer&) = delete;
};
} // namespace evget

#endif
