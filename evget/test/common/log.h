#ifndef EVGET_TEST_COMMON_LOG_H
#define EVGET_TEST_COMMON_LOG_H

#include <spdlog/logger.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace test {

/**
 * \brief Sends the default spdlog logger into a buffer at debug level.
 */
class CapturedLog {
public:
    /**
     * \brief Replace the default logger with `<level> <message>` lines.
     */
    CapturedLog();

    /**
     * \brief Restore the previous default logger.
     */
    ~CapturedLog();

    CapturedLog(const CapturedLog&) = delete;
    CapturedLog(CapturedLog&&) = delete;
    CapturedLog& operator=(const CapturedLog&) = delete;
    CapturedLog& operator=(CapturedLog&&) = delete;

    /**
     * \brief Get the recorded lines.
     * \return uses `<level> <message>` per log call
     */
    [[nodiscard]] std::vector<std::string> Lines() const;

private:
    std::ostringstream output_;
    std::shared_ptr<spdlog::logger> previous_;
};

} // namespace test

#endif
