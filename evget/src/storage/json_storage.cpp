#include "evget/storage/json_storage.h"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <memory>
#include <ostream>
#include <utility>
#include <variant>
#include <vector>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/schema.h"

evget::Result<void> evget::JsonStorage::StoreEvent(Data events) {
    if (events.Empty()) {
        return Result<void>{};
    }

    auto formatted_entries = std::vector<nlohmann::json>{};
    for (auto entry : events.Entries()) {
        entry.ToNamedRepresentation();
        auto entry_with_fields = entry.GetEntryWithFields();

        auto formatted_fields = std::vector<nlohmann::json>{};
        for (auto i = 0; i < entry_with_fields.data.size(); i++) {
            formatted_fields.push_back({{"name", entry_with_fields.fields[i]}, {"data", entry_with_fields.data[i]}});
        }
        formatted_entries.push_back(
            {{"type", FromEntryType(entry_with_fields.type)},
             {"fields", formatted_fields},
             {"modifiers", entry_with_fields.modifiers}}
        );
    }

    nlohmann::json output{};
    output["entries"] = formatted_entries;

    std::visit([output](auto&& ostream) { *ostream << output.dump(4) << "\n"; }, ostream_);

    return Result<void>{};
}

evget::JsonStorage::JsonStorage(std::unique_ptr<std::ostream> ostream) : ostream_{std::move(ostream)} {}

evget::JsonStorage::JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream)
    : ostream_{std::move(ostream)} {}
