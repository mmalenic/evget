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

#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <vector>

namespace EvgetCore::Event {

/**
 * \brief An entry type.
 */
enum class EntryType {
    Key,
    MouseClick,
    MouseMove,
    MouseScroll
};

/**
 * \brief An entry contains data, modifiers, and its type.
 */
class Entry {
public:
    Entry(EntryType type, std::vector<std::string> data, std::vector<std::string> modifiers);

    [[nodiscard]] EntryType type() const;
    [[nodiscard]] const std::vector<std::string>& data() const;
    [[nodiscard]] const std::vector<std::string>& modifiers() const;
private:
    EntryType _type;
    std::vector<std::string> _data;
    std::vector<std::string> _modifiers;
};

} // EvgetCore

#endif //ENTRY_H
