#include "common/log.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

test::CapturedLog::CapturedLog() : previous_{spdlog::default_logger()} {
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(output_);
    auto logger = std::make_shared<spdlog::logger>("captured", sink);
    logger->set_pattern("%l %v");
    logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);
}

test::CapturedLog::~CapturedLog() {
    spdlog::set_default_logger(previous_);
}

std::vector<std::string> test::CapturedLog::Lines() const {
    std::vector<std::string> lines{};
    std::istringstream input{output_.str()};
    for (std::string line; std::getline(input, line);) {
        lines.push_back(line);
    }
    return lines;
}
