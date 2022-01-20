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

#ifndef EVGET_SRC_EVENT_TOUCH_H
#define EVGET_SRC_EVENT_TOUCH_H

#include <chrono>
#include "Data.h"
#include "evget/Event/Common/Time.h"
#include "evget/Event/Cursor/PositionX.h"
#include "evget/Event/Cursor/PositionY.h"
#include "evget/Event/Cursor/EllipseMajorInner.h"
#include "evget/Event/Cursor/EllipseMinorInner.h"
#include "evget/Event/Cursor/PositionXOuterTest.h"
#include "evget/Event/Cursor/PositionYOuterTest.h"
#include "evget/Event/Cursor/EllipseMajorOuter.h"
#include "evget/Event/Cursor/EllipseMinorOuter.h"
#include "evget/Event/Cursor/Orientation.h"
#include "evget/Event/Cursor/Multitouch.h"

namespace Event {
    /**
     * Represents a key event.
     */
    class Touch : public Data {
    public:
        class TouchBuilder {
        public:
            friend class Touch;

            TouchBuilder();

            /**
             * Add time in nanoseconds.
             */
            TouchBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add position x.
             */
            TouchBuilder& positionX(int x);

            /**
             * Add position y.
             */
            TouchBuilder& positionY(int y);

            /**
             * Add ellipse major inner.
             */
            TouchBuilder& ellipseMajorInner(int width);

            /**
             * Add ellipse minor inner.
             */
            TouchBuilder& ellipseMinorInner(int width);

            /**
             * Add orientation.
             */
            TouchBuilder& orientation(int orientation);

            /**
             * Add position x outer.
             */
            TouchBuilder& positionXOuter(int x);

            /**
             * Add position y outer.
             */
            TouchBuilder& positionYOuter(int y);

            /**
             * Add ellipse major outer.
             */
            TouchBuilder& ellipseMajorOuter(int width);

            /**
             * Add ellipse minor outer.
             */
            TouchBuilder& ellipseMinorOuter(int width);

            /**
             * Add multitouch.
             */
            TouchBuilder& multitouch(int id);

            /**
             * Build touch event.
             */
            Touch build();

        private:
            Common::Time _time;
            Cursor::PositionX _positionX;
            Cursor::PositionY _positionY;
            Cursor::EllipseMajorInner _ellipseMajorInner;
            Cursor::EllipseMinorInner _ellipseMinorInner;
            Cursor::Orientation _orientation;
            Cursor::PositionXOuter _positionXOuter;
            Cursor::PositionYOuter _positionYOuter;
            Cursor::EllipseMajorOuter _ellipseMajorOuter;
            Cursor::EllipseMinorOuter _ellipseMinorOuter;
            Cursor::Multitouch _multitouch;
        };

        /**
         * Create event entry.
         */
        explicit Touch(const TouchBuilder& builder);
    };
}


#endif //EVGET_SRC_EVENT_TOUCH_H
