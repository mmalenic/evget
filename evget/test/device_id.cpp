#include "evget/device_id.h"

#include <gtest/gtest.h>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>

#include <algorithm>
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

TEST(DeviceId, DeterministicUuidNamespace) {
    const boost::uuids::uuid first_ns = boost::uuids::string_generator{}("6ba7b810-9dad-11d1-80b4-00c04fd430c8");
    const boost::uuids::uuid second_ns = boost::uuids::string_generator{}("6ba7b811-9dad-11d1-80b4-00c04fd430c8");
    const std::string name{"device"};

    evget::DeviceId<std::string> first{};
    evget::DeviceId<std::string> second{};

    ASSERT_NE(first.UuidDeterministic(name, first_ns), second.UuidDeterministic(name, second_ns));
}
