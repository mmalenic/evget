#ifndef EVGETX11_INPUT_HANDLER_H
#define EVGETX11_INPUT_HANDLER_H

#include <functional>

#include "evget/error.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

namespace evgetx11 {

class InputHandler {
public:
    explicit InputHandler(X11Api& x_wrapper);

    /**
     * Get the next event.
     */
    [[nodiscard]] InputEvent GetEvent() const;

private:
    std::reference_wrapper<X11Api> x_wrapper_;
};

class InputHandlerBuilder {
public:
    static evget::Result<InputHandler> Build(X11Api& x_wrapper);

private:
    static constexpr int kVersionMajor = 2;
    static constexpr int kVersionMinor = 2;

    static void SetMask(X11Api& x_wrapper);
    static evget::Result<void> AnnounceVersion(X11Api& x_wrapper);
};
} // namespace evgetx11

#endif
