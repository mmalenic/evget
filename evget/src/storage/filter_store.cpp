#include "evget/storage/filter_store.h"

#include <set>
#include <utility>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evget/storage/store.h"

evget::FilterStore::FilterStore(Store& inner, std::set<DeviceType> allowed)
    : inner_{&inner}, allowed_{std::move(allowed)} {}

evget::Result<void> evget::FilterStore::StoreEvent(Data event) {
    Data filtered{};
    for (auto&& entry : std::move(event).IntoEntries()) {
        const auto& data = entry.Data();
        if (data.size() <= detail::kDeviceTypeIndex) {
            filtered.AddEntry(std::move(entry));
            continue;
        }

        auto device = FromUnderlying<DeviceType>(data.at(detail::kDeviceTypeIndex));
        if (!device.has_value() || allowed_.contains(*device)) {
            filtered.AddEntry(std::move(entry));
        }
    }

    if (filtered.Empty()) {
        return {};
    }
    return inner_->StoreEvent(std::move(filtered));
}
