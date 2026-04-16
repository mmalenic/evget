#include "evget/cli.h"

#include <CLI/CLI.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "evget/database/sqlite/connection.h"
#include "evget/error.h"
#include "evget/event/device_type.h"
#include "evget/storage/database_storage.h"
#include "evget/storage/json_storage.h"
#include "evget/storage/store.h"

evget::Cli::Cli(evget::EventSource default_event_source) : event_source_{default_event_source} {}

const std::vector<std::string>& evget::Cli::Output() const {
    return this->output_;
}

std::expected<bool, int> evget::Cli::Parse(int argc, char** argv) {
    spdlog::cfg::load_env_levels();

    CLI::App app{"Show and store events from input devices.", "evget"};
    argv = app.ensure_utf8(argv);

    auto should_exit = false;
    app.add_flag_callback(
        "-v,--version",
        [&should_exit] {
            should_exit = true;
            std::cout << std::format("evlist {}\n\n{}\n{}\n", EVGET_VERSION, EVGET_LICENSE, EVGET_COPYRIGHT);
        },
        "Print version"
    );

    app.add_option(
           "-n,--store-n-events",
           store_n_events_,
           "Controls how many events to receive before outputting them to the store."
    )
        ->default_val(kDefaultNEvents)
        ->check(CLI::PositiveNumber);
    app.add_option(
           "-s,--store-after-seconds",
           store_after_,
           "Store events at least every interval specified with this option, even if fewer events than "
           "`--store-n-events` has been receieved."
    )
        ->default_val(kDefaultStoreAfter)
        ->check(CLI::PositiveNumber);
    app.add_option("-e,--event-source", event_source_)
        ->transform(CLI::Transformer{EventSourceMappings(), CLI::ignore_case})
        ->option_text(
            FormatEnum("EVENT_SOURCE", "The source of events.", event_source_descriptions_, ToString(event_source_))
        );

    app.add_option(
           "-D,--display",
           display_,
           "X11 display to connect to (e.g. ':0'). If not specified, the default display is used. "
           "Only used by the X11 event source."
    )
        ->default_str("$DISPLAY");
    app.add_option("-S,--seat", seat_, "libinput udev seat to assign. Only used by the libinput event source.")
        ->default_str("seat0");

    std::vector<std::string> raw_filter;
    app.add_option("-f,--filter", raw_filter, "Only capture events from the comma-separated list of device types.")
        ->transform(CLI::IsMember({"all", "mouse", "keyboard", "touchpad", "touchscreen", "tablet"}, CLI::ignore_case))
        ->option_text(FormatEnum("DEVICES", "Filter captured events by device type.", device_type_descriptions_, "all"))
        ->delimiter(',');

    app.add_option_function<std::string>(
           "-d,--screen-dimensions",
           [this](const std::string& value) {
               auto pos = value.find('x');
               if (pos == std::string::npos) {
                   throw CLI::ValidationError("--screen-dimensions", "expected format WIDTHxHEIGHT (e.g. 1920x1080)");
               }

               auto width = std::stoul(value.substr(0, pos));
               auto height = std::stoul(value.substr(pos + 1));
               screen_dimensions_.emplace(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
           },
           "Screen dimensions used to convert absolute pointer motion to screen "
           "coordinates for evgetlibinput. If not specified, dimensions are queried from DRM."
    )
        ->option_text("WIDTHxHEIGHT [DRM query]")
        ->default_str("DRM query");

    app.add_option("-l,--log-level", log_level_)
        ->transform(CLI::Transformer{LogLevelMappings(), CLI::ignore_case})
        ->option_text(FormatEnum(
            "LEVEL",
            "The logging level. Overrides the SPDLOG_LEVEL environment variable.",
            log_level_descriptions_,
            "$SPDLOG_LEVEL or info"
        ));

    app.add_option(
           "-o,--output",
           output_,
           "The output location of the storage. "
           "The file extension determines the storage format. "
           "Either JSON files or sqlite databases are supported using .json or .sqlite endings. "
           "'-' is supported to output to stdout when using json storage, which disables any logging."
    )
        ->default_val("-");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return std::unexpected{app.exit(e)};
    }

    if (!raw_filter.empty() && !std::ranges::contains(raw_filter, "all")) {
        auto mappings = DeviceTypeMappings();
        filter_.emplace();
        for (const auto& value : raw_filter) {
            if (mappings.contains(value)) {
                filter_->insert(mappings.at(value));
            }
        }
    }

    if (output_.empty()) {
        output_.emplace_back("-");
    }

    if (std::ranges::contains(output_, "-")) {
        spdlog::set_level(spdlog::level::off);
    } else if (log_level_.has_value()) {
        spdlog::set_level(*log_level_);
    }

    return should_exit;
}

evget::StorageType evget::Cli::GetStorageType(const std::string& output) {
    auto pos = output.rfind('.');
    if (pos == std::string::npos) {
        return StorageType::kJson;
    }

    auto ext = output.substr(pos);
    std::ranges::transform(ext, ext.begin(), [](unsigned char character) { return std::tolower(character); });

    if (ext == ".sqlite" || ext == ".sqlite3" || ext == ".db" || ext == ".db3" || ext == ".s3db" || ext == ".sl3") {
        return StorageType::kSqLite;
    }

    return StorageType::kJson;
}

evget::Result<std::vector<std::unique_ptr<evget::Store>>> evget::Cli::ToStores() {
    auto stores = std::vector<std::unique_ptr<Store>>{};
    stores.reserve(output_.size());

    for (auto& output : this->output_) {
        switch (GetStorageType(output)) {
            case StorageType::kSqLite: {
                auto connect = std::make_unique<SQLiteConnection>();
                auto database = std::make_unique<DatabaseStorage>(std::move(connect), output);
                auto result = database->Init();
                if (!result.has_value()) {
                    return Err{result.error()};
                }

                stores.emplace_back(std::move(database));

                break;
            }
            case StorageType::kJson: {
                if (output == "-") {
                    // Do nothing to delete std::cout.
                    auto deleter = [](std::ostream*) {};
                    std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> out = {&std::cout, deleter};

                    stores.emplace_back(std::make_unique<JsonStorage>(std::move(out)));
                } else {
                    auto out = std::make_unique<std::ofstream>(output, std::ios_base::app);
                    stores.emplace_back(std::make_unique<JsonStorage>(std::move(out)));
                }

                break;
            }
        }
    }

    return stores;
}

std::string evget::Cli::FormatEnum(
    const std::string& value_descriptor,
    const std::string& enum_description,
    std::vector<std::string>& descriptions,
    const std::string& default_value
) {
    auto out_description = std::format(
        "{} [{}]\n{: <{}}{}\n{: <{}}Possible values:\n",
        value_descriptor,
        default_value,
        "",
        kIndentBy,
        enum_description,
        "",
        kIndentBy
    );
    for (auto i = 0; i < descriptions.size(); ++i) {
        out_description.append(std::format("{: <{}}{}", "", kIndentBy, descriptions.at(i)));
        if (descriptions.size() > 1 && i < descriptions.size() - 1) {
            out_description.append("\n");
        }
    }

    return out_description;
}

std::vector<std::string> evget::Cli::EventSourceDescriptions() {
    return {
        "- libinput: source events from libinput",
        "- x11: source events from the X11 windowing system",
    };
}

std::map<std::string, evget::EventSource> evget::Cli::EventSourceMappings() {
    return {
        {"libinput", EventSource::kLibInput},
        {"x11", EventSource::kX11},
    };
}

std::string evget::Cli::ToString(evget::EventSource event_source) {
    switch (event_source) {
        case EventSource::kLibInput:
            return "libinput";
        case EventSource::kX11:
            return "x11";
    }
    return {};
}

std::vector<std::string> evget::Cli::LogLevelDescriptions() {
    return {
        "- trace: show all messages",
        "- debug: show debug and above",
        "- info: show info and above",
        "- warn: show warnings and above",
        "- error: show errors and above",
        "- critical: show critical messages",
        "- off: disable logging",
    };
}

std::map<std::string, spdlog::level::level_enum> evget::Cli::LogLevelMappings() {
    return {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off},
    };
}

std::vector<std::string> evget::Cli::DeviceTypeDescriptions() {
    return {
        "- all: all device types",
        "- mouse: mouse events",
        "- keyboard: keyboard events",
        "- touchpad: touchpad events",
        "- touchscreen: touchscreen events",
        "- tablet: tablet events",
    };
}

std::map<std::string, evget::DeviceType> evget::Cli::DeviceTypeMappings() {
    return {
        {"mouse", DeviceType::kMouse},
        {"keyboard", DeviceType::kKeyboard},
        {"touchpad", DeviceType::kTouchpad},
        {"touchscreen", DeviceType::kTouchscreen},
        {"tablet", DeviceType::kTablet},
    };
}

evget::EventSource evget::Cli::EventSource() const {
    return event_source_;
}

std::size_t evget::Cli::StoreNEvents() const {
    return store_n_events_;
}

std::chrono::seconds evget::Cli::StoreAfter() const {
    return std::chrono::seconds{store_after_};
}

std::optional<std::pair<std::uint32_t, std::uint32_t>> evget::Cli::ScreenDimensions() const {
    return screen_dimensions_;
}

const std::optional<std::string>& evget::Cli::Display() const {
    return display_;
}

const std::optional<std::string>& evget::Cli::Seat() const {
    return seat_;
}

const std::optional<std::set<evget::DeviceType>>& evget::Cli::Filter() const {
    return filter_;
}
