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
//

#include "evgetcore/Event/Entry.h"
#include <utility>
#include <vector>

#include "evgetcore/Event/Data.h"

const std::vector<EvgetCore::Event::Entry>& EvgetCore::Event::Data::entries() const {
    return _entries;
}

void EvgetCore::Event::Data::mergeWith(Data&& data) {
    auto entries = std::move(data).intoEntries();
    this->_entries
        .insert(this->_entries.end(), std::make_move_iterator(entries.begin()), std::make_move_iterator(entries.end()));
}

std::vector<EvgetCore::Event::Entry> EvgetCore::Event::Data::intoEntries() && {
    return std::move(_entries);
}

void EvgetCore::Event::Data::addEntry(Entry entry) {
    _entries.emplace_back(std::move(entry));
}

bool EvgetCore::Event::Data::empty() const {
    return _entries.empty();
}
