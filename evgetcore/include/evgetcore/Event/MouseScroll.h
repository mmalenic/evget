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
#include "Modifier.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse wheel event.
     */
    class MouseScroll {
    public:
        using SchemaType = Schema<14, Modifier::SchemaType>;

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
         * Add the focus window name.
         */
        MouseScroll& focusWindowName(std::string name);

        /**
         * Add the focus window position x.
         */
        MouseScroll& focusWindowPositionX(double x);

        /**
         * Add the focus window position y.
         */
        MouseScroll& focusWindowPositionY(double y);

        /**
         * Add the focus window width.
         */
        MouseScroll& focusWindowWidth(double width);

        /**
         * Add the focus window height.
         */
        MouseScroll& focusWindowHeight(double height);

        /**
         * Build mouse wheel event.
         */
        Data build();

        /**
         * Generate the MouseScroll schema.
         */
        static constexpr SchemaType generateSchema();

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
        std::optional<std::string> _focusWindowName{};
        std::optional<double> _focusWindowPositionX{};
        std::optional<double> _focusWindowPositionY{};
        std::optional<double> _focusWindowWidth{};
        std::optional<double> _focusWindowHeight{};
    };

    constexpr EvgetCore::Event::MouseScroll::SchemaType EvgetCore::Event::MouseScroll::generateSchema() {
        return SchemaType{"MouseScroll", {
                SchemaField::INTERVAL_FIELD,
                SchemaField::TIMESTAMP_FIELD,
                SchemaField::DEVICE_TYPE_FIELD,
                SchemaField::POSITIONX_FIELD,
                SchemaField::POSITIONY_FIELD,
                SchemaField::SCROLLDOWN_FIELD,
                SchemaField::SCROLLLEFT_FIELD,
                SchemaField::SCROLLRIGHT_FIELD,
                SchemaField::SCROLLUP_FIELD,
                SchemaField::FOCUS_WINDOW_NAME_FIELD,
                SchemaField::FOCUS_WINDOW_POSITION_X_FIELD,
                SchemaField::FOCUS_WINDOW_POSITION_Y_FIELD,
                SchemaField::FOCUS_WINDOW_WIDTH_FIELD,
                SchemaField::FOCUS_WINDOW_HEIGHT_FIELD}, {Modifier::generateSchema(), true}
        };
    }
}

#endif //EVGET_SRC_EVENT_MOUSEWHEEL_H
