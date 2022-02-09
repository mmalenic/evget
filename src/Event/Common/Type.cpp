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

#include <fmt/format.h>
#include "evget/Event/Common/Type.h"

Event::Common::Type::Type(std::string type) : AbstractField{"Type", std::move(type)} {
}

Event::Common::Type::Type() : Type{{}} {
}

std::unique_ptr<Event::Common::Type> Event::Common::Type::createType(Device device) {
    switch (device) {
    case Device::Mouse:
        return std::make_unique<Type>("Mouse");;
    case Device::Keyboard:
        return std::make_unique<Type>("Keyboard");
    case Device::Touchpad:
        return std::make_unique<Type>("Touchpad");
    case Device::Touchscreen:
        return std::make_unique<Type>("Touchscreen");
    }
}
