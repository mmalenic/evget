/**
 * \file cli.h
 * \brief Command line interface parser and configuration management.
 */

#ifndef EVGET_CLI_H
#define EVGET_CLI_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "evget/error.h"
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
    kX11, ///< source events from the X11 windowing system
};

/**
 * \brief The `Cli` class controls command line options.
 */
class Cli {
public:
    /**
     * \brief Create a cli object.
     */
    Cli() = default;

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
    static StorageType GetStorageType(std::string& output);

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

private:
    static constexpr std::size_t kDefaultNEvents{100};
    static constexpr std::size_t kDefaultStoreAfter{60};
    static constexpr std::size_t kIndentBy{30};

    std::vector<std::string> output_;
    std::size_t store_n_events_{kDefaultNEvents};
    std::chrono::seconds store_after_{kDefaultStoreAfter};
    evget::EventSource event_source_{EventSource::kX11};
    std::vector<std::string> event_source_descriptions_{EventSourceDescriptions()};

    static std::string FormatEnum(
        const std::string& value_descriptor,
        const std::string& enum_description,
        std::vector<std::string>& descriptions
    );
    static std::vector<std::string> EventSourceDescriptions();
    static std::map<std::string, evget::EventSource> EventSourceMappings();
};
} // namespace evget

#endif
