#include "evget/storage/filter_store.h"

#include <gtest/gtest.h>

#include <set>
#include <utility>

#include "common/store.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"

TEST(FilterStoreTest, AllDeviceTypesPassThrough) {
    test::StoreMock inner{};
    evget::FilterStore filter{
        inner,
        {evget::DeviceType::kMouse,
         evget::DeviceType::kKeyboard,
         evget::DeviceType::kTouchpad,
         evget::DeviceType::kTouchscreen,
         evget::DeviceType::kTablet,
         evget::DeviceType::kUnknown}
    };

    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());
    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)).has_value());

    auto events = inner.Events();
    ASSERT_EQ(events.size(), 2);
}

TEST(FilterStoreTest, MatchingDeviceTypesForwarded) {
    test::StoreMock inner{};
    evget::FilterStore filter{inner, {evget::DeviceType::kKeyboard}};

    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());
    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)).has_value());

    auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
}

TEST(FilterStoreTest, MultipleAllowedDeviceTypes) {
    test::StoreMock inner{};
    evget::FilterStore filter{inner, {evget::DeviceType::kKeyboard, evget::DeviceType::kMouse}};

    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());
    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)).has_value());
    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeKeyData(evget::DeviceType::kTablet)).has_value());

    auto events = inner.Events();
    ASSERT_EQ(events.size(), 2);
}

TEST(FilterStoreTest, EmptyAllowedSetFiltersAll) {
    test::StoreMock inner{};
    evget::FilterStore filter{inner, {}};

    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeKeyData(evget::DeviceType::kKeyboard)).has_value());
    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeMouseClickData(evget::DeviceType::kMouse)).has_value());

    ASSERT_TRUE(inner.Events().empty());
}

TEST(FilterStoreTest, MixedEntriesInSingleDataFiltered) {
    test::StoreMock inner{};
    evget::FilterStore filter{inner, {evget::DeviceType::kKeyboard}};

    evget::Data combined{};
    for (auto&& entry : test::StoreMock::MakeKeyData(evget::DeviceType::kKeyboard).IntoEntries()) {
        combined.AddEntry(std::move(entry));
    }
    for (auto&& entry : test::StoreMock::MakeMouseClickData(evget::DeviceType::kMouse).IntoEntries()) {
        combined.AddEntry(std::move(entry));
    }

    ASSERT_TRUE(filter.StoreEvent(std::move(combined)).has_value());

    auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    ASSERT_EQ(events.at(0).Entries().at(0).Type(), evget::EntryType::kKey);
}

TEST(FilterStoreTest, EntryWithoutDeviceTypePasses) {
    test::StoreMock inner{};
    evget::FilterStore filter{inner, {evget::DeviceType::kKeyboard}};

    ASSERT_TRUE(filter.StoreEvent(test::StoreMock::MakeData()).has_value());

    auto events = inner.Events();
    ASSERT_EQ(events.size(), 1);
}
