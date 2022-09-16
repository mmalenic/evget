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

#include "EvgetX11TestUtils.h"

XIValuatorClassInfo EvgetX11TestUtils::createXIValuatorClassInfo() {
    return XIValuatorClassInfo {
            .type = XIValuatorClass,
            .sourceid = 1,
            .number = 1,
            .label = 1,
            .min = 0,
            .max = 0,
            .value = 1,
            .resolution = 1,
            .mode = XIModeAbsolute
    };
}

XIScrollClassInfo EvgetX11TestUtils::createXIScrollClassInfo() {
    return XIScrollClassInfo {
            .type = XIScrollClass,
            .sourceid = 1,
            .number = 1,
            .scroll_type = XIScrollTypeVertical,
            .increment = 1,
            .flags = 0
    };
}

XIButtonClassInfo EvgetX11TestUtils::createXIButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask) {
    return XIButtonClassInfo{
            .type = XIButtonClass,
            .sourceid = 1,
            .num_buttons = static_cast<int>(labels.size()),
            .labels = labels.data(),
            .state = XIButtonState {
                    .mask_len = static_cast<int>(mask.size()),
                    .mask = mask.data(),
            },
    };
}