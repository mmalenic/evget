/**
 * \file filter_store.h
 * \brief Store that filters events by device type.
 */

#ifndef EVGET_STORAGE_FILTER_STORE_H
#define EVGET_STORAGE_FILTER_STORE_H

#include <optional>
#include <set>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief A `Store` that removes entries where the device type is not in the allowed set,
 *        before forwarding to an inner `Store`.
 */
class FilterStore : public Store {
public:
    /**
     * \brief Construct a filter store.
     * \param inner reference to the inner store to forward to
     * \param allowed optional set of device types that should be forwarded, nullopt allows all
     */
    FilterStore(Store& inner, std::optional<std::set<DeviceType>> allowed);

    Result<void> StoreEvent(Data event) override;

private:
    Store* inner_;
    std::optional<std::set<DeviceType>> allowed_;
};

} // namespace evget

#endif
