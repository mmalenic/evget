#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

/**
 * The test executable main function. This is used to turn of logs for tests.
 */
int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::off);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
