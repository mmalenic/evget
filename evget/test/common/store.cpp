#include "common/store.h"

#include <chrono>
#include <cstddef>
#include <expected>
#include <mutex>
#include <utility>
#include <vector>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/entry.h"

evget::Result<void> test::StoreMock::StoreEvent(evget::Data event) {
    const std::scoped_lock guard{lock_};
    events_.push_back(std::move(event));
    condition_.notify_all();
    return {};
}

void test::StoreMock::WaitForEvents(std::size_t count) {
    std::unique_lock guard{lock_};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    condition_.wait_for(guard, std::chrono::seconds{5}, [&] { return events_.size() >= count; });
}

std::vector<evget::Data> test::StoreMock::Events() {
    const std::scoped_lock guard{lock_};
    return events_;
}

evget::Data test::StoreMock::MakeData() {
    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {"value"}, {}});
    return data;
}

evget::Result<void> test::StoreErrorMock::StoreEvent(evget::Data /*event*/) {
    return std::unexpected{
        evget::Error{.error_type = evget::ErrorType::kDatabaseManagerError, .message = "mock error"}
    };
}
