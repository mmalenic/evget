#ifndef EVGET_STORAGE_STORE_H
#define EVGET_STORAGE_STORE_H

#include "evget/error.h"
#include "evget/event/data.h"

namespace evget {

/**
 * An interface which represents storing data.
 */
class Store {
public:
    /**
     * Store the event data.
     */
    virtual Result<void> StoreEvent(Data event) = 0;

    Store() = default;

    virtual ~Store() = default;

    Store(const Store&) = delete;
    Store(Store&&) noexcept = delete;
    Store& operator=(const Store&) = delete;
    Store& operator=(Store&&) noexcept = delete;
};
} // namespace evget
#endif
