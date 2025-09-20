#include "evgetx11/event_switch_touch.h"

#include <X11/extensions/XInput2.h>

#include <optional>
#include <string>

#include "evget/event/device_type.h"
#include "evgetx11/event_switch.h"

void evgetx11::EventSwitchTouch::RefreshDevices(
    int /* device_id */,
    std::optional<int> /* pointer_id */,
    evget::DeviceType /* device */,
    const std::string& /* name */,
    const XIDeviceInfo& /* info */,
    EventSwitch& /* x_event_switch */
) {
    // Nothing to do, just matching expected template interface.
}
