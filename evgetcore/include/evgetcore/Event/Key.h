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

#ifndef EVGET_SRC_KEY_H
#define EVGET_SRC_KEY_H

#include <chrono>
#include <c++/11/array>
#include "Data.h"
#include "Schema.h"
#include "ButtonAction.h"
#include "Modifier.h"

namespace EvgetCore::Event {
    /**
     * Represents a key event.
     */
    class Key {
    public:
        using SchemaType = Schema<16, Modifier::SchemaType>;

        Key() = default;

        /**
         * Add interval in microseconds.
         */
        Key& interval(SchemaField::Interval interval);

        /**
         * Add date timestamp.
         */
        Key& timestamp(SchemaField::Timestamp dateTime);

        /**
         * Key device.
         */
        Key& device(Device device);

        /**
         * Add position x.
         */
        Key& positionX(double x);

        /**
         * Add position y.
         */
        Key& positionY(double y);

        /**
         * Add action.
         */
        Key& action(ButtonAction action);

        /**
         * Add id.
         */
        Key& button(int button);

        /**
         * Add id.
         */
        Key& name(std::string name);

        /**
         * Add character.
         */
        Key& character(std::string character);

        /**
         * Add the focus window name.
         */
        Key& focusWindowName(std::string name);

        /**
         * Add the focus window position x.
         */
        Key& focusWindowPositionX(double x);

        /**
         * Add the focus window position y.
         */
        Key& focusWindowPositionY(double y);

        /**
         * Add the focus window width.
         */
        Key& focusWindowWidth(double width);

        /**
         * Add the focus window height.
         */
        Key& focusWindowHeight(double height);

        /**
         * Build key event.
         */
        Data build();

        /**
         * Generate the schema for this data.
         */
        static constexpr SchemaType generateSchema();

    private:
        std::optional<SchemaField::Interval> _interval{};
        std::optional<SchemaField::Timestamp> _timestamp{};
        std::optional<Device> _device{};
        std::optional<double> _positionX{};
        std::optional<double> _positionY{};
        std::optional<ButtonAction> _action{};
        std::optional<int> _button{};
        std::optional<std::string> _name{};
        std::optional<std::string> _character{};
        std::optional<std::string> _layout{};
        std::optional<std::string> _variant{};
        std::optional<std::string> _focusWindowName{};
        std::optional<double> _focusWindowPositionX{};
        std::optional<double> _focusWindowPositionY{};
        std::optional<double> _focusWindowWidth{};
        std::optional<double> _focusWindowHeight{};
    };

    constexpr EvgetCore::Event::Key::SchemaType EvgetCore::Event::Key::generateSchema() {
        return SchemaType{"Key", {
            SchemaField::INTERVAL_FIELD,
            SchemaField::TIMESTAMP_FIELD,
            SchemaField::DEVICE_TYPE_FIELD,
            SchemaField::POSITIONX_FIELD,
            SchemaField::POSITIONY_FIELD,
            SchemaField::ACTION_FIELD,
            SchemaField::IDENTIFIER_FIELD,
            SchemaField::NAME_FIELD,
            SchemaField::CHARACTER_FIELD,
            SchemaField::LAYOUT_FIELD,
            SchemaField::VARIANT_FIELD,
            SchemaField::FOCUS_WINDOW_NAME_FIELD,
            SchemaField::FOCUS_WINDOW_POSITION_X_FIELD,
            SchemaField::FOCUS_WINDOW_POSITION_Y_FIELD,
            SchemaField::FOCUS_WINDOW_WIDTH_FIELD,
            SchemaField::FOCUS_WINDOW_HEIGHT_FIELD}, {Modifier::generateSchema(), true}
        };
    }
}

#endif //EVGET_SRC_KEY_H
