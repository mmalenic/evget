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

#ifndef EVGET_SRC_EVENT_MOUSECLICK_H
#define EVGET_SRC_EVENT_MOUSECLICK_H

#include "Data.h"
#include "evget/Event/Common/Time.h"
#include "evget/Event/Cursor/PositionX.h"
#include "evget/Event/Cursor/PositionY.h"
#include "evget/Event/Button/Press.h"
#include "evget/Event/Button/Release.h"

namespace Event {
    /**
     * Represents a mouse click event.
     */
    class MouseClick : public Data {
    public:
        class MouseClickBuilder {
        public:
            friend class MouseClick;

            MouseClickBuilder();

            /**
             * Add time in nanoseconds.
             */
            MouseClickBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add position x.
             */
            MouseClickBuilder& positionX(int x);

            /**
             * Add position y.
             */
            MouseClickBuilder& positionY(int y);

            /**
             * Add press.
             */
            MouseClickBuilder& press(std::string button);

            /**
             * Add release.
             */
            MouseClickBuilder& release(std::string button);

            /**
             * Build mouse click event.
             */
            MouseClick build();

        private:
            Common::Time _time;
            Cursor::PositionX _positionX;
            Cursor::PositionY _positionY;
            Button::Press _press;
            Button::Release _release;
        };

        /**
         * Create event entry.
         */
        explicit MouseClick(const MouseClickBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSECLICK_H
