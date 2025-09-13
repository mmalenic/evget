#ifndef EVGETX11_DISPLAY_DELETER_H
#define EVGETX11_DISPLAY_DELETER_H

#include <X11/Xlib.h>

#include <functional>

namespace evgetx11 {
template <auto F>
class DisplayDeleter {
public:
    explicit DisplayDeleter(Display& display);

    template <typename T>
    void operator()(T* pointer) const;

private:
    std::reference_wrapper<Display> display_;
};

template <auto F>
template <typename T>
void evgetx11::DisplayDeleter<F>::operator()(T* pointer) const {
    F(&display_.get(), pointer);
}

template <auto F>
DisplayDeleter<F>::DisplayDeleter(Display& display) : display_{display} {}
} // namespace evgetx11

#endif
