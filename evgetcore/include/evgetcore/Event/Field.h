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

#ifndef EVGET_SRC_EVENT_FIELD_H
#define EVGET_SRC_EVENT_FIELD_H

#include <c++/11/chrono>
#include "AbstractField.h"
#include "evgetcore/Event/Button/ButtonAction.h"
#include "evgetcore/Event/Common/Device.h"
#include "evgetcore/Event/Pointer/Direction.h"

namespace EvgetCore::Event {
    /**
     * Represents a field in an event.
     */
    class Field {
    public:
        using Entries = std::vector<std::unique_ptr<AbstractData>>;
        using Iterator = Entries::const_iterator;
        using EntryOrData = std::variant<std::string, Entries>;
        using DateTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

        Field(std::string name, Entries entries);
        Field(std::string name, std::string entry);

        /**
         * Create a Field using string_view, which copies over the data to a string.
         */
        Field(std::string_view name, std::string_view entry);

        /**
         * Create a Field using string_view, which copies over the data to a string.
         */
        Field(std::string_view name, std::string entry);

        explicit Field(std::string name);

        /**
         * Check if variant is the entry.
         */
        constexpr bool isEntry();

        /**
         * Check if variant is more data.
         */
        constexpr bool isData();

        /**
         * Get the entry.
         */
        [[nodiscard]] std::string getEntry() const;

        /**
         * Get the name.
         */
        [[nodiscard]] std::string getName() const;

        [[nodiscard]] Iterator begin() const;

        [[nodiscard]] Iterator end() const;

        /**
         * Create an Action based on the ButtonAction enum.
         */
        static Field createAction(Button::ButtonAction action);

        /**
         * Create a character from a string.
         */
        static Field createCharacter(std::string character);

        /**
         * Create an identifier using int id, usually representing a button id.
         */
        static Field createIdentifier(int id);

        /**
         * Create a name field.
         */
        static Field createName(std::string name);

        /**
         * Create a date time field using a DateTime value.
         */
        static Field createDateTime(DateTime dateTime);

        /**
         * Create a date time field using a DateTime value.
         */
        static Field createDeviceType(Event::Common::Device device);

        /**
         * Create a time field, which represents an interval of time.
         */
        static Field createTime(std::chrono::nanoseconds interval);

        /**
         * Create a position x Field.
         */
        static Field createPositionX(double position);

        /**
         * Create a position y Field.
         */
        static Field createPositionY(double position);

        /**
         * Create a scroll Field using the Direction and amount.
         */
        static Field createScroll(Pointer::Direction direction, double amount);

    private:
        static constexpr std::string_view ACTION_FIELD_NAME{"Action"};
        static constexpr std::string_view ACTION_PRESS{"Press"};
        static constexpr std::string_view ACTION_RELEASE{"Release"};
        static constexpr std::string_view ACTION_REPEAT{"Repeat"};
        static constexpr std::string_view CHARACTER_FIELD_NAME{"Character"};
        static constexpr std::string_view IDENTIFIER_FIELD_NAME{"Identifier"};
        static constexpr std::string_view NAME_FIELD_NAME{"Name"};
        static constexpr std::string_view DATE_TIME_FIELD_NAME{"DateTime"};
        static constexpr std::string_view DEVICE_TYPE_FIELD_NAME{"DeviceType"};
        static constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
        static constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};
        static constexpr std::string_view TIME_FIELD_NAME{"Time"};
        static constexpr std::string_view POSITIONX_FIELD_NAME{"PositionX"};
        static constexpr std::string_view POSITIONY_FIELD_NAME{"PositionY"};
        static constexpr std::string_view SCROLLDOWN_FIELD_NAME{"ScrollDown"};
        static constexpr std::string_view SCROLLLEFT_FIELD_NAME{"ScrollLeft"};
        static constexpr std::string_view SCROLLRIGHT_FIELD_NAME{"ScrollRight"};
        static constexpr std::string_view SCROLLUP_FIELD_NAME{"ScrollUp"};


        /**
         * Create a Field using a double value.
         */
        static Field createDouble(std::string_view name, double value);

        std::string name;
        EntryOrData entry;
    };

    constexpr bool EvgetCore::Event::Field::isEntry() {
        return std::holds_alternative<std::string>(entry);
    }

    constexpr bool EvgetCore::Event::Field::isData() {
        return std::holds_alternative<Entries>(entry);
    }
}

#endif //EVGET_SRC_EVENT_FIELD_H
