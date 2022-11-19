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

#ifndef EVGET_RELATION_H
#define EVGET_RELATION_H

#include <functional>

namespace EvgetCore::Event {
    /**
     * A relation between two schemas.
     */
    template<typename From, typename To>
    class Relation {
    public:
        /**
         * Create the relation object.
         */
        constexpr Relation(const From &fromSchema, const To &toSchema, bool isUnique);

        /**
         * Get the from side of the relation.
         */
        constexpr From getFromSchema() const;

        /**
         * Get the to side of the relation.
         */
        constexpr To getToSchema() const;

        /**
         * Does this relation has unique entries on the to side.
         */
        [[nodiscard]] constexpr bool isUnique() const;

    private:
        std::reference_wrapper <From> fromSchema{};
        std::reference_wrapper <To> toSchema{};
        bool _isUnique{};
    };

    template<typename From, typename To>
    constexpr
    Relation<From, To>::Relation(const From &fromSchema, const To &toSchema, bool isUnique) : fromSchema{fromSchema},
                                                                                              toSchema{toSchema},
                                                                                              _isUnique{isUnique} {
    }

    template<typename From, typename To>
    constexpr From Relation<From, To>::getFromSchema() const {
        return fromSchema;
    }

    template<typename From, typename To>
    constexpr To Relation<From, To>::getToSchema() const {
        return toSchema;
    }

    template<typename From, typename To>
    constexpr bool Relation<From, To>::isUnique() const {
        return _isUnique;
    }
}

#endif //EVGET_RELATION_H
