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

#ifndef EVGET_INCLUDE_SYSTEMEVENTDEVICE_H
#define EVGET_INCLUDE_SYSTEMEVENTDEVICE_H

/**
 * Defines the type of events that system events can take.
 * @tparam T the type of event
 */
template <typename T>
class SystemEventDevice {
public:
    enum Value {
        mouseDevice,
        keyDevice,
        touchDevice
    };

    /**
     * Create the device.
     * @param value value
     * @param data data
     */
    SystemEventDevice(Value value, T data);

    explicit operator Value() const;
    bool operator ==(const SystemEventDevice &b) const;
    bool operator ==(const Value &b) const;
    operator bool() = delete;

    virtual ~SystemEventDevice() = default;
    SystemEventDevice(const SystemEventDevice&) = default;
    SystemEventDevice(SystemEventDevice&&) noexcept = default;
    SystemEventDevice& operator=(const SystemEventDevice&) = default;
    SystemEventDevice& operator=(SystemEventDevice&&) noexcept = default;

private:
    Value value;
    T data;
};

template<typename T>
SystemEventDevice<T>::SystemEventDevice(SystemEventDevice::Value value, T data) : value{value}, data{data} {
}

template<typename T>
SystemEventDevice<T>::operator Value() const {
    return value;
}

template<typename T>
bool SystemEventDevice<T>::operator==(const SystemEventDevice& b) const {
    return value == b.value;
}

template<typename T>
bool SystemEventDevice<T>::operator==(const SystemEventDevice::Value& b) const {
    return value == b;
}

#endif //EVGET_INCLUDE_SYSTEMEVENTDEVICE_H
