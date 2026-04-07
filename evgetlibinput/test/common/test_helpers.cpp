#include "common/test_helpers.h"

#include <utility>

#include "evgetlibinput/xkbcommon.h"

evgetlibinput::XkbCommon test::MakeUsXkb() {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    return std::move(xkb).value();
}
