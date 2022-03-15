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
#include "evget/Event/Common/DeviceType.h"
#include "evget/Event/Pointer/ScrollUp.h"
#include "evget/Event/Pointer/ScrollDown.h"
#include "evget/Event/Pointer/ScrollLeft.h"
#include "evget/Event/Pointer/ScrollRight.h"

namespace Event {
    /**
     * Represents a mouse wheel event.
     */
    class MouseScroll : public AbstractData {
    public:
        class MouseScrollBuilder {
        public:
            friend class MouseScroll;

            MouseScrollBuilder();

            /**
             * Add time in nanoseconds.
             */
            MouseScrollBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Mouse device.
             */
            MouseScrollBuilder& device(Event::Common::Device device);

            /**
             * Add scroll up.
             */
            MouseScrollBuilder& up(double amount);

            /**
             * Add scroll down.
             */
            MouseScrollBuilder& down(double amount);

            /**
             * Add scroll left.
             */
            MouseScrollBuilder& left(double amount);

            /**
             * Add scroll right.
             */
            MouseScrollBuilder& right(double amount);

            /**
             * Build mouse wheel event.
             */
            std::unique_ptr<MouseScroll> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Common::DeviceType> _device;
            std::unique_ptr<Pointer::ScrollUp> _up;
            std::unique_ptr<Pointer::ScrollDown> _down;
            std::unique_ptr<Pointer::ScrollLeft> _left;
            std::unique_ptr<Pointer::ScrollRight> _right;
        };

        /**
         * Create event entry.
         */
        explicit MouseScroll(MouseScrollBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
