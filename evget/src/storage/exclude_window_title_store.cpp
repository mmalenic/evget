#include "evget/storage/exclude_window_title_store.h"

#include <optional>
#include <utility>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/entry.h"
#include "evget/storage/store.h"

evget::ExcludeWindowTitleStore::ExcludeWindowTitleStore(Store& inner) : inner_{&inner} {}

evget::Result<void> evget::ExcludeWindowTitleStore::StoreEvent(Data event) {
    if (event.Empty()) {
        return {};
    }

    Data excluded{};
    for (auto&& entry : std::move(event).IntoEntries()) {
        if (entry.Data().size() <= detail::kFocusWindowNameIndex) {
            excluded.AddEntry(std::move(entry));
            continue;
        }

        auto data = entry.Data();
        data.at(detail::kFocusWindowNameIndex) = std::nullopt;
        excluded.AddEntry(Entry{entry.Type(), data, entry.Modifiers()});
    }

    return inner_->StoreEvent(std::move(excluded));
}
