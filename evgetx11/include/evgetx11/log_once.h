/**
 * \file log_once.h
 * \brief Logging for failures that repeat on every event.
 */

#ifndef EVGETX11_LOG_ONCE_H
#define EVGETX11_LOG_ONCE_H

#include <string>
#include <unordered_set>

namespace evgetx11 {

/**
 * \brief Logs each distinct message at warn first time and then at debug.
 */
class LogOnce {
public:
    /**
     * \brief Log a message at warn if it has not been logged before, otherwise at debug.
     * \param message the message
     */
    void Warn(const std::string& message);

private:
    std::unordered_set<std::string> warned_;
};

} // namespace evgetx11

#endif
