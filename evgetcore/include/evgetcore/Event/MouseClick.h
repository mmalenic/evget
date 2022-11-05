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

#include <memory>
#include "Data.h"
#include "Schema.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse click event.
     */
    class MouseClick {
    public:
        MouseClick() = default;

        /**
         * Add time in nanoseconds.
         */
        MouseClick& time(std::chrono::nanoseconds nanoseconds);

        /**
         * Add date time
         */
        MouseClick& dateTime(Schema::DateTime dateTime);

        /**
         * Mouse device.
         */
        MouseClick& device(Device device);

        /**
         * Add position x.
         */
        MouseClick& positionX(double x);

        /**
         * Add position y.
         */
        MouseClick& positionY(double y);

        /**
         * Add action.
         */
        MouseClick& action(ButtonAction action);

        /**
         * Add id.
         */
        MouseClick& button(int button);

        /**
         * Add name.
         */
        MouseClick& name(std::string name);

        /**
         * Build mouse click event.
         */
        Data build();

    private:
        std::optional<std::chrono::nanoseconds> _time{};
        std::optional<Schema::DateTime> _dateTime{};
        std::optional<Device> _device{};
        std::optional<double> _positionX{};
        std::optional<double> _positionY{};
        std::optional<ButtonAction> _action{};
        std::optional<int> _button{};
        std::optional<std::string> _name{};
    };
}

#endif //EVGET_SRC_EVENT_MOUSECLICK_H
