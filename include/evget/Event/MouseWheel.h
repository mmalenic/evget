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
#include "evget/Event/Pointer/WheelDirection.h"
#include "evget/Event/Pointer/WheelAmount.h"

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
             * Mouse device.
             */
            MouseWheelBuilder& device(Event::Common::Device device);

            /**
             * Add wheel down.
             */
            MouseWheelBuilder& direction(Pointer::Direction direction);

            /**
             * Add wheel up.
             */
            MouseWheelBuilder& amount(double amount);

            /**
             * Build mouse wheel event.
             */
            std::unique_ptr<MouseWheel> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Common::DeviceType> _device;
            std::unique_ptr<Pointer::WheelDirection> _direction;
            std::unique_ptr<Pointer::WheelAmount> _amount;
        };

        /**
         * Create event entry.
         */
        explicit MouseWheel(MouseWheelBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
