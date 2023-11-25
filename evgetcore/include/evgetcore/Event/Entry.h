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
#include <array>

namespace EvgetCore::Event {

namespace detail {
constexpr auto mouseMoveNFields = 12;
constexpr auto mouseScrollNFields = 14;
constexpr auto mouseClickNFields = 15;
constexpr auto keyNFields = 16;

template <std::size_t From, std::size_t To, typename AddElements>
concept AddToArray = std::ranges::range<AddElements> && To > From && requires(AddElements addElements) {
    addElements.size() <= To - From;
};

/**
 * \brief Add additional elements to an array.
 */
template <std::size_t From, std::size_t To, typename AddElements>
requires AddToArray<From, To, AddElements>
constexpr std::array<std::string_view, To>
addToArray(std::array<std::string_view, From> from, AddElements addElements) {
    std::array<std::string_view, To> out{};

    auto next = std::copy(from.begin(), from.end(), out.begin());
    std::copy(addElements.begin(), addElements.end(), next);

    return out;
}

constexpr std::array<std::string_view, mouseMoveNFields> mouseMoveFields{
    "interval",
    "timestamp",
    "position_x",
    "position_y",
    "device_name",
    "focus_window_name",
    "focus_window_position_x",
    "focus_window_position_y",
    "focus_window_width",
    "focus_window_height",
    "info",
    "device_type",
};

constexpr std::array<std::string_view, mouseScrollNFields> mouseScrollFields = addToArray<mouseMoveNFields, mouseScrollNFields>(mouseMoveFields, std::vector{
    "scroll_vertical",
    "scroll_horizontal",
});

constexpr std::array<std::string_view, mouseClickNFields> mouseClickFields = addToArray<mouseMoveNFields, mouseClickNFields>(mouseMoveFields, std::vector{
    "button_id",
    "button_name",
    "button_action",
});

constexpr std::array<std::string_view, keyNFields> keyFields = addToArray<mouseClickNFields, keyNFields>(mouseClickFields, std::vector{
    "character",
});
}

/**
 * \brief An entry type.
 */
enum class EntryType {
    Key,
    MouseClick,
    MouseMove,
    MouseScroll
};

struct EntryWithFields {
    EntryType type;
    std::vector<std::string> fields;
    std::vector<std::string> data;
    std::vector<std::string> modifiers;
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

    /**
     * \brief Rearrange the entry fields from an integer to named representation.
     */
    void toNamedRepresentation();

    /**
     * \brief Get the entry with fields.
     */
    EntryWithFields getEntryWithFields();

private:
    EntryType _type;
    std::vector<std::string> _data;
    std::vector<std::string> _modifiers;
};

} // EvgetCore

#endif //ENTRY_H
