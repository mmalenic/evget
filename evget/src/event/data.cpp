#include "evget/event/data.h"

#include <iterator>
#include <utility>
#include <vector>

#include "evget/event/entry.h"

const std::vector<evget::Entry>& evget::Data::Entries() const {
    return entries_;
}

void evget::Data::MergeWith(Data&& data) {
    auto entries = std::move(data).IntoEntries();
    this->entries_
        .insert(this->entries_.end(), std::make_move_iterator(entries.begin()), std::make_move_iterator(entries.end()));
}

std::vector<evget::Entry> evget::Data::IntoEntries() && {
    return std::move(entries_);
}

void evget::Data::AddEntry(Entry entry) {
    entries_.push_back(std::move(entry));
}

bool evget::Data::Empty() const {
    return entries_.empty();
}
