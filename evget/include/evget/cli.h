/**
 * \file cli.h
 * \brief Command line interface parser and configuration management.
 */

#ifndef EVGET_CLI_H
#define EVGET_CLI_H

#include <spdlog/common.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "evget/error.h"
#include "evget/event/device_type.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief The type of storage to use.
 */
enum class StorageType : uint8_t {
    kSqLite, ///< use an SQLite database to store events
    kJson ///< use JSON for event formatting
};

/**
 * \brief Where to source events from.
 */
enum class EventSource : uint8_t {
    kLibInput, ///< source events from libinput
    kX11, ///< source events from the X11 windowing system
};

/**
 * \brief The `Cli` class controls command line options.
 */
class Cli {
public:
    /**
     * \brief Create a cli object.
     * \param default_event_source the default event source if left unspecified by the user
     */
    explicit Cli(evget::EventSource default_event_source);

    /**
     * \brief Get the output location.
     * \return output location
     */
    [[nodiscard]] const std::vector<std::string>& Output() const;

    /**
     * \brief Parse the CLI, exiting the program if there are any errors. This handles
     * printing messages from `--help` and exiting if there is a parse error.
     *
     * \param argc argc input argc
     * \param argv argv input argv
     * \return whether to exit or an error exit code
     */
    std::expected<bool, int> Parse(int argc, char** argv);

    /**
     * \brief Get the storage type from an output string.
     * \param output reference to the output string
     * \return storage type enumeration value
     */
    static StorageType GetStorageType(const std::string& output);

    /**
     * \brief Convert CLI configuration to `Store` objects.
     * \return result containing vector of `Store` unique pointers or error
     */
    Result<std::vector<std::unique_ptr<Store>>> ToStores();

    /**
     * \brief Get the configured event source.
     * \return event source enumeration value
     */
    [[nodiscard]] evget::EventSource EventSource() const;

    /**
     * \brief Get the number of events to store before issuing a write operation.
     * \return number of events
     */
    [[nodiscard]] std::size_t StoreNEvents() const;

    /**
     * \brief Get the time interval after which to store events.
     * \return time interval in seconds
     */
    [[nodiscard]] std::chrono::seconds StoreAfter() const;

    /**
     * \brief Get the screen dimensions.
     * \return optional pair of (width, height) in pixels
     */
    [[nodiscard]] std::optional<std::pair<std::uint32_t, std::uint32_t>> ScreenDimensions() const;

    /**
     * \brief Get the X11 display name to connect to.
     * \return optional display name, if empty, the default display is used
     */
    [[nodiscard]] const std::optional<std::string>& Display() const;

    /**
     * \brief Get the libinput udev seat name to assign.
     * \return optional seat name, if empty, the default seat is used
     */
    [[nodiscard]] const std::optional<std::string>& Seat() const;

    /**
     * \brief Get the set of device types allowed through the event filter.
     * \return optional set of allowed device types, nullopt allows all
     */
    [[nodiscard]] const std::optional<std::set<DeviceType>>& Filter() const;

private:
    static constexpr std::size_t kDefaultNEvents{100};
    static constexpr std::size_t kDefaultStoreAfter{100};
    static constexpr std::size_t kIndentBy{30};

    std::vector<std::string> output_;
    std::size_t store_n_events_{kDefaultNEvents};
    std::size_t store_after_{kDefaultStoreAfter};
    evget::EventSource event_source_{EventSource::kX11};
    std::optional<spdlog::level::level_enum> log_level_;
    std::optional<std::pair<std::uint32_t, std::uint32_t>> screen_dimensions_;
    std::optional<std::string> display_;
    std::optional<std::string> seat_;
    std::optional<std::set<DeviceType>> filter_;
    std::vector<std::string> event_source_descriptions_{EventSourceDescriptions()};
    std::vector<std::string> log_level_descriptions_{LogLevelDescriptions()};
    std::vector<std::string> device_type_descriptions_{DeviceTypeDescriptions()};

    static std::string FormatEnum(
        const std::string& value_descriptor,
        const std::string& enum_description,
        std::vector<std::string>& descriptions,
        const std::string& default_value
    );
    static std::vector<std::string> EventSourceDescriptions();
    static std::map<std::string, evget::EventSource> EventSourceMappings();
    static std::string ToString(evget::EventSource event_source);
    static std::vector<std::string> LogLevelDescriptions();
    static std::map<std::string, spdlog::level::level_enum> LogLevelMappings();
    static std::vector<std::string> DeviceTypeDescriptions();
    static std::map<std::string, DeviceType> DeviceTypeMappings();
};
} // namespace evget

#endif
