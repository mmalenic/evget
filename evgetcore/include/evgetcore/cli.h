// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef INPUT_EVENT_RECORDER_COMMANDLINE_H
#define INPUT_EVENT_RECORDER_COMMANDLINE_H

#include <expected>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <map>

#include "Storage/DatabaseManager.h"

namespace EvgetCore {

namespace fs = std::filesystem;

/**
 * The type of storage to use
 */
enum class StorageType : uint8_t {
    /**
     * Use an SQLite database to store events.
     */
    SQLite,
    /**
     * Use JSON for outputting events.
     */
    Json
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
    [[nodiscard]] const std::vector<std::string>& output() const;

    /**
     * Parse the CLI, exiting the program if there are any errors. This handles
     * printing messages from `--help` and exiting if there is a parse error.
     *
     * @param argc argc input argc
     * @param argv argv input argv
     * @return whether to exit or an error exit code
     */
    std::expected<bool, int> parse(int argc, char** argv);

    static StorageType get_storage_type(std::string& output);

    std::vector<std::unique_ptr<Storage::Store>> to_stores();

private:
    static constexpr uint8_t INDENT_BY{30};

    std::vector<std::string> output_{};

    template <typename T>
static std::string format_enum(
    const std::string& value_descriptor,
    const std::string& enum_description,
    uint8_t first_ident,
    const std::map<T, std::string>& descriptions
);

};

    template <typename T>
std::string Cli::format_enum(
    const std::string& value_descriptor,
    const std::string& enum_description,
    uint8_t first_ident,
    const std::map<T, std::string>& descriptions
) {
        auto out_description = std::format(
            "<{}>{: <{}}{}\n\n{: <{}}Possible values:\n",
            value_descriptor,
            "",
            first_ident,
            enum_description,
            "",
            INDENT_BY
        );
        for (const auto& [_, description] : descriptions) {
            out_description.append(
                std::format("{: <{}}{}\n", "", INDENT_BY, description)
            );
        }

        return out_description;
    }

}  // namespace EvgetCore
#endif  // INPUT_EVENT_RECORDER_COMMANDLINE_H
