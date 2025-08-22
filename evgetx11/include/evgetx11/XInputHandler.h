#ifndef EVGETX11_XINPUTHANDLER_H
#define EVGETX11_XINPUTHANDLER_H

#include <functional>

#include "evget/error.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XWrapper.h"

namespace evgetx11 {

class XInputHandler {
public:
    static evget::Result<XInputHandler> Build(XWrapper& x_wrapper);

    /**
     * Get the next event.
     */
    [[nodiscard]] XInputEvent GetEvent() const;

private:
    explicit XInputHandler(XWrapper& x_wrapper);

    static constexpr int kVersionMajor = 2;
    static constexpr int kVersionMinor = 2;

    std::reference_wrapper<XWrapper> x_wrapper_;

    static void SetMask(XWrapper& x_wrapper);

    static evget::Result<void> AnnounceVersion(XWrapper& x_wrapper);
};
}  // namespace evgetx11

#endif
