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

#ifndef EVGET_INCLUDE_SYSTEMEVENT_H
#define EVGET_INCLUDE_SYSTEMEVENT_H

namespace EvgetCore {

/**
 * Defines the type of events that system events can take.
 * @tparam T the type of event
 */
template <typename T>
class SystemEvent {
public:
    /**
     * Possible types.
     */
    enum class Type { mouseDevice, keyDevice, touchDevice };

    /**
     * Get the type.
     * @return type
     */
    Type getType() const;

    /**
     * Get the data.
     * @return data
     */
    T getData() const;

    /**
     * Create the device.
     * @param type type
     * @param data data
     */
    SystemEvent(Type type, T data);

    explicit operator Type() const;
    bool operator==(const SystemEvent& b) const;
    bool operator==(const Type& b) const;
    operator bool() = delete;

private:
    Type type;
    T data;
};

template <typename T>
SystemEvent<T>::SystemEvent(SystemEvent::Type type, T data) : type{type}, data{data} {}

template <typename T>
SystemEvent<T>::operator Type() const {
    return type;
}

template <typename T>
bool SystemEvent<T>::operator==(const SystemEvent& b) const {
    return type == b.type;
}

template <typename T>
bool SystemEvent<T>::operator==(const SystemEvent::Type& b) const {
    return type == b;
}

template <typename T>
T SystemEvent<T>::getData() const {
    return data;
}

template <typename T>
typename SystemEvent<T>::Type SystemEvent<T>::getType() const {
    return type;
}
}  // namespace EvgetCore

#endif  // EVGET_INCLUDE_SYSTEMEVENT_H
