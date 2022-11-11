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

namespace EvgetCore::Event {
    /**
     * Represents a mouse move event.
     */
    class MouseMove {
    public:
        MouseMove() = default;

        /**
         * Add time in nanoseconds.
         */
        MouseMove& time(std::chrono::nanoseconds nanoseconds);

        /**
         * Add date time
         */
        MouseMove& dateTime(SchemaField::Timestamp dateTime);

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
         * Build key event.
         */
        Data build();

    private:
        std::optional<std::chrono::nanoseconds> _time{};
        std::optional<SchemaField::Timestamp> _dateTime{};
        std::optional<Device> _device{};
        std::optional<double> _positionX{};
        std::optional<double> _positionY{};
    };
}

#endif //EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
