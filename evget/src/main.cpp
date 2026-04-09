#if !defined(FEATURE_EVGETLIBINPUT) && !defined(FEATURE_EVGETX11)
#error "define at least one of `FEATURE_EVGETLIBINPUT`or `FEATURE_EVGETX11`"
#endif

#include <spdlog/spdlog.h>

#include <exception>
#include <expected>
#include <memory>
#include <optional>
#include <utility>

#include "evget/async/scheduler/scheduler.h"
#include "evget/cli.h"
#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/storage/database_manager.h"
#include "evget/storage/store.h"

#ifdef FEATURE_EVGETLIBINPUT
#include "evget/input_event.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/event_transformer.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/next_event.h"
#include "evgetlibinput/xkbcommon.h"
#endif

#ifdef FEATURE_EVGETX11
#include <X11/Xlib.h>

#include "evget/event_transformer.h"
#include "evgetx11/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"
#include "evgetx11/x11.h"
#endif

namespace {

#ifdef FEATURE_EVGETLIBINPUT
/// \brief Owns all libinput backend objects.
class LibInputBackend {
public:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes,cppcoreguidelines-non-private-member-variables-in-classes)
    evget::EventHandler<evget::InputEvent<evgetlibinput::LibInputEvent>>& handler;

    LibInputBackend(const LibInputBackend&) = delete;
    LibInputBackend(LibInputBackend&&) = delete;
    LibInputBackend& operator=(const LibInputBackend&) = delete;
    LibInputBackend& operator=(LibInputBackend&&) = delete;
    ~LibInputBackend() = default;

private:
    friend evget::Result<std::unique_ptr<LibInputBackend>>
    CreateLibInputBackend(evget::Cli& cli, evget::Store& storage);

    LibInputBackend(
        std::unique_ptr<evgetlibinput::LibInputApi> libinput,
        evgetlibinput::XkbCommon xkb,
        evgetlibinput::ScreenDimensions dimensions,
        evget::Store& storage
    )
        : handler(handler_),
          libinput_(std::move(libinput)),
          xkb_(std::move(xkb)),
          transformer_(*this->libinput_, this->xkb_, dimensions),
          next_event_(*this->libinput_),
          handler_(storage, transformer_, next_event_) {}

    std::unique_ptr<evgetlibinput::LibInputApi> libinput_;
    evgetlibinput::XkbCommon xkb_;
    evgetlibinput::EventTransformer transformer_;
    evgetlibinput::NextEvent next_event_;
    evget::EventHandler<evget::InputEvent<evgetlibinput::LibInputEvent>> handler_;
};

evget::Result<std::unique_ptr<LibInputBackend>> CreateLibInputBackend(evget::Cli& cli, evget::Store& storage) {
    auto libinput = evgetlibinput::LibInput::New();
    if (!libinput.has_value()) {
        return std::unexpected(libinput.error());
    }

    auto xkb = evgetlibinput::XkbCommon::New();
    if (!xkb.has_value()) {
        return std::unexpected(xkb.error());
    }

    evgetlibinput::ScreenDimensions dimensions{};
    if (auto cli_dimensions = cli.ScreenDimensions()) {
        dimensions = {.width = cli_dimensions->first, .height = cli_dimensions->second};
    } else {
        auto drm = evgetlibinput::DrmOutput::New();
        if (!drm.has_value()) {
            return std::unexpected(drm.error());
        }
        dimensions = drm->GetDimensions();
    }

    return std::unique_ptr<LibInputBackend>(
        new LibInputBackend(std::move(*libinput), std::move(*xkb), dimensions, storage)
    );
}
#endif

#ifdef FEATURE_EVGETX11
/// \brief Owns all X11 backend objects.
class X11Backend {
public:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes,cppcoreguidelines-non-private-member-variables-in-classes)
    std::optional<evget::EventHandler<evgetx11::InputEvent>>& handler;

    X11Backend(const X11Backend&) = delete;
    X11Backend(X11Backend&&) = delete;
    X11Backend& operator=(const X11Backend&) = delete;
    X11Backend& operator=(X11Backend&&) = delete;
    ~X11Backend() = default;

private:
    friend evget::Result<std::unique_ptr<X11Backend>> CreateX11Backend(evget::Store& storage);

    explicit X11Backend(std::unique_ptr<Display, decltype(&XCloseDisplay)> display)
        : handler(handler_), display_(std::move(display)), api_(*this->display_) {}

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display_;
    evgetx11::X11 api_;
    std::unique_ptr<evget::EventTransformer<evgetx11::InputEvent>> transformer_;
    std::unique_ptr<evgetx11::InputHandler> next_event_;
    std::optional<evget::EventHandler<evgetx11::InputEvent>> handler_;
};

evget::Result<std::unique_ptr<X11Backend>> CreateX11Backend(evget::Store& storage) {
    auto backend = std::unique_ptr<X11Backend>(new X11Backend({XOpenDisplay(nullptr), XCloseDisplay}));

    backend->transformer_ = evgetx11::EventTransformerBuilder{}.PointerKey(backend->api_).Touch().Build(backend->api_);

    auto next_event = evgetx11::InputHandlerBuilder::Build(backend->api_);
    if (!next_event.has_value()) {
        return std::unexpected(next_event.error());
    }
    backend->next_event_ = std::move(*next_event);

    backend->handler_.emplace(storage, *backend->transformer_, *backend->next_event_);
    return backend;
}
#endif

} // namespace

int main(int argc, char* argv[]) {
    evget::EventSource default_source{};
#ifdef FEATURE_EVGETX11
    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
    default_source = evget::EventSource::kX11;
#endif
#ifdef FEATURE_EVGETLIBINPUT
    default_source = evget::EventSource::kLibInput;
#endif

    auto cli = evget::Cli{default_source};
    auto exit = cli.Parse(argc, argv);
    if (!exit.has_value()) {
        return exit.error();
    }
    if (*exit) {
        return 0;
    }

    std::shared_ptr<evget::Scheduler> scheduler;
    try {
        scheduler = std::make_shared<evget::Scheduler>();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return 1;
    }
    auto manager = evget::DatabaseManager{scheduler, {}, cli.StoreNEvents(), cli.StoreAfter()};

    auto stores = cli.ToStores();
    if (!stores.has_value()) {
        spdlog::error("{}", stores.error());
        return 1;
    }
    for (auto&& store : *stores) {
        manager.AddStore(std::move(store));
    }

    auto exit_code = 0;
    try {
        auto event_source = cli.EventSource();

#ifdef FEATURE_EVGETLIBINPUT
        std::unique_ptr<LibInputBackend> li_backend{};
        if (event_source == evget::EventSource::kLibInput) {
            auto result = CreateLibInputBackend(cli, manager);
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            li_backend = std::move(*result);
            scheduler->SpawnResult(li_backend->handler.Start(), li_backend->handler, exit_code);
        }
#endif

#ifdef FEATURE_EVGETX11
        std::unique_ptr<X11Backend> x11_backend{};
        if (event_source == evget::EventSource::kX11) {
            auto result = CreateX11Backend(manager);
            if (!result.has_value()) {
                spdlog::error("{}", result.error());
                return 1;
            }
            x11_backend = std::move(*result);
            scheduler->SpawnResult(x11_backend->handler->Start(), *x11_backend->handler, exit_code);
        }
#endif

        scheduler->Join();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        exit_code = 1;
    }

    return exit_code;
}
