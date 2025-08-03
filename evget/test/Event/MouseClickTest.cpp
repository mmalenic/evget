// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "evget/Event/ButtonAction.h"
#include "evget/Event/Device.h"
#include "evget/Event/Entry.h"
#include "evget/Event/ModifierValue.h"
#include "evget/Event/MouseClick.h"

TEST(MouseClickTest, Event) {
    auto data = evget::Event::Data{};
    auto mouse_click = evget::Event::MouseClick{}
                           .interval(evget::Event::Interval{1})
                           .timestamp(evget::Event::Timestamp{})
                           .positionX(1)
                           .positionY(1)
                           .deviceName("name")
                           .focusWindowName("name")
                           .focusWindowPositionX(1)
                           .focusWindowPositionY(1)
                           .focusWindowWidth(1)
                           .focusWindowHeight(1)
                           .screen(1)
                           .device(evget::Event::Device::Keyboard)
                           .button(1)
                           .name("name")
                           .action(evget::Event::ButtonAction::Press)
                           .modifier(evget::Event::ModifierValue::Alt)
                           .build(data);

    auto entry = mouse_click.entries()[0];
    entry.toNamedRepresentation();
    auto named_entry = entry.getEntryWithFields();

    auto expected_fields = std::vector<std::string>{
        evget::Event::detail::mouseClickFields.begin(),
        evget::Event::detail::mouseClickFields.end()
    };
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
        "Press"
    };

    ASSERT_EQ(named_entry.type, evget::Event::EntryType::MouseClick);
    ASSERT_EQ(named_entry.fields, expected_fields);
    ASSERT_EQ(named_entry.data, expected_data);
    ASSERT_EQ(named_entry.modifiers, std::vector<std::string>{"Alt"});
}
