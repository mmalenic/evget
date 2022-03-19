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

#include "evgetcore/Event/Touch.h"

EvgetCore::Event::Touch::TouchBuilder::TouchBuilder() :
    _time{std::make_unique<Common::Time>()},
    _positionX{std::make_unique<Pointer::PositionX>()},
    _positionY{std::make_unique<Pointer::PositionY>()},
    _ellipseMajorInner{std::make_unique<Pointer::EllipseMajorInner>()},
    _ellipseMinorInner{std::make_unique<Pointer::EllipseMinorInner>()},
    _orientation{std::make_unique<Pointer::Orientation>()},
    _positionXOuter{std::make_unique<Pointer::PositionXOuter>()},
    _positionYOuter{std::make_unique<Pointer::PositionYOuter>()},
    _ellipseMajorOuter{std::make_unique<Pointer::EllipseMajorOuter>()},
    _ellipseMinorOuter{std::make_unique<Pointer::EllipseMinorOuter>()},
    _multitouch{std::make_unique<Pointer::Multitouch>()}
    {
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::positionX(int x) {
    _positionX = std::make_unique<Pointer::PositionX>(x);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::positionY(int y) {
    _positionY = std::make_unique<Pointer::PositionY>(y);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::ellipseMajorInner(int width) {
    _ellipseMajorInner = std::make_unique<Pointer::EllipseMajorInner>(width);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::ellipseMinorInner(int width) {
    _ellipseMinorInner = std::make_unique<Pointer::EllipseMinorInner>(width);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::orientation(int orientation) {
    _orientation = std::make_unique<Pointer::Orientation>(orientation);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::positionXOuter(int x) {
    _positionXOuter = std::make_unique<Pointer::PositionXOuter>(x);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::positionYOuter(int y) {
    _positionYOuter = std::make_unique<Pointer::PositionYOuter>(y);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::ellipseMajorOuter(int width) {
    _ellipseMajorOuter = std::make_unique<Pointer::EllipseMajorOuter>(width);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::ellipseMinorOuter(int width) {
    _ellipseMinorOuter = std::make_unique<Pointer::EllipseMinorOuter>(width);
    return *this;
}

EvgetCore::Event::Touch::TouchBuilder& EvgetCore::Event::Touch::TouchBuilder::multitouch(int id) {
    _multitouch = std::make_unique<Pointer::Multitouch>(id);
    return *this;
}

std::unique_ptr<EvgetCore::Event::Touch> EvgetCore::Event::Touch::TouchBuilder::build() {
    return std::make_unique<EvgetCore::Event::Touch>(*this);
}

EvgetCore::Event::Touch::Touch(
    EvgetCore::Event::Touch::TouchBuilder& builder
) : AbstractData{"Touch"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._positionX));
    fields.emplace_back(std::move(builder._positionY));
    fields.emplace_back(std::move(builder._ellipseMajorInner));
    fields.emplace_back(std::move(builder._ellipseMinorInner));
    fields.emplace_back(std::move(builder._orientation));
    fields.emplace_back(std::move(builder._positionXOuter));
    fields.emplace_back(std::move(builder._positionYOuter));
    fields.emplace_back(std::move(builder._ellipseMajorOuter));
    fields.emplace_back(std::move(builder._ellipseMinorOuter));
    fields.emplace_back(std::move(builder._multitouch));
}