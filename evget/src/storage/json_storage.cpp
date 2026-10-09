#include "evget/storage/json_storage.h"

#include <nlohmann/json.hpp> // NOLINT(misc-include-cleaner)
#include <nlohmann/json_fwd.hpp>

#include <cstddef>
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
        for (std::size_t i = 0; i < entry_with_fields.data.size(); i++) {
            const auto& value = entry_with_fields.data.at(i);
            const nlohmann::json data = value.has_value() ? nlohmann::json(*value) : nlohmann::json(nullptr);
            formatted_fields.push_back({{"name", entry_with_fields.fields.at(i)}, {"data", data}});
        }
        formatted_entries.push_back(
            {{"type", FromEntryType(entry_with_fields.type)},
             {"fields", formatted_fields},
             {"modifiers", entry_with_fields.modifiers}}
        );
    }

    const nlohmann::json output{{"entries", formatted_entries}};
    // Titles can carry legacy bytes, which the default strict handler throws on.
    const auto serialized = output.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
    std::visit([&serialized](auto&& ostream) { *ostream << serialized << "\n" << std::flush; }, ostream_);

    return Result<void>{};
}

evget::JsonStorage::JsonStorage(std::unique_ptr<std::ostream> ostream) : ostream_{std::move(ostream)} {}

evget::JsonStorage::JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream)
    : ostream_{std::move(ostream)} {}
