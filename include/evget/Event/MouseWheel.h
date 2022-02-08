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

#ifndef EVGET_SRC_EVENT_MOUSEWHEEL_H
#define EVGET_SRC_EVENT_MOUSEWHEEL_H

#include <chrono>
#include "AbstractData.h"
#include "evget/Event/Common/Time.h"
#include "evget/Event/Cursor/WheelDown.h"
#include "evget/Event/Cursor/WheelUp.h"
#include "evget/Event/Cursor/IsTouchPad.h"

namespace Event {
    /**
     * Represents a mouse wheel event.
     */
    class MouseWheel : public AbstractData {
    public:
        class MouseWheelBuilder {
        public:
            friend class MouseWheel;

            MouseWheelBuilder();

            /**
             * Add time in nanoseconds.
             */
            MouseWheelBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add wheel down.
             */
            MouseWheelBuilder& wheelDown(int amount);

            /**
             * Add wheel up.
             */
            MouseWheelBuilder& wheelUp(int amount);

            /**
             * Mouse is a touchpad device.
             */
            MouseWheelBuilder& touchPad(bool isTouchPad);

            /**
             * Build mouse wheel event.
             */
            MouseWheel build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Cursor::WheelDown> _wheelDown;
            std::unique_ptr<Cursor::WheelUp> _wheelUp;
            std::unique_ptr<Cursor::IsTouchPad> _isTouchPad;
        };

        /**
         * Create event entry.
         */
        explicit MouseWheel(MouseWheelBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
