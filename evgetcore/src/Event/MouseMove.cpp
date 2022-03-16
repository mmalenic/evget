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

#include "evgetcore/Event/MouseMove.h"

Event::MouseMove::MouseMoveBuilder::MouseMoveBuilder() :
_time{std::make_unique<Common::Time>()},
_device{std::make_unique<Common::DeviceType>()},
_positionX{std::make_unique<Pointer::PositionX>()},
_positionY{std::make_unique<Pointer::PositionY>()} {
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::positionX(double x) {
    _positionX = std::make_unique<Pointer::PositionX>(x);
    return *this;
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::positionY(double y) {
    _positionY = std::make_unique<Pointer::PositionY>(y);
    return *this;
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::device(Event::Common::Device device) {
    _device = Event::Common::DeviceType::createType(device);
    return *this;
}

std::unique_ptr<Event::MouseMove> Event::MouseMove::MouseMoveBuilder::build() {
    return std::make_unique<Event::MouseMove>(*this);
}

Event::MouseMove::MouseMove(
    Event::MouseMove::MouseMoveBuilder& builder
) : AbstractData{"MouseMove"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._device));
    fields.emplace_back(std::move(builder._positionX));
    fields.emplace_back(std::move(builder._positionY));
}
