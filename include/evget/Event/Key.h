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
#include "Data.h"
#include "evget/Event/Common/Time.h"
#include "evget/Event/Button/Press.h"
#include "evget/Event/Button/Release.h"
#include "evget/Event/Button/Repeat.h"
#include "evget/Event/Button/Character.h"

namespace Event {
    /**
     * Represents a key event.
     */
    class Key : public Data {
    public:
        class KeyBuilder {
        public:
            friend class Key;

            KeyBuilder();

            /**
             * Add time in nanoseconds.
             */
            KeyBuilder& time(std::chrono::nanoseconds nanoseconds);

            /**
             * Add press.
             */
            KeyBuilder& press(std::string button);

            /**
             * Add release.
             */
            KeyBuilder& release(std::string button);

            /**
             * Add repeat.
             */
            KeyBuilder& repeat(std::string button);

            /**
             * Add character.
             */
            KeyBuilder& character(char character);

            /**
             * Build mouse move event.
             */
            Key build();

        private:
            Common::Time _time;
            Button::Press _press;
            Button::Release _release;
            Button::Repeat _repeat;
            Button::Character _character;
        };

        /**
         * Create event entry.
         */
        explicit Key(const KeyBuilder& builder);
    };
}

#endif //EVGET_SRC_KEY_H
