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
#include "evgetcore/Event/Common/Time.h"
#include "evgetcore/Event/Common/DeviceType.h"
#include "evgetcore/Event/Pointer/ScrollUp.h"
#include "evgetcore/Event/Pointer/ScrollDown.h"
#include "evgetcore/Event/Pointer/ScrollLeft.h"
#include "evgetcore/Event/Pointer/ScrollRight.h"
#include "evgetcore/Event/Pointer/PositionY.h"
#include "evgetcore/Event/Pointer/PositionX.h"
#include "evgetcore/Event/Common/DateTime.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse wheel event.
     */
    class MouseScroll : public AbstractData {
    public:
        class MouseScrollBuilder {
        public:
            friend class MouseScroll;

            MouseScrollBuilder() = default;

            /**
             * Add time in nanoseconds.
             */
            MouseScrollBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add date time
             */
            MouseScrollBuilder& dateTime(Common::DateTime::TimePoint timePoint);

            /**
             * Mouse device.
             */
            MouseScrollBuilder& device(EvgetCore::Event::Common::Device device);

            /**
             * Add position x.
             */
            MouseScrollBuilder& positionX(double x);

            /**
             * Add position y.
             */
            MouseScrollBuilder& positionY(double y);

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
            std::unique_ptr<Common::Time> _time{};
            std::unique_ptr<Common::DateTime> _dateTime{};
            std::unique_ptr<Common::DeviceType> _device{};
            std::unique_ptr<Pointer::PositionX> _positionX{};
            std::unique_ptr<Pointer::PositionY> _positionY{};
            std::unique_ptr<Pointer::ScrollUp> _up{};
            std::unique_ptr<Pointer::ScrollDown> _down{};
            std::unique_ptr<Pointer::ScrollLeft> _left{};
            std::unique_ptr<Pointer::ScrollRight> _right{};
        };

        /**
         * Create event entry.
         */
        explicit MouseScroll(MouseScrollBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
