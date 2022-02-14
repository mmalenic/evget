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

#ifndef EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
#define EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H

#include "AbstractData.h"
#include "AbstractField.h"
#include <chrono>
#include <optional>
#include "evget/Event/Common/Time.h"
#include "evget/Event/Cursor/PositionX.h"
#include "evget/Event/Cursor/PositionY.h"
#include "evget/Event/Common/DeviceType.h"

namespace Event {
    /**
     * Represents a mouse move event.
     */
    class MouseMove : public AbstractData {
    public:
        class MouseMoveBuilder {
        public:
            friend class MouseMove;

            MouseMoveBuilder();

            /**
             * Add time in nanoseconds.
             */
            MouseMoveBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add position x.
             */
            MouseMoveBuilder& positionX(double x);

            /**
             * Add position y.
             */
            MouseMoveBuilder& positionY(double y);

            /**
             * Mouse type.
             */
            MouseMoveBuilder& type(Event::Common::Device device);

            /**
             * Build mouse move event.
             */
            std::unique_ptr<MouseMove> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Common::DeviceType> _type;
            std::unique_ptr<Cursor::PositionX> _positionX;
            std::unique_ptr<Cursor::PositionY> _positionY;
        };

        /**
         * Create event entry.
         */
        explicit MouseMove(MouseMoveBuilder& builder);
    };
}

#endif //EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
