#include <gtest/gtest.h>

#include <vector>

#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"
// clang-format off
#include "evget/event/mouse_move.h"
// clang-format on

TEST(MouseMoveTest, Event) {
    auto data = evget::Data{};
    auto mouse_move =
        evget::MouseMove{}
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
            .Device(evget::DeviceType::kKeyboard)
            .Modifier(evget::ModifierValue::kAlt)
            .Build(data);

    auto entry = mouse_move.Entries().at(0);
    entry.ToNamedRepresentation();
    auto named_entry = entry.GetEntryWithFields();

    auto expected_fields =
        std::vector<std::string>{evget::detail::kMouseMoveFields.begin(), evget::detail::kMouseMoveFields.end()};
    const std::vector<std::string> expected_data{
        "1",
        "1970-01-01T00:00:00.000000000+0000",
        "1.000000",
        "1.000000",
        "name",
        "name",
        "1.000000",
        "1.000000",
        "1.000000",
        "1.000000",
        "1",
        "Keyboard"
    };

    ASSERT_EQ(named_entry.type, evget::EntryType::kMouseMove);
    ASSERT_EQ(named_entry.fields, expected_fields);
    ASSERT_EQ(named_entry.data, expected_data);
    ASSERT_EQ(named_entry.modifiers, std::vector<std::string>{"Alt"});
}
