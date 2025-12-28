#include "evgetlibinput/next_event.h"

#include <boost/asio/awaitable.hpp>

#include "evget/error.h"
#include "evgetlibinput/libinput_api.h"

evgetlibinput::NextEvent::NextEvent(LibInputApi& libinput_api) : libinput_api_{libinput_api} {}

boost::asio::awaitable<evget::Result<evgetlibinput::InputEvent>> evgetlibinput::NextEvent::Next() const {
    co_return libinput_api_.get().GetEvent();
}
