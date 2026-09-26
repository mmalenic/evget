/**
 * \file exclude_window_title_store.h
 * \brief Store that removes the focus window name from events.
 */

#ifndef EVGET_STORAGE_EXCLUDE_WINDOW_TITLE_STORE_H
#define EVGET_STORAGE_EXCLUDE_WINDOW_TITLE_STORE_H

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief A `Store` that removes the focus window name from every entry and forwards to an inner `Store`.
 */
class ExcludeWindowTitleStore : public Store {
public:
    /**
     * \brief Construct the exclude window title store.
     * \param inner reference to the inner store to forward to
     */
    explicit ExcludeWindowTitleStore(Store& inner);

    Result<void> StoreEvent(Data event) override;

private:
    Store* inner_;
};

} // namespace evget

#endif
