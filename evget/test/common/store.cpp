#include "common/store.h"

#include <chrono>
#include <cstddef>
#include <expected>
#include <mutex>
#include <utility>
#include <vector>

#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/key.h"
#include "evget/event/modifier_value.h"
#include "evget/event/mouse_click.h"
#include "evget/event/schema.h"

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

evget::Data test::StoreMock::MakeKeyData(evget::DeviceType device) {
    evget::Data data{};
    evget::Key{}
        .Interval(evget::IntervalType{1})
        .Timestamp(evget::TimestampType{})
        .PositionX(1)
        .PositionY(1)
        .DeviceName("name")
        .FocusWindowName("name")
        .FocusWindowPositionX(1)
        .FocusWindowPositionY(1)
        .FocusWindowWidth(1)
        .FocusWindowHeight(1)
        .Screen(1)
        .Device(device)
        .SystemEvent("test_event")
        .Button(1)
        .ButtonName("name")
        .Action(evget::ButtonAction::kPress)
        .Character("a")
        .Modifier(evget::ModifierValue::kAlt)
        .Build(data);
    return data;
}

evget::Data test::StoreMock::MakeMouseClickData(evget::DeviceType device) {
    evget::Data data{};
    evget::MouseClick{}
        .Interval(evget::IntervalType{1})
        .Timestamp(evget::TimestampType{})
        .PositionX(1)
        .PositionY(1)
        .DeviceName("name")
        .FocusWindowName("name")
        .FocusWindowPositionX(1)
        .FocusWindowPositionY(1)
        .FocusWindowWidth(1)
        .FocusWindowHeight(1)
        .Screen(1)
        .Device(device)
        .SystemEvent("test_event")
        .Button(1)
        .ButtonName("name")
        .Action(evget::ButtonAction::kPress)
        .Build(data);
    return data;
}

evget::Result<void> test::StoreErrorMock::StoreEvent(evget::Data /*event*/) {
    return std::unexpected{
        evget::Error{.error_type = evget::ErrorType::kDatabaseManagerError, .message = "mock error"}
    };
}
