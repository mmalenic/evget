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

#ifndef EVGET_INCLUDE_SHUTDOWNHANDLER_H
#define EVGET_INCLUDE_SHUTDOWNHANDLER_H

#include <atomic>

template <typename T, typename R, typename S>
class ShutdownHandler {
public:
    typedef R (* ReturnValueFn)();
    typedef void* (* RegisterHandlerFn)(S, T);
    typedef void* (* FlipRegisterHandlerFn)(T, S);

    ShutdownHandler(std::vector<S> registerFor, ReturnValueFn returnValueFn, RegisterHandlerFn registerHandlerFn);
    /**
     * Whether the program should shutdown.
     * @return should shutdown.
     */
    [[nodiscard]] static bool shouldShutdown();

    /**
     * Set the shutdown flag.
     * @param flag shutdown flag
     */
    static void setShutdownFlag(bool flag);

    /**
     * Set the shutdown flag to true.
     */
    static R activateShutdownFlag(T input);

    /**
     * Set the interrupt handler.
     */
    virtual void registerInterruptHandler() = 0;

    ShutdownHandler(const ShutdownHandler&) = delete;
    ShutdownHandler(ShutdownHandler&&) noexcept = delete;
    ShutdownHandler& operator=(const ShutdownHandler&) = delete;
    ShutdownHandler& operator=(ShutdownHandler&&) noexcept = delete;

private:
    static std::atomic<bool> shutdown;
    static ReturnValueFn returnValueFn;
};

template<typename T, typename R, typename S>
bool ShutdownHandler<T, R, S>::shouldShutdown() {
    return shutdown.load();
}

template<typename T, typename R, typename S>
void ShutdownHandler<T, R, S>::setShutdownFlag(bool flag) {
    shutdown.store(flag);
}

template<typename T, typename R, typename S>
R ShutdownHandler<T, R, S>::activateShutdownFlag(T _) {
    setShutdownFlag(true);
    return R();
}
template<typename T, typename R, typename S>
ShutdownHandler<T, R, S>::ShutdownHandler(
    std::vector<S> registerFor,
    ShutdownHandler::ReturnValueFn returnValueFn,
    ShutdownHandler::RegisterHandlerFn registerHandlerFn
) : returnValueFn{returnValueFn} {
    for (auto value : registerFor) {

    }
}

#endif //EVGET_INCLUDE_SHUTDOWNHANDLER_H
