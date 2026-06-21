/**
 * \file raw_event.h
 * \brief Owned raw input event that can cross a channel by value.
 */

#ifndef EVGETWINDOWS_RAW_EVENT_H
#define EVGETWINDOWS_RAW_EVENT_H

#include <windows.h>

#include <variant>

namespace evgetwindows {

/**
 * \brief Owned raw input event that can cross a boost channel and is trivially copyable.
 */
struct RawEvent {
    RAWINPUTHEADER header;
    std::variant<RAWMOUSE, RAWKEYBOARD> data;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_RAW_EVENT_H
