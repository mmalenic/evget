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
#include "evgetcore/Event/Common/Time.h"
#include "evgetcore/Event/Pointer/PositionX.h"
#include "evgetcore/Event/Pointer/PositionY.h"
#include "evgetcore/Event/Common/DeviceType.h"
#include "evgetcore/Event/Common/DateTime.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse move event.
     */
    class MouseMove : public AbstractData {
    public:
        class MouseMoveBuilder {
        public:
            friend class MouseMove;

            MouseMoveBuilder() = default;

            /**
             * Add time in nanoseconds.
             */
            MouseMoveBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add date time
             */
            MouseMoveBuilder& dateTime(Common::DateTime::TimePoint timePoint);

            /**
             * Mouse device.
             */
            MouseMoveBuilder& device(EvgetCore::Event::Common::Device device);

            /**
             * Add position x.
             */
            MouseMoveBuilder& positionX(double x);

            /**
             * Add position y.
             */
            MouseMoveBuilder& positionY(double y);

            /**
             * Build mouse move event.
             */
            std::unique_ptr<MouseMove> build();

        private:
            std::unique_ptr<Common::Time> _time{};
            std::unique_ptr<Common::DateTime> _dateTime{};
            std::unique_ptr<Common::DeviceType> _device{};
            std::unique_ptr<Pointer::PositionX> _positionX{};
            std::unique_ptr<Pointer::PositionY> _positionY{};
        };

        /**
         * Create event entry.
         */
        explicit MouseMove(MouseMoveBuilder& builder);
    };
}

#endif //EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
