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

#include "evget/Event/Touch.h"

Event::Touch::TouchBuilder::TouchBuilder() :
    _time{},
    _positionX{},
    _positionY{},
    _ellipseMajorInner{},
    _ellipseMinorInner{},
    _orientation{},
    _positionXOuter{},
    _positionYOuter{},
    _ellipseMajorOuter{},
    _ellipseMinorOuter{},
    _multitouch{}
    {
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = Common::Time{nanoseconds};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::positionX(int x) {
    _positionX = Cursor::PositionX{x};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::positionY(int y) {
    _positionY = Cursor::PositionY{y};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::ellipseMajorInner(int width) {
    _ellipseMajorInner = Cursor::EllipseMajorInner{width};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::ellipseMinorInner(int width) {
    _ellipseMinorInner = Cursor::EllipseMinorInner{width};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::orientation(int orientation) {
    _orientation = Cursor::Orientation{orientation};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::positionXOuter(int x) {
    _positionXOuter = Cursor::PositionXOuter{x};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::positionYOuter(int y) {
    _positionYOuter = Cursor::PositionYOuter{y};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::ellipseMajorOuter(int width) {
    _ellipseMajorOuter = Cursor::EllipseMajorOuter{width};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::ellipseMinorOuter(int width) {
    _ellipseMinorOuter = Cursor::EllipseMinorOuter{width};
    return *this;
}

Event::Touch::TouchBuilder& Event::Touch::TouchBuilder::multitouch(int id) {
    _multitouch = Cursor::Multitouch{id};
    return *this;
}

Event::Touch Event::Touch::TouchBuilder::build() {
    return Event::Touch(*this);
}

Event::Touch::Touch(
    const Event::Touch::TouchBuilder& builder
) : Data{"Touch", {
    builder._time,
    builder._positionX,
    builder._positionY,
    builder._ellipseMajorInner,
    builder._ellipseMinorInner,
    builder._orientation,
    builder._positionXOuter,
    builder._positionYOuter,
    builder._ellipseMajorOuter,
    builder._ellipseMinorOuter,
    builder._multitouch
}} {
}