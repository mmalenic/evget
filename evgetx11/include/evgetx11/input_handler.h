#ifndef EVGETX11_XINPUTHANDLER_H
#define EVGETX11_XINPUTHANDLER_H

#include <functional>

#include "evget/error.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

namespace evgetx11 {

class InputHandler {
public:
    static evget::Result<InputHandler> Build(X11Api& x_wrapper);

    /**
     * Get the next event.
     */
    [[nodiscard]] InputEvent GetEvent() const;

private:
    explicit InputHandler(X11Api& x_wrapper);

    static constexpr int kVersionMajor = 2;
    static constexpr int kVersionMinor = 2;

    std::reference_wrapper<X11Api> x_wrapper_;

    static void SetMask(X11Api& x_wrapper);

    static evget::Result<void> AnnounceVersion(X11Api& x_wrapper);
};
}  // namespace evgetx11

#endif
