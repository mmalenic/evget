/**
 * \file entry.h
 * \brief Event entry types and field definitions for structured event data.
 */

#ifndef EVGET_EVENT_ENTRY_H
#define EVGET_EVENT_ENTRY_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace evget {

namespace detail {
/// \brief Number of fields in a mouse move event entry.
constexpr auto kMouseMoveNFields = 12;

/// \brief Number of fields in a mouse scroll event entry.
constexpr auto kMouseScrollNFields = 14;

/// \brief Number of fields in a mouse click event entry.
constexpr auto kMouseClickNFields = 15;

/// \brief Number of fields in a key event entry.
constexpr auto kKeyNFields = 16;

template <std::size_t From, std::size_t To, typename AddElements>
concept AddArray = std::ranges::range<AddElements> &&
    To > From&& requires(AddElements add_elements) { add_elements.size() <= To - From; };

/**
 * \brief Add additional elements to an array.
 * \tparam From Size of the source array
 * \tparam To Size of the target array
 * \tparam AddElements Type of elements to add
 * \param from Source array to copy from
 * \param add_elements Elements to add to the array
 * \return New array with combined elements
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

/// \brief Field names for mouse move events.
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

/// \brief Field names for mouse scroll events (extends mouse move fields).
constexpr std::array<std::string_view, kMouseScrollNFields> kMouseScrollFields =
    AddToArray<kMouseMoveNFields, kMouseScrollNFields>(
        kMouseMoveFields,
        std::vector{
            "scroll_vertical",
            "scroll_horizontal",
        }
    );

/// \brief Field names for mouse click events (extends mouse move fields).
constexpr std::array<std::string_view, kMouseClickNFields> kMouseClickFields =
    AddToArray<kMouseMoveNFields, kMouseClickNFields>(
        kMouseMoveFields,
        std::vector{
            "button_id",
            "button_name",
            "button_action",
        }
    );

/// \brief Field names for key events (extends mouse click fields).
constexpr std::array<std::string_view, kKeyNFields> kKeyFields = AddToArray<kMouseClickNFields, kKeyNFields>(
    kMouseClickFields,
    std::vector{
        "character",
    }
);
} // namespace detail

/**
 * \brief An entry type.
 */
enum class EntryType : std::uint8_t {
    kKey, ///< A key entry
    kMouseClick, ///< A mouse click entry
    kMouseMove, ///< A mouse move entry
    kMouseScroll ///< A mouse scroll entry
};

/**
 * \brief An entry with its associated field names for easier processing.
 */
struct EntryWithFields {
    EntryType type; ///< Type of the entry
    std::vector<std::string> fields; ///< Field names for the data
    std::vector<std::string> data; ///< Actual data values
    std::vector<std::string> modifiers; ///< Modifier values
};

/**
 * \brief An entry contains data, modifiers, and its type.
 */
class Entry {
public:
    /**
     * \brief Construct an entry with type, data, and modifiers.
     * \param type Type of the entry
     * \param data Data values for the entry
     * \param modifiers Modifier values for the entry
     */
    Entry(EntryType type, const std::vector<std::string>& data, std::vector<std::string> modifiers);

    /**
     * \brief Get the type of this entry.
     * \return Entry type
     */
    [[nodiscard]] EntryType Type() const;

    /**
     * \brief Get the data values of this entry.
     * \return Reference to data vector
     */
    [[nodiscard]] const std::vector<std::string>& Data() const;

    /**
     * \brief Get the modifier values of this entry.
     * \return Reference to modifiers vector
     */
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

} // namespace evget

#endif
