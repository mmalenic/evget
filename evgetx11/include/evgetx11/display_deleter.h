#ifndef EVGETX11_DISPLAY_DELETER_H
#define EVGETX11_DISPLAY_DELETER_H

#include <X11/Xlib.h>

#include <functional>

namespace evgetx11 {
/**
 * \brief A custom deleter for X11 resources that require a Display pointer for cleanup.
 * \tparam F Function pointer to the X11 cleanup function
 */
template <auto F>
class DisplayDeleter {
public:
    /**
     * \brief Construct a `DisplayDeleter` with a reference to an X11 Display.
     * \param display reference to the X11 Display
     */
    explicit DisplayDeleter(Display& display);

    /**
     * \brief Operator to delete the resource using the specified cleanup function.
     * \tparam T type of the resource to delete
     * \param pointer pointer to the resource to delete
     */
    template <typename T>
    void operator()(T* pointer) const;

private:
    std::reference_wrapper<Display> display_;
};

template <auto F>
template <typename T>
void DisplayDeleter<F>::operator()(T* pointer) const {
    F(&display_.get(), pointer);
}

template <auto F>
DisplayDeleter<F>::DisplayDeleter(Display& display) : display_{display} {}
} // namespace evgetx11

#endif
