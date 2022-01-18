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

#include "evget/Event/MouseMove.h"

Event::MouseMove::MouseMoveBuilder::MouseMoveBuilder() : _time{}, _positionX{}, _positionY{} {
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = Common::Time{nanoseconds};
    return *this;
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::positionX(int x) {
    _positionX = Cursor::PositionX{x};
    return *this;
}

Event::MouseMove::MouseMoveBuilder& Event::MouseMove::MouseMoveBuilder::positionY(int y) {
    _positionY = Cursor::PositionY{y};
    return *this;
}

Event::MouseMove Event::MouseMove::MouseMoveBuilder::build() {
    return Event::MouseMove(*this);
}

Event::MouseMove::MouseMove(
    const Event::MouseMove::MouseMoveBuilder& builder
) : Data{"MouseMove", {builder._time, builder._positionX, builder._positionY}} {
}
