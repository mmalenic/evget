#include "evgetwindows/backend.h"

#include <windows.h>

#include <boost/asio/any_io_executor.hpp>

#include <expected>
#include <memory>
#include <utility>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/input_event.h"
#include "evget/storage/store.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"
#include "evgetwindows/windows_query_api.h"

evgetwindows::Backend::Backend(std::unique_ptr<WindowsApi> windows, evget::Store& storage)
    : windows_(std::move(windows)),
      query_(std::make_unique<WindowsQuery>()),
      transformer_(*query_, tracker_),
      next_event_(*this->windows_),
      handler_(storage, transformer_, next_event_) {
    tracker_.Seed(query_->ToggleState(VK_CAPITAL), query_->ToggleState(VK_NUMLOCK), query_->ToggleState(VK_SCROLL));
}

evget::Result<std::unique_ptr<evgetwindows::Backend>>
evgetwindows::Backend::Create(evget::Store& storage, const boost::asio::any_io_executor& executor) {
    auto windows = Windows::New(executor);
    if (!windows.has_value()) {
        return std::unexpected(windows.error());
    }

    return std::unique_ptr<Backend>(new Backend(std::move(*windows), storage));
}

evget::EventHandler<evget::InputEvent<evgetwindows::RawEvent>>& evgetwindows::Backend::Handler() {
    return handler_;
}

void evgetwindows::Backend::Stop() {
    windows_->Stop();
}
