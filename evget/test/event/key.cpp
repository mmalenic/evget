#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "evget/event/button_action.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
// clang-format off
#include "evget/event/key.h"
// clang-format on
#include "evget/event/modifier_value.h"

TEST(KeyTest, Event) {
    auto data = evget::Data{};
    auto key =
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
            .Device(evget::DeviceType::kKeyboard)
            .Button(1)
            .Name("name")
            .Action(evget::ButtonAction::kPress)
            .Character("a")
            .Modifier(evget::ModifierValue::kAlt)
            .Build(data);

    auto entry = key.Entries()[0];
    entry.ToNamedRepresentation();
    auto named_entry = entry.GetEntryWithFields();

    auto expected_fields = std::vector<std::string>{evget::detail::kKeyFields.begin(), evget::detail::kKeyFields.end()};
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
        "Keyboard",
        "1",
        "name",
        "Press",
        "a"
    };

    ASSERT_EQ(named_entry.type, evget::EntryType::kKey);
    ASSERT_EQ(named_entry.fields, expected_fields);
    ASSERT_EQ(named_entry.data, expected_data);
    ASSERT_EQ(named_entry.modifiers, std::vector<std::string>{"Alt"});
}
