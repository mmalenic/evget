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

#include <chrono>
#include <optional>
#include "Data.h"
#include "Schema.h"
#include "Modifier.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse move event.
     */
    class MouseMove {
    public:
        using SchemaType = Schema<10, Modifier::SchemaType>;

        /**
         * Add interval in microseconds.
         */
        MouseMove& interval(SchemaField::Interval interval);

        /**
         * Add date timestamp.
         */
        MouseMove& timestamp(SchemaField::Timestamp timestamp);

        /**
         * Mouse device.
         */
        MouseMove& device(Device device);

        /**
         * Add position x.
         */
        MouseMove& positionX(double x);

        /**
         * Add position y.
         */
        MouseMove& positionY(double y);

        /**
         * Add the focus window name.
         */
        MouseMove& focusWindowName(std::string name);

        /**
         * Add the focus window position x.
         */
        MouseMove& focusWindowPositionX(double x);

        /**
         * Add the focus window position y.
         */
        MouseMove& focusWindowPositionY(double y);

        /**
         * Add the focus window width.
         */
        MouseMove& focusWindowWidth(double width);

        /**
         * Add the focus window height.
         */
        MouseMove& focusWindowHeight(double height);

        /**
         * Build key event.
         */
        Data build();

        /**
         * Generate the MouseMove schema.
         */
        static constexpr SchemaType generateSchema();

    private:
        std::optional<SchemaField::Interval> _interval{};
        std::optional<SchemaField::Timestamp> _timestamp{};
        std::optional<Device> _device{};
        std::optional<double> _positionX{};
        std::optional<double> _positionY{};
        std::optional<std::string> _focusWindowName{};
        std::optional<double> _focusWindowPositionX{};
        std::optional<double> _focusWindowPositionY{};
        std::optional<double> _focusWindowWidth{};
        std::optional<double> _focusWindowHeight{};
    };

    constexpr EvgetCore::Event::MouseMove::SchemaType EvgetCore::Event::MouseMove::generateSchema() {
        return SchemaType{"MouseMove", {
                SchemaField::INTERVAL_FIELD,
                SchemaField::TIMESTAMP_FIELD,
                SchemaField::DEVICE_TYPE_FIELD,
                SchemaField::POSITIONX_FIELD,
                SchemaField::POSITIONY_FIELD,
                SchemaField::FOCUS_WINDOW_NAME_FIELD,
                SchemaField::FOCUS_WINDOW_POSITION_X_FIELD,
                SchemaField::FOCUS_WINDOW_POSITION_Y_FIELD,
                SchemaField::FOCUS_WINDOW_WIDTH_FIELD,
                SchemaField::FOCUS_WINDOW_HEIGHT_FIELD}, {Modifier::generateSchema(), true}
        };
    }
}

#endif //EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
