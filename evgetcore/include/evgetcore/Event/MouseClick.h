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
#include "AbstractData.h"
#include "evgetcore/Event/Common/Time.h"
#include "evgetcore/Event/Pointer/PositionX.h"
#include "evgetcore/Event/Pointer/PositionY.h"
#include "evgetcore/Event/Common/DeviceType.h"
#include "evgetcore/Event/Button/ButtonAction.h"
#include "evgetcore/Event/Button/Action.h"
#include "evgetcore/Event/Button/Identifier.h"
#include "evgetcore/Event/Button/Name.h"
#include "evgetcore/Event/Common/DateTime.h"

namespace EvgetCore::Event {
    /**
     * Represents a mouse click event.
     */
    class MouseClick : public AbstractData {
    public:
        class MouseClickBuilder {
        public:
            friend class MouseClick;

            MouseClickBuilder() = default;

            /**
             * Add time in nanoseconds.
             */
            MouseClickBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add date time
             */
            MouseClickBuilder& dateTime(Common::DateTime::TimePoint timePoint);

            /**
             * Mouse device.
             */
            MouseClickBuilder& device(Common::Device device);

            /**
             * Add position x.
             */
            MouseClickBuilder& positionX(double x);

            /**
             * Add position y.
             */
            MouseClickBuilder& positionY(double y);

            /**
             * Add action.
             */
            MouseClickBuilder& action(Button::ButtonAction action);

            /**
             * Add id.
             */
            MouseClickBuilder& button(int button);

            /**
             * Add name.
             */
            MouseClickBuilder& name(const std::string& name);

            /**
             * Build mouse click event.
             */
            std::unique_ptr<MouseClick> build();

        private:
            std::unique_ptr<Common::Time> _time{};
            std::unique_ptr<Common::DateTime> _dateTime{};
            std::unique_ptr<Common::DeviceType> _device{};
            std::unique_ptr<Pointer::PositionX> _positionX{};
            std::unique_ptr<Pointer::PositionY> _positionY{};
            std::unique_ptr<Button::Action> _buttonType{};
            std::unique_ptr<Button::Identifier> _buttonId{};
            std::unique_ptr<Button::Name> _buttonName{};
        };

        /**
         * Create event entry.
         */
        explicit MouseClick(MouseClickBuilder& builder);
    };
}

#endif //EVGET_SRC_EVENT_MOUSECLICK_H
