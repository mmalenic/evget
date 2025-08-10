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

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace evget::event {

namespace detail {
constexpr auto kMouseMoveNFields = 12;
constexpr auto kMouseScrollNFields = 14;
constexpr auto kMouseClickNFields = 15;
constexpr auto kEyNFields = 16;

template <std::size_t From, std::size_t To, typename AddElements>
concept AddArray = std::ranges::range<AddElements> &&
    To > From &&
    requires(AddElements add_elements) { add_elements.size() <= To - From; };

/**
 * \brief Add additional elements to an array.
 */
template <std::size_t From, std::size_t To, typename AddElements>
    requires AddArray<From, To, AddElements>
constexpr std::array<std::string_view, To>
AddToArray(std::array<std::string_view, From> from, AddElements add_elements) {
    std::array<std::string_view, To> out{};

    auto next = std::copy(from.begin(), from.end(), out.begin());
    std::copy(add_elements.begin(), add_elements.end(), next);

    return out;
}

constexpr std::array<std::string_view, kMouseMoveNFields> kMouseMoveFields{
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
    "screen",
    "device_type",
};

constexpr std::array<std::string_view, kMouseScrollNFields> kMouseScrollFields =
    AddToArray<kMouseMoveNFields, kMouseScrollNFields>(
        kMouseMoveFields,
        std::vector{
            "scroll_vertical",
            "scroll_horizontal",
        }
    );

constexpr std::array<std::string_view, kMouseClickNFields> kMouseClickFields =
    AddToArray<kMouseMoveNFields, kMouseClickNFields>(
        kMouseMoveFields,
        std::vector{
            "button_id",
            "button_name",
            "button_action",
        }
    );

constexpr std::array<std::string_view, kEyNFields> kEyFields = AddToArray<kMouseClickNFields, kEyNFields>(
    kMouseClickFields,
    std::vector{
        "character",
    }
);
}  // namespace detail

/**
 * \brief An entry type.
 */
enum class EntryType : std::uint8_t { kKey, kMouseClick, kMouseMove, kMouseScroll };

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

    [[nodiscard]] EntryType Type() const;
    [[nodiscard]] const std::vector<std::string>& Data() const;
    [[nodiscard]] const std::vector<std::string>& Modifiers() const;

    /**
     * \brief Rearrange the entry fields from an integer to named representation.
     */
    void ToNamedRepresentation();

    /**
     * \brief Get the entry with fields.
     */
    [[nodiscard]] EntryWithFields GetEntryWithFields() const;

private:
    EntryType type_;
    std::vector<std::string> data_;
    std::vector<std::string> modifiers_;
};

}  // namespace evget::event

#endif  // ENTRY_H
