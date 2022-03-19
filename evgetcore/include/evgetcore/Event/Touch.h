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
#include "AbstractData.h"
#include "evgetcore/Event/Common/Time.h"
#include "evgetcore/Event/Pointer/PositionX.h"
#include "evgetcore/Event/Pointer/PositionY.h"
#include "evgetcore/Event/Pointer/EllipseMajorInner.h"
#include "evgetcore/Event/Pointer/EllipseMinorInner.h"
#include "evgetcore/Event/Pointer/PositionXOuterTest.h"
#include "evgetcore/Event/Pointer/PositionYOuterTest.h"
#include "evgetcore/Event/Pointer/EllipseMajorOuter.h"
#include "evgetcore/Event/Pointer/EllipseMinorOuter.h"
#include "evgetcore/Event/Pointer/Orientation.h"
#include "evgetcore/Event/Pointer/Multitouch.h"

namespace EvgetCore::Event {
    /**
     * Represents a key event.
     */
    class Touch : public AbstractData {
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
            std::unique_ptr<Touch> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Pointer::PositionX> _positionX;
            std::unique_ptr<Pointer::PositionY> _positionY;
            std::unique_ptr<Pointer::EllipseMajorInner> _ellipseMajorInner;
            std::unique_ptr<Pointer::EllipseMinorInner> _ellipseMinorInner;
            std::unique_ptr<Pointer::Orientation> _orientation;
            std::unique_ptr<Pointer::PositionXOuter> _positionXOuter;
            std::unique_ptr<Pointer::PositionYOuter> _positionYOuter;
            std::unique_ptr<Pointer::EllipseMajorOuter> _ellipseMajorOuter;
            std::unique_ptr<Pointer::EllipseMinorOuter> _ellipseMinorOuter;
            std::unique_ptr<Pointer::Multitouch> _multitouch;
        };

        /**
         * Create event entry.
         */
        explicit Touch(TouchBuilder& builder);
    };
}


#endif //EVGET_SRC_EVENT_TOUCH_H
