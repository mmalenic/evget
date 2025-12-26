
#include "evgetlibinput/next_event.h"

#include <boost/asio/awaitable.hpp>

#include <iostream>

#include "evget/error.h"
#include "evgetlibinput/libinput_api.h"

evgetlibinput::NextEvent::NextEvent(LibInputApi& libinput_api) : libinput_api_{libinput_api} {}

boost::asio::awaitable<evget::Result<evgetlibinput::InputEvent>> evgetlibinput::NextEvent::Next() const {
    auto event = libinput_api_.get().GetEvent();
    std::cout << (event.has_value() && (*event) != nullptr);
    co_return event;
}
