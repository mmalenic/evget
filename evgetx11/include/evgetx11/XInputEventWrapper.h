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

#ifndef EVGET_XINPUTEVENTWRAPPER_H
#define EVGET_XINPUTEVENTWRAPPER_H

#include <chrono>
#include "evgetcore/Event/Field.h"

namespace EvgetX11 {
    template<typename T>
    class XInputEventWrapper {
    public:
        using Timestamp = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

        /**
         * Get the timestamp of the event.
         */
        [[nodiscard]] virtual Timestamp getTimestamp() const = 0;

        /**
         * Get the date time of the event.
         */
        [[nodiscard]] virtual const EvgetCore::Event::Field::DateTime &getDateTime() const = 0;

        /**
         * A non owning reference to the data in the event cookie. Must check if data is available first
         * with hasData.
         */
        virtual std::optional<std::reference_wrapper<T>> viewData() const = 0;

        XInputEventWrapper() = default;
        virtual ~XInputEventWrapper() = default;

        XInputEventWrapper(XInputEventWrapper&&) noexcept = delete;
        XInputEventWrapper& operator=(XInputEventWrapper&&) noexcept = delete;

        XInputEventWrapper(const XInputEventWrapper&) = delete;
        XInputEventWrapper& operator=(const XInputEventWrapper&) = delete;
    };
}

#endif //EVGET_XINPUTEVENTWRAPPER_H
