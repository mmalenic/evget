#include "evgetx11/log_once.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "common/log.h"

TEST(LogOnceTest, WarnsOnce) {
    const test::CapturedLog log{};
    evgetx11::LogOnce log_once{};

    log_once.Warn("no active window");
    log_once.Warn("no active window");
    log_once.Warn("no focus window");
    log_once.Warn("no active window");

    ASSERT_EQ(
        log.Lines(),
        (std::vector<std::string>{
            "warning no active window",
            "debug no active window",
            "warning no focus window",
            "debug no active window",
        })
    );
}
