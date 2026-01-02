#include "evgetlibinput/next_event.h"

#include <boost/asio/awaitable.hpp>

#include "evget/error.h"
#include "evget/input_event.h"
#include "evgetlibinput/libinput_api.h"

evgetlibinput::NextEvent::NextEvent(LibInputApi& libinput_api) : libinput_api_{libinput_api} {}

boost::asio::awaitable<evget::Result<evget::InputEvent<evgetlibinput::LibInputEvent>>>
evgetlibinput::NextEvent::Next() const {
    co_return libinput_api_.get().GetEvent().transform([](auto event) { return evget::InputEvent{std::move(event)}; });
}
