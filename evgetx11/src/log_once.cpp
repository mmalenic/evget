#include "evgetx11/log_once.h"

#include <spdlog/spdlog.h>

#include <string>

void evgetx11::LogOnce::Warn(const std::string& message) {
    if (warned_.insert(message).second) {
        spdlog::warn("{}", message);
        return;
    }
    spdlog::debug("{}", message);
}
