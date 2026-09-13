#include "evget/device_id.h"

#include <gtest/gtest.h>

#include <set>
#include <string>

TEST(DeviceId, DeterministicUuidSameNames) {
    const std::string name{R"(\\?\HID#VID_046D&X)"};

    evget::DeviceId<std::string> first{};
    evget::DeviceId<std::string> second{};

    ASSERT_EQ(first.UuidDeterministic(name), second.UuidDeterministic(name));
}

TEST(DeviceId, DeterministicRepeatedCalls) {
    const std::string name{R"(\\?\HID#VID_046D&X)"};

    evget::DeviceId<std::string> device_ids{};

    const std::string first = device_ids.UuidDeterministic(name);
    ASSERT_EQ(first, device_ids.UuidDeterministic(name));
}

TEST(DeviceId, DeterministicDistinctNames) {
    evget::DeviceId<std::string> device_ids{};

    std::set<std::string> uuids{};
    for (const std::string& name : {std::string{"A"}, std::string{"B"}, std::string{"C"}, std::string{"D"}}) {
        uuids.insert(device_ids.UuidDeterministic(name));
    }

    ASSERT_EQ(uuids.size(), 4);
}
