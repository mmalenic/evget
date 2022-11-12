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
//

#ifndef EVGET_MODIFIER_H
#define EVGET_MODIFIER_H

#include <optional>
#include "ModifierValue.h"
#include "Data.h"
#include "Schema.h"

namespace EvgetCore::Event {
    /**
     * Represents a key modifier value.
     */
    class Modifier {
    public:
        /**
         * Add a modifier value.
         */
        Modifier& interval(ModifierValue modifierValue);

        /**
         * Build modifier.
         */
        Data build();

        /**
         * Generate the Modifier schema.
         */
        static constexpr Schema<1> generateSchema();

    private:
        std::optional<ModifierValue> _modifierValue;
    };

    constexpr EvgetCore::Event::Schema<1> EvgetCore::Event::Modifier::generateSchema() {
        return Schema<1>{"Modifier", {
                SchemaField::MODIFIER_FIELD}
        };
    }
}

#endif //EVGET_MODIFIER_H