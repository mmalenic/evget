#include "evgetlibinput/evdev.h"

#include <libevdev/libevdev.h>

const char* evgetlibinput::Evdev::EventCodeName(unsigned int type, unsigned int code) {
    return libevdev_event_code_get_name(type, code);
}
