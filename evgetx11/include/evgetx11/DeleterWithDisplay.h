#ifndef EVGETX11_DELETERWITHDISPLAY_H
#define EVGETX11_DELETERWITHDISPLAY_H

#include <X11/Xlib.h>

#include <functional>

namespace evgetx11 {
template <auto F>
class DeleterWithDisplay {
public:
    explicit DeleterWithDisplay(Display& display);

    template <typename T>
    void operator()(T* pointer) const;

private:
    std::reference_wrapper<Display> display_;
};

template <auto F>
template <typename T>
void evgetx11::DeleterWithDisplay<F>::operator()(T* pointer) const {
    F(&display_.get(), pointer);
}

template <auto F>
DeleterWithDisplay<F>::DeleterWithDisplay(Display& display) : display_{display} {}
}  // namespace evgetx11

#endif
