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

#ifndef LOCKINGVECTOR_H
#define LOCKINGVECTOR_H

#include <vector>
#include <mutex>

namespace Async {

/**
 * \brief A thread-safe lockable vector implementation.
 * \tparam T The inner vector type.
 */
template<class T>
class LockingVector {
public:
    LockingVector();
    explicit LockingVector(std::vector<T> inner);

    /**
     * \brief Move data into the inner vector without locking. Not thread-safe.
     * \param value value to move.
     */
    constexpr void unsafe_push_back(T&& value);

    /**
     * \brief Move data into the inner vector by locking. Thread-safe.
     * \param value value to move.
     */
    constexpr void push_back(T&& value);
private:
    std::vector<T> inner;
    std::mutex lock{};
};

template <class T>
constexpr void LockingVector<T>::unsafe_push_back(T&& value) {
    inner.push_back(value);
}

template <class T>
constexpr void LockingVector<T>::push_back(T&& value) {
    std::lock_guard guard{lock};
    inner.push_back(value);
}

template <class T>
LockingVector<T>::LockingVector() = default;

template <class T>
LockingVector<T>::LockingVector(std::vector<T> inner) : inner{inner} {
}

}

#endif //LOCKINGVECTOR_H
