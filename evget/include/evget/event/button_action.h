#ifndef EVGET_EVENT_BUTTON_ACTION_H
#define EVGET_EVENT_BUTTON_ACTION_H

#include <cstdint>

namespace evget {
enum class ButtonAction : std::uint8_t { kPress, kRelease, kRepeat };
} // namespace evget

#endif
