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

#ifndef DATA_H
#define DATA_H

#include <vector>

#include "evget/Event/Entry.h"

namespace evget::Event {
/**
 * \brief Data represents the actual entries that are generated from device events. The order
 * of the entries defines the final insertion order in the storage component.
 */
class Data {
public:
    /**
     * \brief Get a reference to the entries.
     * \return Entries reference.
     */
    [[nodiscard]] const std::vector<Entry>& entries() const;

    /**
     * \brief Merge with another data object by extending the entries of this object.
     * \param data the data object to merge with.
     */
    void mergeWith(Data&& data);

    /**
     * \brief Take the entries out of this object
     * \return Entries moved out of object.
     */
    std::vector<Entry> intoEntries() &&;

    /**
     * \brief Add an entry.
     * \param entry entry to insert.
     */
    void addEntry(Entry entry);

    /**
     * \brief Are there any entries in this data.
     * \return if there are any entries.
     */
    [[nodiscard]] bool empty() const;

private:
    std::vector<Entry> _entries;
};
}  // namespace evget::Event

#endif  // DATA_H
