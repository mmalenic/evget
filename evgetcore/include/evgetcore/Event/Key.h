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
#include "evgetcore/Event/Common/Time.h"
#include "evgetcore/Event/Button/Character.h"
#include "evgetcore/Event/Button/ButtonAction.h"
#include "evgetcore/Event/Button/Identifier.h"
#include "evgetcore/Event/Button/Action.h"
#include "evgetcore/Event/Button/Name.h"

namespace EvgetCore::Event {
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
            KeyBuilder& action(Button::ButtonAction action);

            /**
             * Add id.
             */
            KeyBuilder& button(int button);

            /**
             * Add id.
             */
            KeyBuilder& name(std::string name);

            /**
             * Add character.
             */
            KeyBuilder& character(const std::string& character);

            /**
             * Build key event.
             */
            std::unique_ptr<Key> build();

        private:
            std::unique_ptr<Common::Time> _time;
            std::unique_ptr<Button::Action> _buttonType;
            std::unique_ptr<Button::Identifier> _button;
            std::unique_ptr<Button::Name> _name;
            std::unique_ptr<Button::Character> _character;
        };

        /**
         * Create event entry.
         */
        explicit Key(KeyBuilder& builder);
    };
}

#endif //EVGET_SRC_KEY_H
