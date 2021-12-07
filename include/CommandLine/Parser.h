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

#include <boost/program_options.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>
#include "Option.h"
#include "OptionValidated.h"
#include "OptionFlag.h"

namespace CommandLine {

    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    /**
     * Available filetypes.
     */
    enum Filetype {
        csv,
        sqlite
    };

    /**
     * The Parser class controls command line options.
     */
    class Parser {
    public:
        /**
         * Create a Parser object.
         *
         * @param platform platform information string
         */
        explicit Parser(std::string platform);

        /**
         * Perform the actual command line parsing. Returns true if operation of the program should continue.
         *
         * @param argc from main
         * @param argv from main
         */
        bool parseCommandLine(int argc, const char *argv[]);

        /**
         * Get config file location.
         */
        [[nodiscard]] fs::path getConfigFile() const;

        /**
         * Get the folder.
         */
        [[nodiscard]] fs::path getFolder() const;

        /**
         * Getter for print flag.
         * @return print flag
         */
        [[nodiscard]] bool shouldPrint() const;

        /**
         * Getter for system events flag.
         * @return system events flag
         */
        [[nodiscard]] bool useSystemEvents() const;

        /**
         * Get the log level.
         * @return
         */
        [[nodiscard]] spdlog::level::level_enum getLogLevel() const;

        /**
         * Get the file type.
         * @return file type
         */
        [[nodiscard]] std::vector<Filetype> getFiletype() const;

        friend std::ostream &operator<<(std::ostream &os, const Filetype &filetype);

        friend std::istream &operator>>(std::istream &in, Filetype &algorithm);

        virtual ~Parser() = default;

        Parser(const Parser &) = default;

        Parser(Parser &&) = default;

        Parser &operator=(const Parser &) = delete;

        Parser &operator=(Parser &&) = delete;

    protected:
        /**
         * Get description.
         * @return description
         */
        po::options_description &getCombinedDesc();

        /**
         * Get commandline description.
         */
        po::options_description &getCmdlineDesc();

        /**
         * Get config description.
         */
        po::options_description &getConfigDesc();

        /**
         * Get the variables map.
         * @return variables map
         */
        [[nodiscard]] const po::variables_map &getVm() const;

        /**
         * Parse the command line only options. Returns true if operation of the
         * program should continue.
         */
        virtual bool parseCmdlineOnlyOptions();

        /**
         * Parse the file and command line options. Returns true if operation of
         * the program should continue.
         */
        virtual bool parseFileAndCmdlineOptions();

    private:
        std::string platformInformation;
        po::options_description cmdlineDesc;
        po::options_description configDesc;
        po::variables_map vm;

        OptionFlag help;
        OptionFlag version;
        Option<fs::path> config;
        Option<fs::path> storageFolder;
        Option<std::vector<Filetype>> filetypes;
        OptionFlag print;
        OptionFlag systemEvents;
        OptionValidated<spdlog::level::level_enum> logLevel;

        /**
         * Get the valid log levels.
         * @return valid log levels
         */
        static std::string logLevelsString();

        /**
         * Converts the log level string into a log level if possible.
         */
        static std::optional<spdlog::level::level_enum> validateLogLevel(std::string logLevel);

        /**
         * Store and notify the variables map.
         */
        static void storeAndNotify(const po::parsed_options& parsedOptions, po::variables_map& vm);

        /**
         * Format an option.
         */
        template<typename T>
        static std::string formatConfigOption(const OptionBase<T>& option);

        /**
        * Format an option.
        */
        template<typename T>
        static std::string formatConfigOption(const OptionBase<T>& option, const std::string& value);

        /**
         * Format the config file.
         */
        virtual std::string formatConfigFile();
    };

}
#endif //INPUT_EVENT_RECORDER_COMMANDLINE_H
