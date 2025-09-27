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
 * The type of storage to use
 */
enum class StorageType : uint8_t {
    /**
     * Use an SQLite database to store events.
     */
    kSqLite,
    /**
     * Use JSON for outputting events.
     */
    kJson
};

/**
 * Where to source events from.
 */
enum class EventSource : uint8_t {
    /**
     * Source events from the X11 windowing system.
     */
    kX11,
};

/**
 * The CoreParser class controls command line options.
 */
class Cli {
public:
    /**
     * Create a CoreParser object.
     */
    Cli() = default;

    /**
     * Get the output location.
     * @return
     */
    [[nodiscard]] const std::vector<std::string>& Output() const;

    /**
     * Parse the CLI, exiting the program if there are any errors. This handles
     * printing messages from `--help` and exiting if there is a parse error.
     *
     * @param argc argc input argc
     * @param argv argv input argv
     * @return whether to exit or an error exit code
     */
    std::expected<bool, int> Parse(int argc, char** argv);

    static StorageType GetStorageType(std::string& output);

    Result<std::vector<std::unique_ptr<Store>>> ToStores();
    [[nodiscard]] evget::EventSource EventSource() const;
    [[nodiscard]] std::size_t StoreNEvents() const;
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
