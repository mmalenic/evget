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
 * The CoreParser class controls command line options.
 */
class Cli {
public:
    /**
     * Create a CoreParser object.
     */
    explicit Cli() = default;

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
    [[nodiscard]] size_t StoreNEvents() const;
    [[nodiscard]] std::chrono::seconds StoreAfter() const;

private:
    static constexpr uint8_t kDefaultNEvents{100};
    static constexpr uint8_t kDefaultStoreAfter{60};
    static constexpr uint8_t kIndentBy{30};

    std::vector<std::string> output_;
    size_t store_n_events_{kDefaultNEvents};
    std::chrono::seconds store_after_{kDefaultStoreAfter};

    template <typename T>
    static std::string FormatEnum(
        const std::string& value_descriptor,
        const std::string& enum_description,
        std::uint8_t first_ident,
        std::map<T, std::string>& descriptions
    );
};

template <typename T>
std::string Cli::FormatEnum(
    const std::string& value_descriptor,
    const std::string& enum_description,
    std::uint8_t first_ident,
    std::map<T, std::string>& descriptions
) {
    auto out_description = std::format(
        "<{}>{: <{}}{}\n\n{: <{}}Possible values:\n",
        value_descriptor,
        "",
        first_ident,
        enum_description,
        "",
        kIndentBy
    );
    for (const auto& [_, description] : descriptions) {
        out_description.append(std::format("{: <{}}{}\n", "", kIndentBy, description));
    }

    return out_description;
}

} // namespace evget

#endif
