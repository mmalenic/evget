#include <gtest/gtest.h>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#if defined(EVGET_CRT_DEBUG_HEAP) && defined(_DEBUG)
#include <crtdbg.h>

#include <initializer_list>
#endif

/**
 * The test executable main function. This is used to turn off logs for tests.
 */
int main(int argc, char** argv) {
#if defined(EVGET_CRT_DEBUG_HEAP) && defined(_DEBUG)
    _CrtSetDbgFlag(
        _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF
    );
    for (const int report : {_CRT_WARN, _CRT_ERROR, _CRT_ASSERT}) {
        _CrtSetReportMode(report, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(report, _CRTDBG_FILE_STDERR);
    }
#endif

    spdlog::set_level(spdlog::level::off);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
