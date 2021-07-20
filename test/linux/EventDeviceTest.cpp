// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "EventDevice.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

using namespace std;

TEST(EventDeviceTest, OrderWithAndWithoutSymlinks) { // NOLINT(cert-err58-cpp)
    EventDevice lt{
        "/dev/input/event3",
        "",
        "",
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    EventDevice gt{
        "/dev/input/event0",
        {},
        {},
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    ASSERT_LT(lt, gt);
}

TEST(EventDeviceTest, OrderBothWithSymlinks) { // NOLINT(cert-err58-cpp)
    EventDevice lt{
        "/dev/input/event3",
        "",
        "",
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    EventDevice gt{
        "/dev/input/event4",
        "",
        "",
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    ASSERT_LT(lt, gt);
}

TEST(EventDeviceTest, OrderBothWithoutSymlinks) { // NOLINT(cert-err58-cpp)
    EventDevice lt{
        "/dev/input/event3",
        {},
        {},
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    EventDevice gt{
        "/dev/input/event4",
        {},
        {},
        "",
        {to_string(EV_SYN), to_string(EV_KEY), to_string(EV_REL), to_string(EV_MSC)}
    };
    ASSERT_LT(lt, gt);
}