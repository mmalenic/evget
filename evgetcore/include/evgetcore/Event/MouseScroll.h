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
#include "Data.h"
#include "Schema.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse wheel event.
     */
    class MouseScroll {
    public:
        MouseScroll() = default;

        /**
         * Add interval in microseconds.
         */
        MouseScroll& interval(SchemaField::Interval interval);

        /**
         * Add date timestamp
         */
        MouseScroll& timestamp(SchemaField::Timestamp timestamp);

        /**
         * Mouse device.
         */
        MouseScroll& device(Device device);

        /**
         * Add position x.
         */
        MouseScroll& positionX(double x);

        /**
         * Add position y.
         */
        MouseScroll& positionY(double y);

        /**
         * Add scroll down.
         */
        MouseScroll& down(double amount);

        /**
         * Add scroll left.
         */
        MouseScroll& left(double amount);

        /**
         * Add scroll right.
         */
        MouseScroll& right(double amount);

        /**
         * Add scroll up.
         */
        MouseScroll& up(double amount);

        /**
         * Build mouse wheel event.
         */
        Data build();

        /**
         * Generate the MouseScroll schema.
         */
        static constexpr Schema<9> generateSchema();

    private:
        std::optional<SchemaField::Interval> _interval{};
        std::optional<SchemaField::Timestamp> _timestamp{};
        std::optional<Device> _device{};
        std::optional<double> _positionX{};
        std::optional<double> _positionY{};
        std::optional<double> _down{};
        std::optional<double> _left{};
        std::optional<double> _right{};
        std::optional<double> _up{};
    };

    constexpr EvgetCore::Event::Schema<9> EvgetCore::Event::MouseScroll::generateSchema() {
        return Schema<9>{"MouseScroll", {
                SchemaField::INTERVAL_FIELD,
                SchemaField::TIMESTAMP_FIELD,
                SchemaField::DEVICE_TYPE_FIELD,
                SchemaField::POSITIONX_FIELD,
                SchemaField::POSITIONY_FIELD,
                SchemaField::SCROLLDOWN_FIELD,
                SchemaField::SCROLLLEFT_FIELD,
                SchemaField::SCROLLRIGHT_FIELD,
                SchemaField::SCROLLUP_FIELD},
        };
    }
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
