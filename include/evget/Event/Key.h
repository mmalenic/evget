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
#include "AbstractData.h"
#include "evget/Event/Common/Time.h"
#include "evget/Event/Pressable/Press.h"
#include "evget/Event/Pressable/Release.h"
#include "evget/Event/Pressable/Repeat.h"
#include "evget/Event/Pressable/Character.h"
#include "evget/Event/Pressable/Action.h"
#include "evget/Event/Pressable/Button.h"
#include "evget/Event/Pressable/ButtonAction.h"

namespace Event {
    /**
     * Represents a key event.
     */
    class Key : public AbstractData {
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
             * Add action.
             */
            KeyBuilder& action(Pressable::Action action);

            /**
             * Add id.
             */
            KeyBuilder& button(std::string button);

            /**
             * Add character.
             */
            KeyBuilder& character(char character);

            /**
             * Build key event.
             */
            std::unique_ptr<Key> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Pressable::ButtonAction> _buttonType;
            std::unique_ptr<Pressable::Button> _button;
            std::unique_ptr<Pressable::Character> _character;
        };

        /**
         * Create event entry.
         */
        explicit Key(KeyBuilder& builder);
    };
}

#endif //EVGET_SRC_KEY_H
