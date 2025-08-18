#include "evget/event/data.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "evget/event/entry.h"

TEST(DataTest, Create) {
    evget::Data data{};
    data.AddEntry({evget::EntryType::kKey, {"data"}, {"modifier"}});

    evget::Data merge{};
    merge.AddEntry({evget::EntryType::kMouseMove, {"merge"}, {"merge_modifier"}});
    data.MergeWith(std::move(merge));

    ASSERT_FALSE(data.Empty());
    ASSERT_EQ(data.Entries().size(), 2);

    auto first = data.Entries()[0];
    ASSERT_EQ(first.Type(), evget::EntryType::kKey);
    ASSERT_EQ(first.Data(), std::vector<std::string>{"data"});
    ASSERT_EQ(first.Modifiers(), std::vector<std::string>{"modifier"});

    auto second = data.Entries()[1];
    ASSERT_EQ(second.Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(second.Data(), std::vector<std::string>{"merge"});
    ASSERT_EQ(second.Modifiers(), std::vector<std::string>{"merge_modifier"});
}
