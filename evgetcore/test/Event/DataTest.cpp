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

#include <gtest/gtest.h>

#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Entry.h"

TEST(DataTest, Create) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Event::Data data{};
    data.addEntry({EvgetCore::Event::EntryType::Key, {"data"}, {"modifier"}});

    EvgetCore::Event::Data merge{};
    merge.addEntry({EvgetCore::Event::EntryType::MouseMove, {"merge"}, {"merge_modifier"}});
    data.mergeWith(std::move(merge));

    ASSERT_FALSE(data.empty());
    ASSERT_EQ(data.entries().size(), 2);

    auto first = data.entries()[0];
    ASSERT_EQ(first.type(), EvgetCore::Event::EntryType::Key);
    ASSERT_EQ(first.data(), std::vector<std::string>{"data"});
    ASSERT_EQ(first.modifiers(), std::vector<std::string>{"modifier"});

    auto second = data.entries()[1];
    ASSERT_EQ(second.type(), EvgetCore::Event::EntryType::MouseMove);
    ASSERT_EQ(second.data(), std::vector<std::string>{"merge"});
    ASSERT_EQ(second.modifiers(), std::vector<std::string>{"merge_modifier"});
}
